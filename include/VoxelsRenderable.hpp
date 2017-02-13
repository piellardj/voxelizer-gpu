#ifndef VOXELSRENDERABLE_HPP_INCLUDED
#define VOXELSRENDERABLE_HPP_INCLUDED


#include <vector>

#include "glm.hpp"
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include "ShaderProgram.hpp"
#include "Voxelizer.hpp"
#include "NonCopyable.hpp"


/**@brief Voxel grid visualization.
 *
 * Can display the grid in the current active OpenGL context. Uses instanciation to save memory.
 */
class VoxelsRenderable: NonCopyable
{
    public:
        VoxelsRenderable(Voxelizer const& voxelizer);
        ~VoxelsRenderable();

        glm::mat4& modelMatrix();
        unsigned int nbVoxels() const;

        /**@brief Draws in the current OpenGL context. */
        void draw(ShaderProgram& shader) const;

    private:
        /**@brief Appends an indexed cube mesh to the given arrays. */
        void addCube(float cubeSize, glm::vec3 center,
                     std::vector<glm::vec3>& vertices,
                     std::vector<glm::vec3>& normals,
                     std::vector<glm::ivec3>& indices);


    private:
        GLuint _verticesBufferId;
        GLuint _normalsBufferId;
        GLuint _indicesBufferId;
        GLuint _positionsBufferId; //voxel's center position

        GLuint _vaoId;

        unsigned int _nbCubes;

        glm::mat4 _modelMatrix;
};

#endif // VOXELSRENDERABLE_HPP_INCLUDED
