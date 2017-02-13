#include "MeshRenderable.hpp"


#include "GLHelper.hpp"
#include "IO.hpp"


MeshRenderable::MeshRenderable(std::string const& filename):
            _verticesNormalsBufferId(-1),
            _indicesBufferId(-1),
            _vaoId(-1),
            _modelMatrix(glm::mat4(1.f))
{
    /* File loading */
    IO::readObj(filename, _vertices, _normals, _indices);

    std::vector<glm::vec3> verticesNormals(2*_vertices.size());
    for (std::size_t i = 0 ; i < _vertices.size() ; ++i) {
        verticesNormals[2*i+0] = _vertices[i];
        verticesNormals[2*i+1] = _normals[i];
    }

    /* Buffers creation */
    GLCHECK(glGenBuffers(1, &_verticesNormalsBufferId));
    GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, _verticesNormalsBufferId));
    GLCHECK(glBufferData(GL_ARRAY_BUFFER, verticesNormals.size()*sizeof(glm::vec3), verticesNormals.data(), GL_STATIC_DRAW));

    GLCHECK(glGenBuffers(1, &_indicesBufferId));
    GLCHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indicesBufferId));
    GLCHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size()*sizeof(glm::ivec3), _indices.data(), GL_STATIC_DRAW));

    /*  VAO creation and binding */
    {
        const GLint vertALoc = 0;
        const GLint normalALoc = 1;

        GLCHECK(glGenVertexArrays(1, &_vaoId));
        GLCHECK(glBindVertexArray(_vaoId));

        GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, _verticesNormalsBufferId));

        GLCHECK(glEnableVertexAttribArray(vertALoc));
        GLCHECK(glVertexAttribPointer(vertALoc, 3, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3), (void*)0));

        GLCHECK(glEnableVertexAttribArray(normalALoc));
        GLCHECK(glVertexAttribPointer(normalALoc, 3, GL_FLOAT, GL_FALSE, 2*sizeof(glm::vec3), (void*)sizeof(glm::vec3)));

        GLCHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indicesBufferId));

        GLCHECK(glBindVertexArray(0));
    }
}

MeshRenderable::~MeshRenderable()
{
    if (_vaoId != (GLuint)(-1)) {
        GLCHECK(glDeleteVertexArrays(1, &_vaoId));
    }
    if (_verticesNormalsBufferId != (GLuint)(-1)) {
        GLCHECK(glDeleteBuffers(1, &_verticesNormalsBufferId));
    }
    if (_indicesBufferId != (GLuint)(-1)) {
        GLCHECK(glDeleteBuffers(1, &_indicesBufferId));
    }
}

std::vector<glm::vec3> const& MeshRenderable::vertices() const
{
    return _vertices;
}
std::vector<glm::vec3> const& MeshRenderable::normals() const
{
    return _normals;
}
std::vector<glm::ivec3> const& MeshRenderable::indices() const
{
    return _indices;
}

glm::mat4& MeshRenderable::modelMatrix()
{
    return _modelMatrix;
}

void MeshRenderable::draw(ShaderProgram& shader) const
{
    if (!shader.isValid())
        return;

    GLuint modelMatrixULoc = shader.getUniformLocation("modelMatrix");
    if(modelMatrixULoc != ShaderProgram::nullLocation) {
        GLCHECK(glUniformMatrix4fv(modelMatrixULoc, 1, GL_FALSE, glm::value_ptr(_modelMatrix)));
    }

    GLCHECK(glBindVertexArray(_vaoId));
    GLCHECK(glDrawElements(GL_TRIANGLES, 3*_indices.size(), GL_UNSIGNED_INT, (void*)0));
    GLCHECK(glBindVertexArray(0));
}
