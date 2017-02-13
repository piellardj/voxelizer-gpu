#include "Scene.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#define _USE_MATH_DEFINES //for windows platform
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <SFML/System/Clock.hpp>

#include "GLHelper.hpp"


Scene::Scene(unsigned int screenWidth, unsigned int screenHeight,
             std::string const& filename):
            _aspectRatio((float)screenWidth / (float)screenHeight),
            _camera(glm::vec3(0,0,0), 5.f, 0.4f, -0.5f),
            _light(glm::vec3(0,0,0), 10.f, 1.f, -2.f),
            _projMatrix(glm::perspective(95.f, _aspectRatio, 0.1f, 100.f)),
            _viewMatrix(glm::lookAt(_camera.getWorldPosition(), _camera.getFocusPoint(), glm::vec3(0,0,1))),
            _skysphere("rc/skysphere.png"),
            _mesh(filename),
            _precision(32),
            _needToRedraw(true)
{
    if (!_shader.loadFromFile("shaders/lighted.vert", "shaders/lighted.frag")) {
        std::cerr << "Error: unable to load lighted shader." << std::endl;
    }
    if (!_voxelsShader.loadFromFile("shaders/voxels.vert", "shaders/lighted.frag")) {
        std::cerr << "Error: unable to load voxels shader." << std::endl;
    }

    recompute();
}

Scene::~Scene()
{
    //_voxelizer.exportToObj("voxels.obj");
}

void Scene::recompute()
{
    _needToRedraw = true;

    sf::Clock clock;

    _voxelizer.recompute(_mesh, _precision);

    std::cout << "\nGrid size: " << _voxelizer.getNbVoxels().x << "x" << _voxelizer.getNbVoxels().y << "x" << _voxelizer.getNbVoxels().z;
    std::cout << " computed in " << clock.getElapsedTime().asSeconds() << " s\n";
    clock.restart();

    _voxelsRenderer.reset(new VoxelsRenderable(_voxelizer));

    std::cout << "Rendering preparation: " << clock.getElapsedTime().asSeconds() << " s";
    std::cout << " (" << _voxelsRenderer->nbVoxels() << " voxels)" << std::endl;
}

void Scene::handleEvents (sf::Event const& event)
{
    switch (event.type) {
        case sf::Event::Resized:
            _aspectRatio = (float)event.size.width / (float)event.size.height;
            _needToRedraw = true;
        break;
        case sf::Event::MouseWheelScrolled:
            {
                float distance = _camera.getDistance();
                distance *= 1.f + 0.02f * event.mouseWheelScroll.delta;
                _camera.setDistance(distance);
                _needToRedraw = true;
            }
        break;
        case sf::Event::KeyPressed:
            if (event.key.code == sf::Keyboard::P) {
                _precision += 32;
                recompute();
            } else if (event.key.code == sf::Keyboard::O) {
                int newPrecision = std::max(_precision-1, 2);
                if (newPrecision != _precision) {
                    _precision = newPrecision;
                    recompute();
                }
                _precision = newPrecision;
            }
        break;
        default:
            break;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        _camera.setFocusPoint(_camera.getFocusPoint() + glm::vec3(0,0,0.1));
        _needToRedraw = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        _camera.setFocusPoint(_camera.getFocusPoint() - glm::vec3(0,0,0.1));
        _needToRedraw = true;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        _camera.setLongitude(_camera.getLongitude() + 0.1f);
        _needToRedraw = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        _camera.setLongitude(_camera.getLongitude() - 0.1f);
        _needToRedraw = true;
    }

    if (_needToRedraw)
        _viewMatrix = glm::lookAt(_camera.getWorldPosition(), _camera.getFocusPoint(), glm::vec3(0,0,1));
}

void Scene::mouseMoved(sf::Vector2f const& relativeMovement)
{
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        TrackballObject *o = &_camera;
        float inversed = 1.f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))  {
            o = &_light;
            inversed = -1.f;
        }

        float latitude = o->getLatitude();
        float longitude = o->getLongitude();

        longitude -= inversed * 2.f * M_PI * relativeMovement.x;
        latitude -= inversed * 0.5f * M_PI * relativeMovement.y;

        o->setLatitude(latitude);
        o->setLongitude(longitude);
        _needToRedraw = true;

        _viewMatrix = glm::lookAt(_camera.getWorldPosition(), _camera.getFocusPoint(), glm::vec3(0,0,1));
    }
}

bool Scene::shouldRedraw() const
{
    return _needToRedraw;
}

void Scene::draw() const
{
    _needToRedraw = false;

    GLCHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GLCHECK(glClearColor(0.f, 1.f, 0.f, 0.f));

    _skysphere.draw(_viewMatrix, _projMatrix);

    if (!_shader.isValid())
        return;

    GLCHECK(glEnable(GL_DEPTH_TEST));
    GLCHECK(glEnable(GL_CULL_FACE));
    GLCHECK(glCullFace(GL_BACK));
    GLCHECK(glDisable(GL_BLEND));

    {
        ShaderProgram::bind(_shader);
        sendCameraLight(_shader);

      //  _mesh.draw(_shader);

       ShaderProgram::unbind();
    }

    if (_voxelsRenderer) {
        ShaderProgram::bind(_voxelsShader);
        sendCameraLight(_voxelsShader);

        _voxelsRenderer->draw(_shader);

        ShaderProgram::unbind();
    }
}

void Scene::sendCameraLight(ShaderProgram& shader) const
{
    glm::mat4 viewProjMatrix = _projMatrix * _viewMatrix;

    GLuint viewProjULoc = shader.getUniformLocation("viewProjMatrix");
    if(viewProjULoc != (GLuint)(-1)) {
        GLCHECK(glUniformMatrix4fv(viewProjULoc, 1, GL_FALSE, glm::value_ptr(viewProjMatrix)));
    }

    GLuint lightULoc = shader.getUniformLocation("light");
    if(lightULoc != (GLuint)(-1)) {
        GLCHECK(glUniform3f(lightULoc, _light.getWorldPosition().x, _light.getWorldPosition().y, _light.getWorldPosition().z));
    }
}
