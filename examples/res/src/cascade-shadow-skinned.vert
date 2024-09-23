#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec4 inTangent;
layout (location = 4) in uvec4 inJointIndices;
layout (location = 5) in vec4 inJointWeights;

// todo: pass via specialization constant
#define SHADOW_MAP_CASCADE_COUNT 4

layout (set = 0, binding = 0) uniform UBO {
	mat4[SHADOW_MAP_CASCADE_COUNT] cascadeViewProjMat;
} ubo;

struct DrawObject {
	mat4 transform;
    vec4 boundingSphere;
    int materialId;
    //vec3 padding;
};

layout(std430, set = 1, binding = 0) readonly buffer DrawObjectBuffer {
	DrawObject objects[];
} drawObjectBuffer;

layout(std430, set = 1, binding = 1) readonly buffer DrawInstanceBuffer {
	uint instanceIds[];
} drawInstanceBuffer;

layout(std430, set = 2, binding = 1) readonly buffer JointMatrices {
    mat4 jointMatrices[];
} jointMatrices;

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

	mat4 skinMat =
        inJointWeights.x * jointMatrices.jointMatrices[inJointIndices.x] +
        inJointWeights.y * jointMatrices.jointMatrices[inJointIndices.y] +
        inJointWeights.z * jointMatrices.jointMatrices[inJointIndices.z] +
        inJointWeights.w * jointMatrices.jointMatrices[inJointIndices.w];

	outUV = inUV;
	gl_Position =  ubo.cascadeViewProjMat[pushConsts.cascadeIndex] * modelMatrix * skinMat * vec4(inPos, 1);
}
