#version 450

layout(set = 0, binding = 0) uniform SceneBuffer {
    mat4 proj;
    mat4 view;
    vec4 lightDir;
} sceneBuffer;

layout(std430, set = 1, binding = 0) readonly buffer TerrainDataBuffer {
    uint packetData[]; // [17bits packing, 12bits tileInSheetId, 3bits materialIdx]
} terrainDataBuffer;

layout(std430, set = 1, binding = 1) readonly buffer DrawInstanceBuffer {
    uint instanceIds[];
} drawInstanceBuffer;

layout(push_constant) uniform PushConstants {
    uvec2 chunkDimensions;
    uvec2 chunkCount;
    uvec2 tilesheetDimensions;
    uvec2 tileDimensions;
    uvec4 materialIds; //the material Id from the manager
    vec2 worldOffset; // could probably calculate this in the shader if we need the space here
} pcs;

layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec2 outUV;
layout (location = 2) flat out uvec2 materialId;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main() {
    uint chunkId = drawInstanceBuffer.instanceIds[gl_InstanceIndex];

    vec3 chunkLocalPos = vec3(
        float(chunkId % pcs.chunkCount.x),
        0.0,
        float(chunkId / pcs.chunkCount.x)
    );
    
    vec3 chunkWorldPos = vec3(
        chunkLocalPos.x * pcs.chunkDimensions.x + pcs.worldOffset.x,
        0.0,
        chunkLocalPos.z * pcs.chunkDimensions.y + pcs.worldOffset.y
    );

    vec3 vertPos = vec3(
        float(gl_VertexIndex % pcs.chunkDimensions.x),
        0.0,
        float(gl_VertexIndex / pcs.chunkDimensions.x)
    );

    uint tileCorner = gl_VertexIndex % 4;
    vec3 cornerOffset = vec3(
        float(tileCorner == 2 || tileCorner == 3),
        0.0,
        float(tileCorner == 1 || tileCorner == 2)
    );

    vertPos += chunkWorldPos;
    vertPos += cornerOffset;

    gl_Position = sceneBuffer.proj * sceneBuffer.view * vec4(vertPos, 1);

    // Vertex position in world space
    outWorldPos = vertPos;

    uint tileId = gl_VertexIndex / 4;
    uint tileData = terrainDataBuffer.packetData[tileId];
    uint tileInSheetId = (tileData >> 3) & 0xFFF;

    float invTileSheetWidth = 1.0 / pcs.tilesheetDimensions.x;
    float invTileSheetHeight = 1.0 / pcs.tilesheetDimensions.y;

    float tileU = pcs.tileDimensions.x * invTileSheetWidth;
    float tileV = pcs.tileDimensions.y * invTileSheetHeight;

    uint tileDenom = uint(pcs.tilesheetDimensions.x / pcs.tileDimensions.x);
    uint tileX = tileInSheetId % tileDenom;
    uint tileY = tileInSheetId / tileDenom;

    float tileOffsetX = tileX * tileU;
    float tileOffsetY = tileY * tileV;

    vec2 calcedUV = vec2(
        tileOffsetX + float((tileCorner == 2 || tileCorner == 3)) * tileU,
        tileOffsetY + float((tileCorner == 1 || tileCorner == 2)) * tileV 
    );

    outUV = calcedUV;
    uint tileMaterialIdx = tileData & 0x7;
    materialId = uvec2(pcs.materialIds[tileMaterialIdx], tileMaterialIdx);
}
