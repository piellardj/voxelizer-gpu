#version 330


uniform mat4 modelMatrix;
uniform mat4 viewProjMatrix;

layout(location = 0) in vec3 vPosition;


void main(void)
{
    gl_Position = viewProjMatrix * modelMatrix * vec4(vPosition, 1);
}
