#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>

namespace ke {
    class Camera {

    private:
        glm::vec3 prevPosition{};
        glm::quat prevRotation = glm::identity<glm::quat>();

        glm::vec3 position{};
        glm::quat rotation = glm::identity<glm::quat>();

        glm::mat4 projection = glm::mat4(1.0f);

        const float fov, zNear, zFar, aspectRatio;

    public:
        inline static const float NEAR_CLIP = 0.01f;
        inline static const float FAR_CLIP = 150.0f;

        Camera(float fov, float aspectRatio, float zNear, float zFar);

        static inline const std::unique_ptr<Camera> create(float fov, float aspectRatio, float zNear, float zFar) {
            return std::make_unique<Camera>(fov, aspectRatio, zNear, zFar);
        }

        void getViewMatrix(glm::mat4& dest);
        void savePreviousTransform();
        void getIntegratedViewMatrix(float alpha, glm::mat4& dest);

        float getFov() const {
            return fov;
        }

        float getNearClip() const {
            return zNear;
        }

        float getFarClip() const {
            return zFar;
        }

        float getAspectRatio() const {
            return aspectRatio;
        }

        const glm::mat4& getProjectionMatrix() const {
            return projection;
        }

        const glm::vec3& getPosition() const {
            return position;
        }
        const void addPosition(glm::vec3& v) {
            position += v;
        }

        const void setPosition(const glm::vec3& p) {
            position = p;
        }

        const glm::quat& getRotation() const {
            return rotation;
        }

        const void setRotation(const glm::quat& q) {
            rotation = q;
        }

        static size_t size();
    };
} // namespace ke