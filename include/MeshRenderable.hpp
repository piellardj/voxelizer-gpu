#ifndef MESH_RENDERABLE_HPP_INCLUDED
#define MESH_RENDERABLE_HPP_INCLUDED


#include <string>
#include <vector>

#include "glm.hpp"
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include "ShaderProgram.hpp"


/**@brief Simple class for mesh loading, handling and rendering.
 * Only manages vertices positions, normals and indexing.
 */
class MeshRenderable
{
    public:
        /**@brief Loads a .OBJ file. */
        MeshRenderable(std::string const& filename);
        ~MeshRenderable();

        glm::mat4& modelMatrix();

        std::vector<glm::vec3> const& vertices() const;
        std::vector<glm::vec3> const& normals() const;
        std::vector<glm::ivec3> const& indices() const;

        /**@brief Draws in the current OpenGL context.
         * Sends vertices positions to location 0.
         * Sends vertices normals to location 1. */
        void draw(ShaderProgram& shader) const;


    private:
        std::vector<glm::vec3> _vertices;
        std::vector<glm::vec3> _normals;
        std::vector<glm::ivec3> _indices;

        GLuint _verticesNormalsBufferId; //interleaved (VNVNVN...)
        GLuint _indicesBufferId;
        GLuint _vaoId;

        glm::mat4 _modelMatrix;
};

#endif //MESH_RENDERABLE_HPP_INCLUDED
