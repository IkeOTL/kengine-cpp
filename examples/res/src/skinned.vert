#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec4 inTangent;
layout (location = 4) in uvec4 inJointIndices;
layout (location = 5) in vec4 inJointWeights;

layout(set = 0, binding = 0) uniform SceneBuffer {
	mat4 proj;
	mat4 view;
	vec4 lightDir;
} sceneBuffer;

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

layout(std430, set = 2, binding = 5) readonly buffer JointMatrices {
    mat4 jointMatrices[];
} jointMatrices;

layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outNormal;
layout (location = 3) out mat3 outTbn;
layout (location = 6) flat out uint materialId;

out gl_PerVertex
{
	vec4 gl_Position;
};

mat3 calculateTbn(mat3 invXposeModelMat, vec3 normal)
{
    vec3 T = normalize(invXposeModelMat * inTangent.xyz);
    vec3 B = normalize(invXposeModelMat * (cross(inNormal.xyz, inTangent.xyz) * inTangent.w));
    vec3 N = normal;
    return mat3(T, B, N);
}

void main() {
    DrawObject obj = drawObjectBuffer.objects[drawInstanceBuffer.instanceIds[gl_InstanceIndex]];
	mat4 modelMatrix = obj.transform;
	//mat4 modelMatrix = drawObjectBuffer.objects[gl_InstanceIndex].transform;

    mat4 skinMat =
        inJointWeights.x * jointMatrices.jointMatrices[inJointIndices.x] +
        inJointWeights.y * jointMatrices.jointMatrices[inJointIndices.y] +
        inJointWeights.z * jointMatrices.jointMatrices[inJointIndices.z] +
        inJointWeights.w * jointMatrices.jointMatrices[inJointIndices.w];

    gl_Position = sceneBuffer.proj * sceneBuffer.view * modelMatrix * skinMat * vec4(inPos, 1);

    // Vertex position in world space
    outWorldPos = vec3(modelMatrix * skinMat * vec4(inPos, 1));
	
	// Normal in world space
	mat3 invXposeModelMat = transpose(inverse(mat3(modelMatrix * skinMat)));
	outNormal = normalize(invXposeModelMat * inNormal);
	    
	outTbn = calculateTbn(invXposeModelMat, outNormal);

	outUV = inUV;
    materialId = obj.materialId;
}
