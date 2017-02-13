#version 330


uniform uint slice;
uniform vec3 gridSize;

out vec3 texelCoord; //voxel coordinates, non normalized


const vec2 corners[4] = vec2[](vec2(0,0),
                               vec2(1,0),
                               vec2(0,1),
                               vec2(1,1));

void main()
{
    vec2 corner = corners[gl_VertexID];
    texelCoord = vec3(corner * gridSize.xy, slice);
    
    gl_Position = vec4(corner*2-1, 0, 1);
}
