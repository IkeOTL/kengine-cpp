#version 450

#define NUM_MATERIALS 4

layout (set = 2, binding = 0) uniform sampler2D colorMap[NUM_MATERIALS];
layout (set = 2, binding = 1) uniform sampler2D normalMap[NUM_MATERIALS];
layout (set = 2, binding = 2) uniform sampler2D metallicRoughnessMap[NUM_MATERIALS];
layout (set = 2, binding = 3) uniform sampler2D aoMap[NUM_MATERIALS];
layout (set = 2, binding = 4) uniform sampler2D emissiveMap[NUM_MATERIALS];

layout (location = 0) in vec3 inWorldPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) flat in uvec2 materialId; // x = SSBO Idx, y = sampler idx

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outAlbedo;
layout (location = 2) out vec4 outPosition;
layout (location = 3) out vec2 outNormal;
layout (location = 4) out vec4 outAoMetallicRoughness;
layout (location = 5) out vec4 outEmissive;

layout (constant_id = 0) const float NEAR_PLANE = 0.01f;
layout (constant_id = 1) const float FAR_PLANE = 150.0f;

struct Material {
    vec4 albedoFactor;
    vec4 emissiveFactor;
    float metallicFactor;	
    float roughnessFactor;
    uint textureSetFlags;
    int padding;
};

layout(std430, set = 1, binding = 3) readonly buffer MaterialsSsbo {
    Material materials[];
} materialsSsbo;

float linearDepth(float depth)
{
    float z = depth * 2.0f - 1.0f; 
    return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));	
}

vec4 getAlbedo(Material mat) {
    vec4 albedo = mat.albedoFactor;

    if ((mat.textureSetFlags & (1u << 0u)) != 0)
        albedo *= texture(colorMap[materialId[1]], inUV) * vec4(1.0);

    return albedo;
}

float getAo(Material mat) {
    if ((mat.textureSetFlags & (1u << 3u)) == 0)
        return 1.0;

    return texture(aoMap[materialId[1]], inUV).r;
}

vec2 getMr(Material mat) {
    float metallic = mat.metallicFactor; // g
    float roughness = mat.roughnessFactor; // r

    if ((mat.textureSetFlags & (1u << 1u)) != 0) {
        vec3 orm = texture(metallicRoughnessMap[materialId[1]], inUV).rgb;

        metallic *= orm.b;
        roughness *= orm.g;
    }

    return vec2(roughness, metallic);
}

vec4 getEmissive(Material mat) {
    if ((mat.textureSetFlags & (1u << 4u)) == 0)
        return vec4(0);

    return vec4(texture(emissiveMap[materialId[1]], inUV).rgb, 1) * mat.emissiveFactor;
}

void main() 
{
    Material mat = materialsSsbo.materials[materialId[0]];

    outPosition = vec4(inWorldPos, 1.0);
    outPosition.a = linearDepth(gl_FragCoord.z);

    outAoMetallicRoughness = vec4(getAo(mat), getMr(mat), materialId[0]);
    outEmissive = getEmissive(mat);
    outAlbedo = getAlbedo(mat);

    outColor = vec4(0.0);
}