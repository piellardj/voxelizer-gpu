#include <iostream>

#include <SFML/System.hpp>
#include <SFML/Window/Window.hpp>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include "GLHelper.hpp"
#include "Voxelizer.hpp"
#include "Scene.hpp"


static void initGLEW()
{
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();

    if( GLEW_OK != err )
        std::cerr << "Error while initializing GLEW: " << glewGetErrorString(err) << std::endl;

    std::cout << "Using GLEW version: " << glewGetString(GLEW_VERSION) << std::endl;
}

sf::Vector2f getRelativeMouseCoords(sf::Window const& window);
bool isMouseInWindow(sf::Window const& window);

int main(int argc, char* argv[])
{
    const float FPS = 50.f;
    std::string filename;

    /* Argument parsing */
    if (argc != 2) {
        std::cout << "Argument expected.\n";
        std::cout << "Usage: voxelizer <path to obj file>" << std::endl;
        return EXIT_SUCCESS;
    } else {
        filename = std::string(argv[1]);
        std::cout << "Opening " << filename << "...\n" << std::endl;
    }

    /* Window creation, OpenGL initialization */
    sf::ContextSettings openGL3DContext(24, 0, 4, //depth, stencil, no antialiasing
                                        3, 3, //openGL 3.0 requested
                                        sf::ContextSettings::Core);
    sf::Window window;
    window.create(sf::VideoMode(800,600), "Voxelization",
                  sf::Style::Default,
                  openGL3DContext);
    initGLEW();

    std::cout << "Using OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Using OpenGL renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Using GLSL version: " << glGetString (GL_SHADING_LANGUAGE_VERSION) << std::endl << std::endl;

    Scene scene(window.getSize().x, window.getSize().y, filename);

    sf::Clock clock;
    sf::Vector2f mousePos = getRelativeMouseCoords(window);
    while (window.isOpen()) {
        clock.restart();

        /* Drawing */
        if (scene.shouldRedraw()) {
            //window.setActive(true);
            GLCHECK(glViewport(0, 0, window.getSize().x, window.getSize().y));
            scene.draw();
            window.display();
        }

        sf::Event event;
        while (window.pollEvent(event)) {
            scene.handleEvents(event);

            switch (event.type) {
                case sf::Event::Closed:
                    window.close();
                break;
                case sf::Event::Resized:
                    GLCHECK(glViewport(0, 0, event.size.width, event.size.height));
                break;
                case sf::Event::KeyPressed:
                    if (event.key.code == sf::Keyboard::Escape) {
                        window.close();
                    }
                break;
                default:
                    break;
            }
        }

        {
            sf::Vector2f newMousePos = getRelativeMouseCoords(window);
            if (isMouseInWindow(window) && window.hasFocus()) {
                scene.mouseMoved(newMousePos - mousePos);
            }
            mousePos = newMousePos;
        }

        if (clock.getElapsedTime().asSeconds() * FPS < 1.f) {
            sf::sleep(sf::seconds(1.f/FPS - clock.getElapsedTime().asSeconds()));
        }
    }

    return EXIT_SUCCESS;
}


sf::Vector2f getRelativeMouseCoords(sf::Window const& window)
{
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);

    sf::Vector2f pos(mousePos.x, window.getSize().y - mousePos.y);
    pos.x /= static_cast<float>(window.getSize().x);
    pos.y /= static_cast<float>(window.getSize().y);
    return pos;
}

bool isMouseInWindow(sf::Window const& window)
{
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    return (mousePos.x >= 0) && (mousePos.y >= 0) &&
           (mousePos.x < (int)window.getSize().x) && (mousePos.y < (int)window.getSize().y);
}
