#include "Voxelizer.hpp"


#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/component_wise.hpp>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/Image.hpp>

#include "ShaderProgram.hpp"
#include "GLHelper.hpp"
#include "glm.hpp"


/* Returns y such as x+y = ceil(x) */
static inline float complement(float x)
{
    return std::ceil(x) - x;
}

static unsigned int makeMultipleOf32(unsigned int n)
{
    if (n % 32 != 0)
        n += 32 - (n % 32);
    return n;
}

static void allocate3DTexture(GLuint& id, unsigned int width, unsigned int height, unsigned int depth)
{
    GLCHECK(glGenTextures(1, &id));
    GLCHECK(glBindTexture(GL_TEXTURE_3D, id));
    GLCHECK(glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8UI, width, height, depth, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT_8_8_8_8, 0));

    GLCHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

    GLCHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
}

Voxelizer::Voxelizer():
            _nbVoxels(0u, 0u, 0u),
            _minCorner(0.f, 0.f, 0.f),
            _maxCorner(0.f, 0.f, 0.f),
            _framebufferId(-1)
{
    GLCHECK(glGenFramebuffers(1, &_framebufferId));

    /* Shader loading */
    if (!_sliceShader.loadFromFile("shaders/flatSlice.vert", "shaders/flatSlice.frag")) {
        std::cerr << "Error: couldn't load flatSlice shader." << std::endl;
    }
    if (!_compileShader.loadFromFile("shaders/compileProjections.vert", "shaders/compileProjections.frag")) {
        std::cerr << "Error: couldn't load compileProjections shader." << std::endl;
    }
}

Voxelizer::~Voxelizer()
{
    if (_framebufferId != (GLuint)(-1)) {
        GLCHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        GLCHECK(glDeleteFramebuffers(1, &_framebufferId));
    }
}

void Voxelizer::recompute(MeshRenderable& mesh, unsigned int resolution)
{
    computeGridSize(mesh, resolution);

    _voxels.resize(_nbVoxels.x * _nbVoxels.y * _nbVoxels.z / 32, 0u);
    std::fill(_voxels.begin(), _voxels.end(), 0u);

    computeVoxels(mesh);
}

void Voxelizer::computeGridSize(MeshRenderable const& mesh, unsigned int resolution)
{
    glm::vec3 minCoords = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 maxCoords = glm::vec3(std::numeric_limits<float>::min());
    for (glm::vec3 const& v : mesh.vertices()) {
        minCoords  = glm::min(minCoords, v);
        maxCoords  = glm::max(maxCoords, v);
    }

    glm::vec3 boundingBoxSize = maxCoords - minCoords;
    glm::vec3 boundingBoxCenter = 0.5f * (maxCoords + minCoords);

    float smallestSide = std::min(boundingBoxSize.x, std::min(boundingBoxSize.y, boundingBoxSize.z));
    _voxelSize = smallestSide / (float)resolution;

    _nbVoxels = boundingBoxSize / _voxelSize;
    _nbVoxels.x = makeMultipleOf32(_nbVoxels.x);
    _nbVoxels.y = makeMultipleOf32(_nbVoxels.y);
    _nbVoxels.z = makeMultipleOf32(_nbVoxels.z);

    _minCorner = boundingBoxCenter - 0.5f * glm::vec3(_nbVoxels) * _voxelSize;
    _maxCorner = boundingBoxCenter + 0.5f * glm::vec3(_nbVoxels) * _voxelSize;
}

