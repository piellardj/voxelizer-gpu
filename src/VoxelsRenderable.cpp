#include "VoxelsRenderable.hpp"


#include "GLHelper.hpp"


VoxelsRenderable::VoxelsRenderable(Voxelizer const& voxelizer):
            _verticesBufferId(-1),
            _normalsBufferId(-1),
            _indicesBufferId(-1),
            _positionsBufferId(-1),
            _vaoId(-1),
            _nbCubes(0u),
            _modelMatrix(glm::mat4(1.f))
{
    /* First create the base cube mesh */
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::ivec3> indices;

    addCube(voxelizer.getVoxelSize(), glm::vec3(0.f), vertices, normals, indices);

    /* Then parse the raw grid data to know where to instanciate the cubes */
    std::vector<glm::vec3> positions;
    glm::uvec3 nbVoxels = voxelizer.getNbVoxels();
    std::vector<uint32_t>::const_iterator it = voxelizer.grid().begin();
    for (unsigned int iZ = 0 ; iZ < nbVoxels.z ; iZ+=32) {
        for (unsigned int iY = 0 ; iY < nbVoxels.y ; ++iY) {
            for (unsigned int iX = 0 ; iX < nbVoxels.x ; ++iX) {
                uint32_t v = *it;
                if (v) {
                    for (uint32_t i = 0 ; i < 32 ; ++i)
                        if (v & (0b1 << i))
                            positions.push_back(voxelizer.voxelPosition(iX,iY,iZ+i));
                }
                ++it;
            }
        }
    }
    _nbCubes = positions.size();

    /* Buffers creation */
    GLCHECK(glGenBuffers(1, &_verticesBufferId));
    GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, _verticesBufferId));
    GLCHECK(glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW));

    GLCHECK(glGenBuffers(1, &_normalsBufferId));
    GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, _normalsBufferId));
    GLCHECK(glBufferData(GL_ARRAY_BUFFER, normals.size()*sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW));

    GLCHECK(glGenBuffers(1, &_indicesBufferId));
    GLCHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indicesBufferId));
    GLCHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(glm::ivec3), indices.data(), GL_STATIC_DRAW));

    GLCHECK(glGenBuffers(1, &_positionsBufferId));
    GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, _positionsBufferId));
    GLCHECK(glBufferData(GL_ARRAY_BUFFER, positions.size()*sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW));

    /*  VAO creation and binding */
    {
        const GLint vertALoc = 0;
        const GLint normalALoc = 1;
        const GLint positionALoc = 2;

        GLCHECK(glGenVertexArrays(1, &_vaoId));
        GLCHECK(glBindVertexArray(_vaoId));

        GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, _verticesBufferId));
        GLCHECK(glEnableVertexAttribArray(vertALoc));
        GLCHECK(glVertexAttribPointer(vertALoc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0));

        GLCHECK(glEnableVertexAttribArray(normalALoc));
        GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, _normalsBufferId));
        GLCHECK(glVertexAttribPointer(normalALoc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0));

        GLCHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indicesBufferId));

        GLCHECK(glEnableVertexAttribArray(positionALoc));
        GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, _positionsBufferId));
        GLCHECK(glVertexAttribPointer(positionALoc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0));
        GLCHECK(glVertexAttribDivisor(positionALoc, 1));

        GLCHECK(glBindVertexArray(0));
    }
}

VoxelsRenderable::~VoxelsRenderable()
{
    if (_vaoId != (GLuint)(-1)) {
        GLCHECK(glDeleteVertexArrays(1, &_vaoId));
    }
    if (_verticesBufferId != (GLuint)(-1)) {
        GLCHECK(glDeleteBuffers(1, &_verticesBufferId));
    }
    if (_normalsBufferId != (GLuint)(-1)) {
        GLCHECK(glDeleteBuffers(1, &_normalsBufferId));
    }
    if (_indicesBufferId != (GLuint)(-1)) {
        GLCHECK(glDeleteBuffers(1, &_indicesBufferId));
    }
    if (_positionsBufferId != (GLuint)(-1)) {
        GLCHECK(glDeleteBuffers(1, &_positionsBufferId));
    }
}

glm::mat4& VoxelsRenderable::modelMatrix()
{
    return _modelMatrix;
}

unsigned int VoxelsRenderable::nbVoxels() const
{
    return _nbCubes;
}

