#version 450

layout(set = 0, binding = 0) uniform SceneBuffer {
    mat4 proj;
    mat4 view;
    vec4 lightDir;
} sceneBuffer;

struct DrawObject {
    uvec4 materialId;
};

layout(std430, set = 1, binding = 0) readonly buffer DrawObjectBuffer {
    DrawObject objects[];
} drawObjectBuffer;

layout(std430, set = 1, binding = 1) readonly buffer DrawInstanceBuffer {
    uint instanceIds[];
} drawInstanceBuffer;

layout(push_constant) uniform PushConstants {
    uvec2 chunkDimensions;
    uvec2 chunkCount;
    uvec2 tilesheetDimensions;
    uvec2 tileDimensions;
} pcs;

layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec2 outUV;
layout (location = 4) flat out uvec2 materialId;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main() {
    uint chunkId = drawInstanceBuffer.instanceIds[gl_InstanceIndex];
    DrawObject obj = drawObjectBuffer.objects[chunkId];

    vec3 vertPos;

    gl_Position = sceneBuffer.proj * sceneBuffer.view * vec4(vertPos, 1);

    // Vertex position in world space
    outWorldPos = vertPos;

    vec2 calcedUV;
    outUV = calcedUV;
    uint tileMaterialIdx; // need to pull out of packed terrain ssbo
    materialId = uvec2(obj.materialId[tileMaterialIdx], tileMaterialIdx);
}
