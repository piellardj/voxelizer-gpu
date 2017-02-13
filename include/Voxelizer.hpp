#ifndef VOXELIZER_HPP_INCLUDED
#define VOXELIZER_HPP_INCLUDED


#include <cstdint>

#include "ShaderProgram.hpp"
#include "MeshRenderable.hpp"
#include "NonCopyable.hpp"

#include "glm.hpp"


/**@brief Computes the voxelization of a mesh.
 *
 * All the computation is handled on GPU.
 * This class uses the hardware rasterization to do the voxelization.
 * It renders the mesh from different points of view, each time adjusting Znear/Zfar planes.
 * Lastly it compiles all these renders to create the final grid.
 *
 * The result is accessible through getters methods.
 * It is stored in the following binary format:
 * - each of the grid's dimensions is a multiple of 32
 * - each voxel takes exactly 1 bit, so each std::vector<uint32_t> value contains the state of 32 voxels
 * - the array is ordered like so:
 *    (X0,Y0,Z00-31)   | (X1,Y0,Z00-31)   | ... | (Xmax,Y0,Z00-31)
 *    (X0,Y1,Z00-31)   | (X1,Y1,Z00-31)   | ... | (Xmax,Y1,Z00-31)
 *    (X0,Ymax,Z00-31) | (X1,Ymax,Z00-31) | ... | (Xmax,Ymax,Z00-31)
 *
 *    (X0,Y0,Z32-63)   | (X1,Y0,Z32-63)   | ... | (Xmax,Y0,Z32-63)
 *    (X0,Y1,Z32-63)   | (X1,Y1,Z32-63)   | ... | (Xmax,Y1,Z32-63)
 *    (X0,Ymax,Z32-63) | (X1,Ymax,Z32-63) | ... | (Xmax,Ymax,Z32-63)
 * ...
 *    (X0,Y0,Z?-max)   | (X1,Y0,Z?-max)   | ... | (Xmax,Y0,Z?-max)
 *    (X0,Y1,Z?-max)   | (X1,Y1,Z?-max)   | ... | (Xmax,Y1,Z?-max)
 *    (X0,Ymax,Z?-max) | (X1,Ymax,Z?-max) | ... | (Xmax,Ymax,Z?-max)
 *
 * The result is viewable using the VoxelsRenderable class.
 */
class Voxelizer: NonCopyable
{
    public:
        /**@brief Constructor. Prepares the computation and initializes the grid to be empty. */
        Voxelizer ();
        ~Voxelizer();

        void recompute(MeshRenderable& mesh, unsigned int resolution);

        /**@brief Access to the raw grid data. */
        std::vector<uint32_t> const& grid() const;

        /**@brief The grid dimensions. */
        glm::uvec3 const& getNbVoxels() const;

        /**@brief The size of a voxel, to fit the voxel grid on the original mesh. */
        float getVoxelSize() const;

        /**@brief The position of a voxel in the mesh coordinates. */
        glm::vec3 voxelPosition(unsigned int iX, unsigned int iY, unsigned int iZ) const;


    private:
        /**@brief Computes the optimal 3D grid dimensions. */
        void computeGridSize(MeshRenderable const& mesh, unsigned int resolution);

        /**@brief Fills the 3D grid */
        void computeVoxels(MeshRenderable& mesh);

        enum class Axis {X, Y, Z};
        void drawSlice (MeshRenderable& mesh, Axis axis, unsigned int slice);


    private:
        glm::uvec3 _nbVoxels; //should be multiples of 4
        float _voxelSize;

        glm::vec3 _minCorner;
        glm::vec3 _maxCorner;

        std::vector<uint32_t> _voxels;

        GLuint _framebufferId;
        ShaderProgram _sliceShader;
        ShaderProgram _compileShader;
};


#endif // VOXELIZER_HPP_INCLUDED