void VoxelsRenderable::addCube(float cubeSize, glm::vec3 center,
                               std::vector<glm::vec3>& vertices,
                               std::vector<glm::vec3>& normals,
                               std::vector<glm::ivec3>& indices)
{
    const unsigned int nbVertices = vertices.size();
    const unsigned int nbNormals = normals.size();

    vertices.push_back((glm::vec3(0,0,0) - glm::vec3(.5,.5,.5)) * cubeSize + center);
    vertices.push_back((glm::vec3(1,0,0) - glm::vec3(.5,.5,.5)) * cubeSize + center);
    vertices.push_back((glm::vec3(1,0,1) - glm::vec3(.5,.5,.5)) * cubeSize + center);
    vertices.push_back((glm::vec3(0,0,1) - glm::vec3(.5,.5,.5)) * cubeSize + center);

    vertices.push_back((glm::vec3(0,1,0) - glm::vec3(.5,.5,.5)) * cubeSize + center);
    vertices.push_back((glm::vec3(1,1,0) - glm::vec3(.5,.5,.5)) * cubeSize + center);
    vertices.push_back((glm::vec3(1,1,1) - glm::vec3(.5,.5,.5)) * cubeSize + center);
    vertices.push_back((glm::vec3(0,1,1) - glm::vec3(.5,.5,.5)) * cubeSize + center);

    vertices.push_back((glm::vec3(0,0,0) - glm::vec3(.5,.5,.5)) * cubeSize + center);
    vertices.push_back((glm::vec3(0,1,0) - glm::vec3(.5,.5,.5)) * cubeSize + center);
    vertices.push_back((glm::vec3(0,1,1) - glm::vec3(.5,.5,.5)) * cubeSize + center);
    vertices.push_back((glm::vec3(0,0,1) - glm::vec3(.5,.5,.5)) * cubeSize + center);

    vertices.push_back((glm::vec3(1,0,0) - glm::vec3(.5,.5,.5)) * cubeSize + center);
    vertices.push_back((glm::vec3(1,1,0) - glm::vec3(.5,.5,.5)) * cubeSize + center);
    vertices.push_back((glm::vec3(1,1,1) - glm::vec3(.5,.5,.5)) * cubeSize + center);
    vertices.push_back((glm::vec3(1,0,1) - glm::vec3(.5,.5,.5)) * cubeSize + center);

    vertices.push_back((glm::vec3(0,0,0) - glm::vec3(.5,.5,.5)) * cubeSize + center);
    vertices.push_back((glm::vec3(1,0,0) - glm::vec3(.5,.5,.5)) * cubeSize + center);
    vertices.push_back((glm::vec3(1,1,0) - glm::vec3(.5,.5,.5)) * cubeSize + center);
    vertices.push_back((glm::vec3(0,1,0) - glm::vec3(.5,.5,.5)) * cubeSize + center);

    vertices.push_back((glm::vec3(0,0,1) - glm::vec3(.5,.5,.5)) * cubeSize + center);
    vertices.push_back((glm::vec3(1,0,1) - glm::vec3(.5,.5,.5)) * cubeSize + center);
    vertices.push_back((glm::vec3(1,1,1) - glm::vec3(.5,.5,.5)) * cubeSize + center);
    vertices.push_back((glm::vec3(0,1,1) - glm::vec3(.5,.5,.5)) * cubeSize + center);

    normals.resize(nbNormals+4,  glm::vec3(0,-1,0));
    normals.resize(nbNormals+8,  glm::vec3(0,+1,0));
    normals.resize(nbNormals+12, glm::vec3(-1,0,0));
    normals.resize(nbNormals+16, glm::vec3(+1,0,0));
    normals.resize(nbNormals+20, glm::vec3(0,0,-1));
    normals.resize(nbNormals+24, glm::vec3(0,0,+1));

    indices.push_back(glm::ivec3(0,1,2) + glm::ivec3(nbVertices));
    indices.push_back(glm::ivec3(0,2,3) + glm::ivec3(nbVertices));

    indices.push_back(glm::ivec3(4,6,5) + glm::ivec3(nbVertices));
    indices.push_back(glm::ivec3(4,7,6) + glm::ivec3(nbVertices));

    indices.push_back(glm::ivec3(8,10,9) + glm::ivec3(nbVertices));
    indices.push_back(glm::ivec3(8,11,10) + glm::ivec3(nbVertices));

    indices.push_back(glm::ivec3(12,13,14) + glm::ivec3(nbVertices));
    indices.push_back(glm::ivec3(12,14,15) + glm::ivec3(nbVertices));

    indices.push_back(glm::ivec3(16,18,17) + glm::ivec3(nbVertices));
    indices.push_back(glm::ivec3(16,19,18) + glm::ivec3(nbVertices));

    indices.push_back(glm::ivec3(20,21,22) + glm::ivec3(nbVertices));
    indices.push_back(glm::ivec3(20,22,23) + glm::ivec3(nbVertices));
}

void VoxelsRenderable::draw(ShaderProgram& shader) const
{
    GLuint modelMatrixULoc = shader.getUniformLocation("modelMatrix");
    if(modelMatrixULoc != ShaderProgram::nullLocation) {
        GLCHECK(glUniformMatrix4fv(modelMatrixULoc, 1, GL_FALSE, glm::value_ptr(_modelMatrix)));
    }

    GLCHECK(glDisable(GL_BLEND));
    GLCHECK(glEnable(GL_CULL_FACE));
    GLCHECK(glCullFace(GL_BACK));

    GLCHECK(glBindVertexArray(_vaoId));
    GLCHECK(glDrawElementsInstanced(GL_TRIANGLES, 2*3*6, GL_UNSIGNED_INT, (void*)0, _nbCubes));
    GLCHECK(glBindVertexArray(0));
}
