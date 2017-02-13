#ifndef SCENE_HPP_INCLUDED
#define SCENE_HPP_INCLUDED


#include "TrackballObject.hpp"
#include "Skysphere.hpp"
#include "MeshRenderable.hpp"
#include "Voxelizer.hpp"
#include "VoxelsRenderable.hpp"

#include <SFML/Window/Event.hpp>

#include <memory>
#include <string>


/** Class handling the scene: events and rendering management. */
class Scene
{
    public:
        /**@brief Initializes the scene.
         *
         * @arg filename The .OBJ file to load and voxelize.
         */
        Scene(unsigned int screenWidth, unsigned int screenHeight,
              std::string const& filename);
        ~Scene();

        /**@brief For parameters adjustments */
        void handleEvents (sf::Event const& event);

        /**@brief Moves the camera or the light.
         * @arg relativeMovement supposed to be normalized window coordinates. */
        void mouseMoved(sf::Vector2f const& relativeMovement);

        /**@brief Tells whether or not the scene needs to be redrawn (camera or light moved, voxelization level changed. */
        bool shouldRedraw() const;

        /**@brief Draws in the current OpenGL context. */
        void draw() const;

    private:
        /**@brief Recomputes the rasterization and updates the rendering. */
        void recompute();
        void doDraw() const;
        void sendCameraLight(ShaderProgram& shader) const;


    private:
        float _aspectRatio;
        TrackballObject _camera;
        TrackballObject _light;

        glm::mat4 _projMatrix;
        glm::mat4 _viewMatrix;

        Skysphere _skysphere;
        MeshRenderable _mesh;

        Voxelizer _voxelizer;
        std::unique_ptr<VoxelsRenderable> _voxelsRenderer;

        int _precision;

        mutable ShaderProgram _shader;
        mutable ShaderProgram _voxelsShader;

public:
        mutable bool _needToRedraw;
};

#endif // SCENE_HPP_INCLUDED
