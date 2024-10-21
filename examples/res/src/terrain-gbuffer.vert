#version 450

layout(set = 0, binding = 0) uniform SceneBuffer {
    mat4 proj;
    mat4 view;
    vec4 lightDir;
} sceneBuffer;

layout(std430, set = 1, binding = 0) readonly buffer TerrainDataBuffer {
    uint packedData[]; // [17bits packing, 12bits tileInSheetId, 3bits materialIdx]
} terrainDataBuffer;

layout (set = 1, binding = 1) uniform sampler2D terrainHeights;

layout(std430, set = 1, binding = 2) readonly buffer DrawInstanceBuffer {
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
    vec4 vertHeightFactor; // just to move texel calculation out
    uint tileDenom; // just to move a division out
} pcs;

layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec2 outUV;
layout (location = 2) flat out uvec2 materialId;

const float MAX_VERT_HEIGHT_SCALE = 10.0f / 0xFF;

const vec2 cornerOffsets[4] = vec2[4](
    vec2(0.0, 0.0), // tileCorner = 0
    vec2(0.0, 1.0), // tileCorner = 1
    vec2(1.0, 1.0), // tileCorner = 2
    vec2(1.0, 0.0)  // tileCorner = 3
);

out gl_PerVertex
{
    vec4 gl_Position;
};

void main() {
    uint chunkId = drawInstanceBuffer.instanceIds[gl_InstanceIndex];

    uint tileId = gl_VertexIndex >> 2; // gl_VertexIndex / 4
    uint tileCorner = gl_VertexIndex & 3; // gl_VertexIndex % 4

    uint chunkIdxX = chunkId % pcs.chunkCount.x;
    uint chunkIdxZ = chunkId / pcs.chunkCount.x;

    uvec3 chunkPos = uvec3(
        chunkIdxX * pcs.chunkDimensions.x ,
        0.0,
        chunkIdxZ * pcs.chunkDimensions.y 
    );

    vec3 chunkWorldPos = vec3(
        chunkPos.x + pcs.worldOffset.x,
        0.0,
        chunkPos.z + pcs.worldOffset.y
    );

    // Compute vertex position within the chunk
    uint chunkDimX = pcs.chunkDimensions.x;
    uint tilePosX = tileId % chunkDimX;
    uint tilePosZ = tileId / chunkDimX;

    vec3 vertPos = vec3(float(tilePosX), 0.0, float(tilePosZ));

    vec2 offset = cornerOffsets[tileCorner];
    vec3 cornerOffset = vec3(offset.x, 0.0, offset.y);

    vertPos += chunkWorldPos + cornerOffset;
    
    // apply vert height
    {
        vec2 texCoord = (vertPos.xz + pcs.vertHeightFactor.xy) * pcs.vertHeightFactor.zw;
        float h = texture(terrainHeights, texCoord).r;
        vertPos.y += h * 25.5;
    }

    gl_Position = sceneBuffer.proj * sceneBuffer.view * vec4(vertPos, 1.0);

    // Vertex position in world space
    outWorldPos = vertPos;

    // todo: potentially precompute all this into a lookup   
    uint tilesPerChunk = pcs.chunkDimensions.x * pcs.chunkDimensions.y;
    //uint globalTileId = tileId + (chunkId * tilesPerChunk); 
    uint globalTileId = (chunkPos.z + tilePosZ) * pcs.chunkDimensions.x * pcs.chunkCount.x + chunkPos.x + tilePosX;
    uint tileData = terrainDataBuffer.packedData[globalTileId];
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
