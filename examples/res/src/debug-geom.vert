#version 450

layout (location = 0) in vec3 inPos;

layout(set = 0, binding = 0) uniform SceneBuffer {
	mat4 proj;
	mat4 view;
	vec4 lightDir; // not used here ... yet?
} sceneBuffer;

layout(push_constant) uniform PushConsts {
    mat4 transform;
    vec4 color;
} pushConsts;

layout (location = 0) out vec4 outColor;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() {
    gl_Position = sceneBuffer.proj * sceneBuffer.view * pushConsts.transform * vec4(inPos, 1);
    outColor = pushConsts.color;
}
