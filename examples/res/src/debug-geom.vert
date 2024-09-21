#version 450

layout (location = 0) in vec3 inPos;

layout(set = 0, binding = 0) uniform SceneBuffer {
	mat4 proj;
	mat4 view;
	vec4 lightDir; // not used here ... yet?
} sceneBuffer;

struct DrawObject {
    mat4 transform;
    vec4 boundingSphere;
    int materialId;
    int padding[3]; // Padding to align to 16 bytes
};

layout(std430, set = 1, binding = 0) readonly buffer DrawObjectBuffer {
    DrawObject objects[];
} drawObjectBuffer;

layout(std430, set = 1, binding = 1) readonly buffer DrawInstanceBuffer {
    uint instanceIds[];
} drawInstanceBuffer;

layout(push_constant) uniform PushConsts {
    vec4 color;
} pushConsts;

layout (location = 0) out vec4 outColor;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() {
    DrawObject obj = drawObjectBuffer.objects[drawInstanceBuffer.instanceIds[gl_InstanceIndex]];
    mat4 modelMatrix = obj.transform;
    gl_Position = sceneBuffer.proj * sceneBuffer.view * modelMatrix * vec4(inPos, 1);
    outColor = pushConsts.color;
}
