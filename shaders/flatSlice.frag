#version 330


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
    float z = gl_FragCoord.z * gl_FragCoord.w;
    
    uint v = 1u << uint(32.0*z);
    
    fragColor = byteToColor(v);
}
