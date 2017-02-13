#version 330


uniform vec3 light;

in vec4 worldPosition;
in vec4 normal; //normalisée (car cubes)

out vec4 fragColor;


void main()
{
    vec3 toLight = normalize(light - worldPosition.xyz);
    
    const float c = 0.2;
    float o = dot(toLight, normal.xyz)*0.5 + 0.5;
    
    fragColor = vec4(c + 0.8*o);
}
