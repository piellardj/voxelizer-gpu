#version 330
#define M_PI 3.14159265358979323846


uniform sampler2D envMap;

in vec3 fromCameraVector;

out vec4 fragColor;


/* Prend un vecteur normalisé.
 * Retourne les coordonnées (angle plan xy, angle vertical) */
vec2 polarCoords (vec3 vector)
{
    vector = normalize(vector);
    return vec2(atan(vector.y, vector.x) / (2.0*M_PI)  + 0.5,
                acos(vector.z) / M_PI);
}

vec4 sampleEnvMap (vec3 direction)
{
    vec2 coords = polarCoords(direction);
    //coords.y = 1 - coords.y; //SFML inverts textures
    return texture(envMap, coords);
}

void main()
{
    fragColor = sampleEnvMap(normalize(fromCameraVector));
}
