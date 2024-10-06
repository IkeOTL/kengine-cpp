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
    vec2 tileUvSize; // just to move 2 divisions out
    uint tileDenom; // just to move a division out
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
    
    vec3 chunkWorldPos = vec3(
        float(chunkId % pcs.chunkCount.x) * pcs.chunkDimensions.x + pcs.worldOffset.x,
        0.0,
        float(chunkId / pcs.chunkCount.x) * pcs.chunkDimensions.y + pcs.worldOffset.y
    );

    // todo: try to calc vert pos using globalTileId, need to figure out the func for that
    // would allow us to not need chunk worldpos
    uint tileId = gl_VertexIndex / 4;
    vec3 vertPos = vec3(
        float(tileId % pcs.chunkDimensions.x),
        0.0,
        float(tileId / pcs.chunkDimensions.x) 
    );

    uint tileCorner = gl_VertexIndex % 4;
    vec3 cornerOffset = vec3(
        float((tileCorner >> 1) & 1),
        0.0,
        float(((tileCorner + 1) >> 1) & 1)
    );

    vertPos += chunkWorldPos;
    vertPos += cornerOffset;

    gl_Position = sceneBuffer.proj * sceneBuffer.view * vec4(vertPos, 1);

    // Vertex position in world space
    outWorldPos = vertPos;

    // todo: potentially precompute all this into a lookup    
    uint globalTileId = tileId + (chunkId * pcs.chunkDimensions.x * pcs.chunkDimensions.y);
    uint tileData = terrainDataBuffer.packetData[globalTileId];
    uint tileInSheetId = (tileData >> 3) & 0xFFF;

    uint tileX = tileInSheetId % pcs.tileDenom;
    uint tileY = tileInSheetId / pcs.tileDenom;

    float tileOffsetX = tileX * pcs.tileUvSize.x;
    float tileOffsetY = tileY * pcs.tileUvSize.y;

    vec2 calcedUV = vec2(
        tileOffsetX + float((tileCorner >> 1) & 1) * pcs.tileUvSize.x,
        tileOffsetY + float(((tileCorner + 1) >> 1) & 1) * pcs.tileUvSize.y
    );

    outUV = calcedUV;
    uint tileMaterialIdx = tileData & 0x7;
    materialId = uvec2(pcs.materialIds[tileMaterialIdx], tileMaterialIdx);
}
