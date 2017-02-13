#ifndef SKYSPHERE_HPP_INCLUDED
#define SKYSPHERE_HPP_INCLUDED


#include <string>

#include "glm.hpp"
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include "NonCopyable.hpp"
#include "ShaderProgram.hpp"


/**@brief Class for skysphere rendering, using a single quad. */
class Skysphere: NonCopyable
{
    public:
        Skysphere (std::string const& texturePath);
        ~Skysphere();

        /**@brief Draws in the current active OpenGL context. */
        void draw (glm::mat4 const& viewMat, glm::mat4 const& projMat) const;


    private:
        GLuint _envMapTexture;

        GLuint _cornersBuffer;
        GLuint _vaoId;

        mutable ShaderProgram _shader;
};


#endif // SKYSPHERE_HPP_INCLUDED
