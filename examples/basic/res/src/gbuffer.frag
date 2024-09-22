#version 450

layout (set = 2, binding = 0) uniform sampler2D colorMap;
layout (set = 2, binding = 1) uniform sampler2D normalMap;
layout (set = 2, binding = 2) uniform sampler2D metallicRoughnessMap;
layout (set = 2, binding = 3) uniform sampler2D aoMap;
layout (set = 2, binding = 4) uniform sampler2D emissiveMap;

layout (location = 0) in vec3 inWorldPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in mat3 inTbn;
layout (location = 6) flat in uint materialId;

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
    int albedoTextureSet;
    int metallicRoughnessTextureSet;
    int normalTextureSet;	
    int occlusionTextureSet;
    int emissiveTextureSet;
    float metallicFactor;	
    float roughnessFactor;
    int padding;
};

layout(std430, set = 1, binding = 2) readonly buffer MaterialsSsbo {
    Material materials[];
} materialsSsbo;

float linearDepth(float depth)
{
    float z = depth * 2.0f - 1.0f; 
    return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));	
}

vec3 calculateNormal(Material mat)
{
    if (mat.normalTextureSet == -1)
        return normalize(inNormal);

    vec3 tangentNormal = texture(normalMap, inUV).rgb * 2.0 - 1.0;
    return normalize(inTbn * tangentNormal);
}

vec2 signNotZero(vec2 v) {
  return vec2((v.x >= 0.0) ? 1.0 : -1.0, (v.y >= 0.0) ? 1.0 : -1.0);
}

vec2 encodeNormal(vec3 v) {
    vec2 p = v.xy * (1.0 / (abs(v.x) + abs(v.y) + abs(v.z)));
    return (v.z <= 0.0) ? ((1.0 - abs(p.yx)) * signNotZero(p)) : p;
}

vec2 calculatePackedNormal(Material mat)
{
    return encodeNormal(calculateNormal(mat));
}

vec4 getAlbedo(Material mat) {
    vec4 albedo = mat.albedoFactor;

    if (mat.albedoTextureSet != -1)
        albedo *= texture(colorMap, inUV) * vec4(1.0);

    return albedo;
}

float getAo(Material mat) {    
    if (mat.occlusionTextureSet == -1)
        return 1.0;

    return texture(aoMap, inUV).r;
}

vec2 getMr(Material mat) {
    float metallic = mat.metallicFactor; // g
    float roughness = mat.roughnessFactor; // r

    if (mat.metallicRoughnessTextureSet != -1) {
        vec3 orm = texture(metallicRoughnessMap, inUV).rgb;

        metallic *= orm.b;
        roughness *= orm.g;
    }

    return vec2(roughness, metallic);
}

vec4 getEmissive(Material mat) {    
    if (mat.emissiveTextureSet == -1)
        return vec4(0);

    return vec4(texture(emissiveMap, inUV).rgb, 1) * mat.emissiveFactor;
}

void main() 
{
    Material mat = materialsSsbo.materials[materialId];

    outPosition = vec4(inWorldPos, 1.0);
    outPosition.a = linearDepth(gl_FragCoord.z);

    outNormal = calculatePackedNormal(mat);
    outAoMetallicRoughness = vec4(getAo(mat), getMr(mat), materialId); // float(materialId)
    outEmissive = getEmissive(mat);
    outAlbedo = getAlbedo(mat);

    outColor = vec4(0.0);
}