void Voxelizer::computeVoxels(MeshRenderable& mesh)
{
    if (_framebufferId == (GLuint)(-1)) {
        std::cerr << "Voxels couldn't be computed: invalid framebuffer." << std::endl;
        return;
    }
    if (!_sliceShader.isValid() || !_compileShader.isValid()) {
        std::cerr << "Voxels couldn't be computed: invalid shader." << std::endl;
        return;
    }

    /* Saving current state for later restoration */
    const glm::mat4 modelMatrix = mesh.modelMatrix();
    GLint previousFramebufferId;
    GLCHECK(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFramebufferId));

    GLCHECK(glBindFramebuffer(GL_FRAMEBUFFER, _framebufferId));
    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    GLCHECK(glDrawBuffers(1, drawBuffers));

    ShaderProgram::bind(_sliceShader);
    GLCHECK(glDisable(GL_DEPTH_TEST));
    GLCHECK(glDisable(GL_BLEND));
    GLCHECK(glDisable(GL_CULL_FACE));
    GLCHECK(glEnable(GL_COLOR_LOGIC_OP));
    GLCHECK(glLogicOp(GL_OR));
    //GLCHECK(glEnable(GL_TEXTURE_3D));
    GLCHECK(glClearColor(0.f, 0.f, 0.f, 0.f));

    /* Allocated bigger than needed to avoid resizing it */
    std::vector<uint32_t> CPUBuffer(_nbVoxels.x * _nbVoxels.y * _nbVoxels .z / 32);

    /* Textures allocation */
    GLuint xProjTextureId = 0, yProjTextureId = 0, zProjTextureId = 0;

    /* Projection on (Y,Z) planes (X axis)*/
    allocate3DTexture(xProjTextureId, _nbVoxels.y, _nbVoxels.z, _nbVoxels.x/32);
    for (unsigned int sliceX = 0 ; sliceX < _nbVoxels.x ; sliceX+=32) {
        GLCHECK(glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, xProjTextureId, 0, sliceX/32));
        GLCHECK(glViewport(0, 0, _nbVoxels.y, _nbVoxels.z));
        GLCHECK(glClear(GL_COLOR_BUFFER_BIT));
        GLCHECK(glClearColor(0.f, 0.f, 0.f, 0.f));

        drawSlice(mesh, Axis::X, sliceX);
    }

    /* Projection on (X,Z) planes (Y axis) */
    allocate3DTexture(yProjTextureId, _nbVoxels.x, _nbVoxels.z, _nbVoxels.y/32);
    for (unsigned int sliceY = 0 ; sliceY < _nbVoxels.y ; sliceY+=32) {
        GLCHECK(glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, yProjTextureId, 0, sliceY/32));
        GLCHECK(glViewport(0, 0, _nbVoxels.x, _nbVoxels.z));
        GLCHECK(glClear(GL_COLOR_BUFFER_BIT));
        GLCHECK(glClearColor(0.f, 0.f, 0.f, 0.f));

        drawSlice(mesh, Axis::Y, sliceY);
    }

    /* Projection on (X,Y) planes (Z axis) */
    allocate3DTexture(zProjTextureId, _nbVoxels.x, _nbVoxels.y, _nbVoxels.z/32);
    for (unsigned int sliceZ = 0 ; sliceZ < _nbVoxels.z ; sliceZ+=32) {
        GLCHECK(glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, zProjTextureId, 0, sliceZ/32));
        GLCHECK(glViewport(0, 0, _nbVoxels.x, _nbVoxels.y));
        GLCHECK(glClear(GL_COLOR_BUFFER_BIT));
        GLCHECK(glClearColor(0.f, 0.f, 0.f, 0.f));

        drawSlice(mesh, Axis::Z, sliceZ);
    }

    /* Finally we compile the 3 projection into one, reusing the (X,Y) plane texture */
    {
        GLuint vaoId;
        GLCHECK(glGenVertexArrays(1, &vaoId));
        GLCHECK(glBindVertexArray(vaoId));

        ShaderProgram::bind(_compileShader);

        GLuint sliceULoc = _compileShader.getUniformLocation("slice");
        GLuint gridSizeULoc = _compileShader.getUniformLocation("gridSize");
        if (sliceULoc == (GLuint)(-1) || gridSizeULoc == (GLuint)(-1))
            std::cerr << "fjozeijfozei" << std::endl;
        GLCHECK(glUniform3f(gridSizeULoc, _nbVoxels.x, _nbVoxels.y, _nbVoxels.z));

        GLuint xProjTexLoc = _compileShader.getUniformLocation("xProjTex");
        GLuint yProjTexLoc = _compileShader.getUniformLocation("yProjTex");
        if (xProjTexLoc == (GLuint)(-1) || yProjTexLoc == (GLuint)(-1))
            std::cerr << "podapfhiizha" << std::endl;
        GLCHECK(glUniform1i(xProjTexLoc, 0));
        GLCHECK(glUniform1i(yProjTexLoc, 1));

        GLCHECK(glActiveTexture(GL_TEXTURE0));
        GLCHECK(glBindTexture(GL_TEXTURE_3D, xProjTextureId));
        GLCHECK(glActiveTexture(GL_TEXTURE1));
        GLCHECK(glBindTexture(GL_TEXTURE_3D, yProjTextureId));

        for (unsigned int sliceZ = 0 ; sliceZ < _nbVoxels.z ; sliceZ+=32) {
            GLCHECK(glUniform1ui(sliceULoc, sliceZ));

            GLCHECK(glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, zProjTextureId, 0, sliceZ/32));
            GLCHECK(glViewport(0, 0, _nbVoxels.x, _nbVoxels.y));
            GLCHECK(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
        }

        ShaderProgram::unbind();

        GLCHECK(glDeleteVertexArrays(1, &vaoId));
    }

    /* Lastly, retreieve the result */
    GLCHECK(glBindTexture(GL_TEXTURE_3D, zProjTextureId));
    GLCHECK(glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT_8_8_8_8, _voxels.data()));

    /* Textures desallocation */
    GLCHECK(glBindTexture(GL_TEXTURE_3D, 0));
    GLCHECK(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0));
    GLCHECK(glDeleteTextures(1, &xProjTextureId));
    GLCHECK(glDeleteTextures(1, &yProjTextureId));
    GLCHECK(glDeleteTextures(1, &zProjTextureId));

    /* Restoring previous state */
    GLCHECK(glDisable(GL_COLOR_LOGIC_OP));

    ShaderProgram::unbind();

    mesh.modelMatrix() = modelMatrix;
    GLCHECK(glBindFramebuffer(GL_FRAMEBUFFER, previousFramebufferId));
}

