#pragma once
#include <kengine/vulkan/VulkanInclude.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

struct Vertex {
    glm::vec3 position;

    void setPosition(glm::vec3 v) {
        position = v;
    }

    virtual void setNormal(glm::vec3 v) {}
    virtual void setTexCoords(glm::vec2 v) {}
    virtual void setColor(glm::vec3 v) {}
    virtual void setTangent(glm::vec4 v) {}
    virtual void setBlendIndex(glm::uvec4 v) {}
    virtual void setBlendWeight(glm::vec4 v) {}

    virtual glm::vec3* getPosition() {
        return &position;
    }

    virtual glm::vec3* getNormal() {
        return nullptr;
    }

    virtual glm::vec2* getTexCoords() {
        return nullptr;
    }

    virtual glm::vec3* getColor() {
        return nullptr;
    }

    virtual glm::vec4* getTangent() {
        return nullptr;
    }

    virtual glm::uvec4* getBlendIndex() {
        return nullptr;
    }

    virtual glm::vec4* getBlendWeight() {
        return nullptr;
    }
};

struct SimpleTexturedVertex : Vertex {
    glm::vec2 texCoords;

    glm::vec2* getTexCoords() override {
        return &texCoords;
    }

    void setTexCoords(glm::vec2 v) override {
        texCoords = v;
    }
};

struct SimpleColoredVertex : Vertex {
    glm::vec3 color;

    glm::vec3* getColor() override {
        return &color;
    }

    void setColor(glm::vec3 v) override {
        color = v;
    }
};

struct TexturedVertex : SimpleTexturedVertex {
    glm::vec3 normal;
    glm::vec4 tangent;

    glm::vec3* getNormal() override {
        return &normal;
    }

    void setNormal(glm::vec3 v) override {
        normal = v;
    }

    glm::vec4* getTangent() override {
        return &tangent;
    }

    void setTangent(glm::vec4 v) override {
        tangent = v;
    }
};

struct ColoredVertex : SimpleColoredVertex {
    glm::vec3 normal;
    glm::vec4 tangent;

    glm::vec3* getNormal() override {
        return &normal;
    }

    void setNormal(glm::vec3 v) override {
        normal = v;
    }

    glm::vec4* getTangent() override {
        return &tangent;
    }

    void setTangent(glm::vec4 v) override {
        tangent = v;
    }
};

struct RiggedTexturedVertex : TexturedVertex {
    glm::uvec4 blendIndex;
    glm::vec4 blendWeight;

    glm::uvec4* getBlendIndex() override {
        return &blendIndex;
    }

    glm::vec4* getBlendWeight() override {
        return &blendWeight;
    }

    void setBlendIndex(glm::uvec4 v) override {
        blendIndex = v;
    }

    void setBlendWeight(glm::vec4 v) override {
        blendWeight = v;
    }
};

struct RiggedColoredVertex : ColoredVertex {
    glm::uvec4 blendIndex;
    glm::vec4 blendWeight;

    glm::uvec4* getBlendIndex() override {
        return &blendIndex;
    }

    glm::vec4* getBlendWeight() override {
        return &blendWeight;
    }

    void setBlendIndex(glm::uvec4 v) override {
        blendIndex = v;
    }

    void setBlendWeight(glm::vec4 v) override {
        blendWeight = v;
    }
};
