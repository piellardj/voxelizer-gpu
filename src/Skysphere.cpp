#include "Skysphere.hpp"


#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <SFML/Graphics/Image.hpp>

#include "GLHelper.hpp"


Skysphere::Skysphere(std::string const& texturePath):
            _envMapTexture(-1),
            _cornersBuffer(-1),
            _vaoId(-1)
{
    /* Texture loading */
    {
        sf::Image tmp;
        if (!tmp.loadFromFile(texturePath)) {
            std::cerr << "Erreur: impossible de charger " << texturePath << std::endl;
        } else {
            GLCHECK(glGenTextures(1, &_envMapTexture));
            GLCHECK(glBindTexture(GL_TEXTURE_2D, _envMapTexture));
            GLCHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tmp.getSize().x, tmp.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp.getPixelsPtr()));
            GLCHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            GLCHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            GLCHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
            GLCHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            GLCHECK(glBindTexture(GL_TEXTURE_2D, 0));
        }
    }

    /* Shaders loading */
    if (!_shader.loadFromFile("shaders/skysphere.vert", "shaders/skysphere.frag")) {
        std::cerr << "Error: unable to load shader skysphere" << std::endl;
    }

    /* VAO creation */
    GLCHECK(glGenVertexArrays(1, &_vaoId));
}

Skysphere::~Skysphere()
{
    if (_envMapTexture != (GLuint)(-1)) {
        GLCHECK(glDeleteTextures(1, &_envMapTexture));
    }
    if (_vaoId != (GLuint)(-1)) {
        GLCHECK(glDeleteVertexArrays(1, &_vaoId));
    }
    if (_cornersBuffer != (GLuint)(-1)) {
        GLCHECK(glDeleteBuffers(1, &_cornersBuffer));
    }
}

void Skysphere::draw (glm::mat4 const& viewMat, glm::mat4 const& projMat) const
{
    if (!_shader.isValid())
        return;

    glm::mat4 invProjMat = glm::inverse(projMat);
    glm::mat3 invViewMat = glm::inverse(viewMat);

    ShaderProgram::bind(_shader);

    GLuint envMapULoc = _shader.getUniformLocation("envMap");
    GLuint invProjULoc = _shader.getUniformLocation("invProjMatrix");;
    GLuint invViewULoc = _shader.getUniformLocation("invViewMatrix");

    if (envMapULoc != ShaderProgram::nullLocation) {
        GLCHECK(glUniform1i(envMapULoc, 0));
        GLCHECK(glActiveTexture(GL_TEXTURE0));
        GLCHECK(glBindTexture(GL_TEXTURE_2D, _envMapTexture));
    }
    if (invProjULoc != ShaderProgram::nullLocation) {
        GLCHECK(glUniformMatrix4fv(invProjULoc, 1, GL_FALSE, glm::value_ptr(invProjMat)));
    }
    if (invViewULoc != ShaderProgram::nullLocation) {
        GLCHECK(glUniformMatrix3fv(invViewULoc, 1, GL_FALSE, glm::value_ptr(invViewMat)));
    }

    GLCHECK(glDisable(GL_DEPTH_TEST));
    GLCHECK(glBindVertexArray(_vaoId));
    GLCHECK(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    GLCHECK(glBindVertexArray(0));
    GLCHECK(glEnable(GL_DEPTH_TEST));

    if (envMapULoc != ShaderProgram::nullLocation) {
        GLCHECK(glBindTexture(GL_TEXTURE_2D, 0));
    }

    ShaderProgram::unbind();
}
