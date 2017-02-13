#version 330


uniform mat4 invProjMatrix;
uniform mat3 invViewMatrix;

out vec3 fromCameraVector;


const vec2 corners[4] = vec2[](vec2(-1,-1),
                               vec2(+1,-1),
                               vec2(-1,+1),
                               vec2(+1,+1));

void main( void )
{
    vec2 corner = corners[gl_VertexID];
    
    vec3 unprojected = (invProjMatrix * vec4(corner, 0, 1)).xyz;
    fromCameraVector = invViewMatrix * unprojected;
    
    gl_Position = vec4(corner, 0, 1);
}
