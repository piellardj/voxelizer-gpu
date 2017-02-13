#version 330


uniform mat4 modelMatrix;
uniform mat4 viewProjMatrix;

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec3 cubePosition;

out vec4 worldPosition;
out vec4 normal;


void main(void)
{
    worldPosition = modelMatrix * vec4(vPosition + cubePosition, 1);
    normal = modelMatrix * vec4(vNormal, 0);
    
    gl_Position = viewProjMatrix * worldPosition;
}
