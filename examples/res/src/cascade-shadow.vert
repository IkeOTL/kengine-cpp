#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;

// todo: pass via specialization constant
#define SHADOW_MAP_CASCADE_COUNT 4

layout (set = 0, binding = 0) uniform UBO {
	mat4[SHADOW_MAP_CASCADE_COUNT] cascadeViewProjMat;
} ubo;

struct DrawObject {
	mat4 transform;
    vec4 boundingSphere;
    int materialId;
   // vec3 padding;
};

layout(std430, set = 1, binding = 0) readonly buffer DrawObjectBuffer {
	DrawObject objects[];
} drawObjectBuffer;

layout(std430, set = 1, binding = 1) readonly buffer DrawInstanceBuffer {
	uint instanceIds[];
} drawInstanceBuffer;

layout(push_constant) uniform PushConsts {
	uint cascadeIndex;
} pushConsts;

layout (location = 0) out vec2 outUV;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() {
	mat4 modelMatrix = drawObjectBuffer.objects[drawInstanceBuffer.instanceIds[gl_InstanceIndex]].transform;

	outUV = inUV;
	vec3 pos = vec3(modelMatrix * vec4(inPos, 1));
	gl_Position =  ubo.cascadeViewProjMat[pushConsts.cascadeIndex] * vec4(pos, 1.0);
}
