#include "ShaderProgram.hpp"


#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "GLHelper.hpp"


const GLuint ShaderProgram::nullLocation = (GLuint)(-1);


static std::string loadFile(std::string const& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: couldn't open file \"" << filename << "\"." << std::endl;
        return std::string();
    }

    std::stringstream ss;
    ss << file.rdbuf();
    file.close();

    return ss.str();
}

static GLuint compileShader(std::string const& shaderText, GLuint type)
{
    /* Check shader type */
    std::string shaderType;
    if (type == GL_VERTEX_SHADER)
        shaderType = "vertex";
    else if (type == GL_FRAGMENT_SHADER)
        shaderType = "fragment";
    else {
        std::cerr << "Error: only vertex and fragment shader are supported." << std::endl;
        return 0;
    }

    /* Shader creation */
    GLuint shaderId;
    GLCHECK(shaderId = glCreateShader(type));
    if (shaderId == 0) {
        std::cerr << "Error: couldn't create " << shaderType << " shader." << std::endl;
        return 0;
    }

    /* Sending code */
    GLchar const* shaderString[1] = {shaderText.c_str()};
    GLint const shaderLength[1] = {(GLint)shaderText.size()};
    GLCHECK(glShaderSource(shaderId, 1,  shaderString, shaderLength));

    /* Compilation */
    GLCHECK(glCompileShader(shaderId));
    GLint compileStatus;
    GLCHECK(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus));
    if (compileStatus == GL_FALSE) {
        std::cerr << "Error: couldn't compile " << shaderType << " shader." << std::endl;

        GLint logLen;
        GLCHECK(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLen));
        if(logLen > 0) {
            std::vector<char> log(logLen);
            GLsizei written;
            GLCHECK(glGetShaderInfoLog(shaderId, logLen, &written, log.data()));
            std::cerr << "== Shader log: ==\n" << log.data() << std::endl;
        }
        std::cerr << shaderText << std::endl;
        GLCHECK(glDeleteShader(shaderId));
        return 0;
    }

    return shaderId;
}

ShaderProgram::ShaderProgram():
            _programId(0),
            _valid(false)
{
}

ShaderProgram::~ShaderProgram()
{
    if (glIsProgram(_programId) == GL_TRUE) {
        GLCHECK(glDeleteProgram(_programId));
    }
}

bool ShaderProgram::loadFromFile(std::string const& vertexFilename, std::string const& fragmentFilename)
{
    std::string vertexShader = loadFile(vertexFilename);
    std::string fragmentShader = loadFile(fragmentFilename);
    return loadFromString(vertexShader, fragmentShader);
}

bool ShaderProgram::loadFromString(std::string const& vertexShader, std::string const& fragmentShader)
{
    GLuint vertexShaderId = compileShader(vertexShader, GL_VERTEX_SHADER);
    GLuint fragmentShaderId = compileShader(fragmentShader, GL_FRAGMENT_SHADER);

    if (vertexShaderId == 0 || fragmentShaderId == 0) {
        std::cerr << "Couldn't load new shader program. Shader program unchanged." << std::endl;

        if (vertexShaderId == 0) {
            GLCHECK(glDeleteShader(vertexShaderId));
        }
        if (fragmentShaderId == 0) {
            GLCHECK(glDeleteShader(fragmentShaderId));
        }
        return false;
    }

    /* We save the current shader's ID for easy restoration if building failure */
    const GLuint previousProgramId = _programId;

    GLCHECK(_programId = glCreateProgram());
    if (_programId == 0) {
        std::cerr << "Couldn't create new shader program. Shader program unchanged." << std::endl;
        _programId = previousProgramId;
        return false;
    }

    GLCHECK(glAttachShader(_programId, vertexShaderId));
    GLCHECK(glAttachShader(_programId, fragmentShaderId));
    GLCHECK(glLinkProgram(_programId));

     GLint linkStatus;
     GLCHECK(glGetProgramiv(_programId, GL_LINK_STATUS, &linkStatus));
     if (linkStatus == GL_FALSE) {
        std::cerr << "Couldn't link shader program made of vertex shader [" << vertexShader << "] and fragment shader [" << fragmentShader << "].";
        std::cerr << " Shader program unchanged." << std::endl;
        _programId = previousProgramId;
        return false;
     }

     /* If the shader was already a shader program, free previous resources. */
    if (glIsProgram(previousProgramId)) {
        GLCHECK(glDeleteProgram(previousProgramId));
    }

    GLCHECK(glDeleteShader(vertexShaderId));
    GLCHECK(glDeleteShader(fragmentShaderId));

    queryActiveAttributesLocation();
    queryActiveUniformsLocation();

    _valid = true;
    return true;
}

void ShaderProgram::bind(ShaderProgram& program)
{
    GLCHECK(glUseProgram(program._programId));
}

void ShaderProgram::unbind()
{
    GLCHECK(glUseProgram(0));
}

bool ShaderProgram::isValid() const
{
    return _valid;
}

GLuint ShaderProgram::getUniformLocation (std::string const& name) const
{
   std::unordered_map<std::string, GLint>::const_iterator result = _uniforms.find(name);
    if (result != _uniforms.end())
        return result->second;
    return ShaderProgram::nullLocation;
}

GLuint ShaderProgram::getAttribLocation (std::string const& name) const
{
    std::unordered_map<std::string, GLint>::const_iterator result = _attributes.find(name);
    if (result != _attributes.end())
        return result->second;
    return ShaderProgram::nullLocation;
}

GLuint ShaderProgram::programId()
{
    return _programId;
}

void ShaderProgram::queryActiveUniformsLocation()
{
    _uniforms.clear();
    _attributes.clear();

    /* Reading uniforms */
    GLint nbUniforms = 0, maxUniformNameLength = 0;
    GLCHECK(glGetProgramiv(_programId, GL_ACTIVE_UNIFORMS, &nbUniforms));
    GLCHECK(glGetProgramiv(_programId, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformNameLength));
    std::vector<char> name(512);//maxUniformNameLength);

    for (GLint iU = 0 ; iU < nbUniforms ; ++iU) {
        GLsizei nameLength;
        GLenum uniformType;
        GLint uniformSize;
        GLuint uniformLocation;

        GLCHECK(glGetActiveUniform(_programId, iU, name.size(), &nameLength, &uniformSize, &uniformType, name.data()));
        GLCHECK(uniformLocation = glGetUniformLocation(_programId, name.data()));

        _uniforms.emplace(std::string(name.data()), uniformLocation);
    }
}

void ShaderProgram::queryActiveAttributesLocation()
{
    /* Reading attributes */
    GLint nbAttributes = 0, maxAttributeNameLength = 0;
    GLCHECK(glGetProgramiv(_programId, GL_ACTIVE_ATTRIBUTES, &nbAttributes));
    GLCHECK(glGetProgramiv(_programId, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxAttributeNameLength));
    std::vector<char> name(512);//maxAttributeNameLength);

    for (GLint iA = 0 ; iA < nbAttributes ; ++iA) {
        GLsizei nameLength;
        GLenum attribType;
        GLint attribSize;
        GLint attribLocation;

        GLCHECK(glGetActiveAttrib(_programId, iA, name.size(), &nameLength, &attribSize, &attribType, name.data()));
        GLCHECK(attribLocation = glGetAttribLocation(_programId, name.data()));

        _attributes.emplace(std::string(name.data()), attribLocation);
    }
}
