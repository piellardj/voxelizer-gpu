#version 330


uniform usampler3D xProjTex; //(y, z, x)
uniform usampler3D yProjTex; //(x, z, y)

uniform vec3 gridSize;

in vec3 texelCoord; //voxel coordinates, non normalized

out uvec4 fragColor;


uvec4 byteToColor(const uint v)
{
    uint r = v >> 24u;
    uint g = (v & 0x00FF0000u) >> 16u;
    uint b = (v & 0x0000FF00u) >> 8u;
    uint a = v & 0x000000FFu;
    
    return uvec4(r, g, b, a);
}

uint colorToByte(const uvec4 color)
{
    return (uint(color.r) << 24u) |
           (uint(color.g) << 16u) |
           (uint(color.b) << 8u)  |
            uint(color.a);
}

void main()
{
    vec3 texelSize = 1.0 / (gridSize-1.0);
    
    uint x = uint(texelCoord.x);
    uint y = uint(texelCoord.y);
    uint z = uint(texelCoord.z);
    
    uint iX = x / 32u;
    uint bX = x % 32u;
    
    uint iY = y / 32u;
    uint bY = y % 32u;
    
    uint xProj = 0u, yProj = 0u;
    vec3 coords;
    uint v;
    for (uint bZ = 0u ; bZ < 32u ; ++bZ) {
        /* Reading X projection texture */
        coords = vec3(y, z + bZ, iX) * vec3(texelSize.yz, 32.0 * texelSize.x);
        v = colorToByte(texture(xProjTex, coords));
        if ((v & (1u << bX)) != 0u)
            xProj = xProj | (1u << bZ);
        
        /* Reading Y projection texture */
        coords = vec3(x, z + bZ, iY) * vec3(texelSize.xz, 32.0*texelSize.y);
        v = colorToByte(texture(yProjTex, coords));
        if ((v & (1u << bY)) != 0u)
            yProj = yProj | (1u << bZ);
    }
    
    fragColor = byteToColor(xProj | yProj);
    
    //fragColor = uvec4(0, y, 0, 0);
    //fragColor = byteToColor(2355425u);//uvec4(0u);
}
