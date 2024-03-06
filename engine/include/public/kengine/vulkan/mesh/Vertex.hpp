#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class VertexAttribute {
public:
    const static int POSITION = 1;
    const static int NORMAL = 1 << 1;
    const static int TEX_COORDS = 1 << 2;
    const static int COLOR = 1 << 3;
    const static int TANGENTS = 1 << 4;
    const static int SKELETON = 1 << 5;
    const static int POSITION_2D = 1 << 6;
};

struct Vertex {
    glm::vec3 position;

    virtual ~Vertex() = default;

    static size_t sizeOf() {
        return sizeof(Vertex);
    }
};

struct SimpleTexturedVertex : Vertex {
    glm::vec2 texCoords;

    static size_t sizeOf() {
        return sizeof(SimpleTexturedVertex);
    }
};

struct SimpleColoredVertex : Vertex {
    glm::vec3 color;

    static size_t sizeOf() {
        return sizeof(SimpleColoredVertex);
    }
};

struct TexturedVertex : SimpleTexturedVertex {
    glm::vec3 normal;
    glm::vec4 tangent;

    static size_t sizeOf() {
        return sizeof(TexturedVertex);
    }
};

struct ColoredVertex : SimpleColoredVertex {
    glm::vec3 normal;
    glm::vec4 tangent;

    static size_t sizeOf() {
        return sizeof(ColoredVertex);
    }
};

struct RiggedTexturedVertex : TexturedVertex {
    glm::uvec4 blendIndex;
    glm::vec4 blendWeight;

    static size_t sizeOf() {
        return sizeof(RiggedTexturedVertex);
    }
};

struct RiggedColoredVertex : ColoredVertex {
    glm::uvec4 blendIndex;
    glm::vec4 blendWeight;

    static size_t sizeOf() {
        return sizeof(RiggedColoredVertex);
    }
};
