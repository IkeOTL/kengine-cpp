#version 450

layout (set = 0, input_attachment_index = 0, binding = 0) uniform subpassInput samplerAlbedo;
layout (set = 0, input_attachment_index = 1, binding = 1) uniform subpassInput samplerPosition;
layout (set = 0, input_attachment_index = 2, binding = 2) uniform subpassInput samplerNormal;
layout (set = 0, input_attachment_index = 3, binding = 3) uniform subpassInput samplerOrm;
layout (set = 0, input_attachment_index = 4, binding = 4) uniform subpassInput samplerEmissive;
layout (set = 0, binding = 6) uniform sampler2DArray shadowMap;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

layout (constant_id = 0) const int MAX_NUM_LIGHTS = 2;

const float pi = 3.141592653589793;
const float minRoughness = 0.04;

struct Light {
    vec4 color;
    vec3 position;
    float radius;
};

layout(set = 0, binding = 5) uniform Lights {
    vec3 viewPos;
    int lightCount;
    mat4 viewMat;
    Light lights[MAX_NUM_LIGHTS];
} lights;

#define SHADOW_MAP_CASCADE_COUNT 4

layout(set = 0, binding = 7) uniform Cascades {
    mat4 cascadeViewProjMat[SHADOW_MAP_CASCADE_COUNT];
    vec4 cascadeSplits; // this locks us to 4 cascades max.
    vec3 lightDir;
} cascades;

vec2 signNotZero(vec2 v) {
  return vec2((v.x >= 0.0) ? 1.0 : -1.0, (v.y >= 0.0) ? 1.0 : -1.0);
}

vec3 decodeNormal(vec2 e) {
    vec3 v = vec3(e.xy, 1.0 - abs(e.x) - abs(e.y));

    if (v.z < 0)
        v.xy = (1.0 - abs(v.yx)) * signNotZero(v.xy);

    return normalize(v);
}

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
};

layout(std430, set = 0, binding = 8) readonly buffer Materials {
    Material materials[];
} materials;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = pi * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    //return F0 + (1.0 - F0) * clamp(1.0 - pow(cosTheta, 10.0), 0.0, 1.0);
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

const mat4 biasMat = mat4( 
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.5, 0.5, 0.0, 1.0 
);

float shadowTextureProj(vec4 shadowCoord, vec2 offset, uint cascadeIndex)
{
    float shadow = 1.0;
    float bias = 0.005;

    if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0) {
        float dist = texture(shadowMap, vec3(shadowCoord.st + offset, cascadeIndex)).r;
        if (shadowCoord.w > 0 && dist < shadowCoord.z - bias) {
            shadow = 0.6;
        }
    }

    return shadow;
}

const float NEAR_PLANE = 0.01f;
const float FAR_PLANE = 150.0f;

float linearDepth(float depth)
{
	float z = depth * 2.0f - 1.0f; 
	return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));	
}

void main() 
{
    vec3 fragPos = subpassLoad(samplerPosition).rgb;
    vec4 albedo = subpassLoad(samplerAlbedo);
    vec2 normal = subpassLoad(samplerNormal).rg;
    vec4 ormm = subpassLoad(samplerOrm);
    vec3 emissive = subpassLoad(samplerEmissive).rgb;

    vec3 N = decodeNormal(normal);
    vec3 V = normalize(lights.viewPos.xyz - fragPos);

    float metallic = ormm.b;
    float roughness = ormm.g;
    float ao = ormm.r;
    uint materialId = uint(ormm.a);

    Material mat = materials.materials[materialId];

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo.rgb, metallic);

    vec3 lightAgg  = vec3(0);
    // Directional light
    {
        vec3 L = -cascades.lightDir;
        vec3 H = normalize(L + V); // Half vector between both L and V

        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;

         // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;	  

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        //float attenuation = 1.0 / (distance * distance);
        //vec3 radiance = lightColors[i] * attenuation;
        vec3 radiance = vec3(1.0, 0.8, 0.6);

        // add to outgoing radiance Lo
        lightAgg += (kD * albedo.rgb / pi + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    vec3 ambient = vec3(.4) * albedo.rgb;
    vec3 color = ambient + lightAgg;

    // apply AO
    {
        const float occlusionStrength = 1.0f;
        color = mix(color, color * ao, occlusionStrength);
    }

    // Apply emissive    
    color += emissive;

    // Apply shadow
    {
        vec3 viewMatxPos =  (lights.viewMat * vec4(fragPos, 1.0)).xyz;
        // Get cascade index for the current fragment's view position
        uint cascadeIndex = 0;
        for(uint i = 0; i < SHADOW_MAP_CASCADE_COUNT - 1; ++i) {
            if(viewMatxPos.z < cascades.cascadeSplits[i]) {	
                cascadeIndex = i + 1;
            }
        }

        // Depth compare for shadowing
        vec4 shadowCoord = (biasMat * cascades.cascadeViewProjMat[cascadeIndex]) * vec4(fragPos, 1.0);	

        float shadow = 0;
        shadow = shadowTextureProj(shadowCoord / shadowCoord.w, vec2(0.0), cascadeIndex);

        color.xyz *= shadow; 

        switch(cascadeIndex) {
            case 0 : 
                color.rgb *= vec3(1.0f, 0.25f, 0.25f);
                break;
            case 1 : 
                color.rgb *= vec3(0.25f, 1.0f, 0.25f);
                break;
            case 2 : 
                color.rgb *= vec3(0.25f, 0.25f, 1.0f);
                break;
            case 3 : 
                color.rgb *= vec3(1.0f, 1.0f, 0.25f);
                break;
        }
    }

    //HDR thing
    //color = color / (color + vec3(1.0));
    //outColor = vec4(albedo.rgb, 1);
    outColor = vec4(color.rgb, 1);
}