void Voxelizer::drawSlice(MeshRenderable& mesh, Axis axis, unsigned int slice)
{
    glm::mat4 viewProj, rot;

    if (axis == Axis::X) {
        float x = _minCorner.x + _voxelSize * (float)(slice);
        viewProj = glm::ortho(_minCorner.y, _maxCorner.y, //left right
                              _minCorner.z, _maxCorner.z, //bottom top
                              x, x + 32.f*_voxelSize); //near far

        rot = glm::rotate(3.1415f/2.f, glm::vec3(0,0,1));
        rot = glm::rotate(-3.1415f/2.f, glm::vec3(1,0,0)) * rot;
    } else if (axis == Axis::Y) {
        float y = _minCorner.y + _voxelSize * (float)(slice);
        viewProj = glm::ortho(_minCorner.x, _maxCorner.x, //left right
                              _minCorner.z, _maxCorner.z, //bottom top
                              y, y + 32.f*_voxelSize); //near far

        rot = glm::rotate(-3.1415f/2.f, glm::vec3(1,0,0));
        //rot = glm::rotate(3.1415f, glm::vec3(0,1,0)) * rot;
    } else { //Z
        float z = _minCorner.z + _voxelSize * (float)(slice);
        viewProj = glm::ortho(_minCorner.x, _maxCorner.x, //left right
                              _minCorner.y, _maxCorner.y, //bottom top
                              z, z + 32.f*_voxelSize); //near far

        rot = glm::rotate(3.1415f, glm::vec3(0,0,1));
        rot = glm::rotate(3.1415f, glm::vec3(0,1,0)) * rot;
    }
    mesh.modelMatrix() = rot;

    GLuint viewProjULoc = _sliceShader.getUniformLocation("viewProjMatrix");
    if(viewProjULoc != ShaderProgram::nullLocation) {
        GLCHECK(glUniformMatrix4fv(viewProjULoc, 1, GL_FALSE, glm::value_ptr(viewProj)));
    }

    mesh.draw(_sliceShader);
}

glm::uvec3 const& Voxelizer::getNbVoxels() const
{
    return _nbVoxels;
}

float Voxelizer::getVoxelSize() const
{
    return _voxelSize;
}

glm::vec3 Voxelizer::voxelPosition(unsigned int iX, unsigned int iY, unsigned int iZ) const
{
    return _minCorner + _voxelSize * glm::vec3(iX,iY,iZ) + .5f * glm::vec3(_voxelSize);
}

std::vector<uint32_t> const& Voxelizer::grid() const
{
    return _voxels;
}
