#pragma once
#include <kengine/Transform.hpp>
#include <memory>
#include <string>
#include <vector>
#include <functional>

class SceneGraph;

class Spatial : public std::enable_shared_from_this<Spatial> {
private:
    int32_t sceneId = -1;
    std::function<void()> changeCb = nullptr;

    std::shared_ptr<Spatial> parent = nullptr;
    const std::string name;

    std::unordered_map<std::string, std::shared_ptr<Spatial>> children;

    Transform localTransform;
    Transform worldTransform;

    bool transformDirty = true;

public:
    Spatial(const std::string& name) : name(name) {}
    ~Spatial() = default;

    int32_t getSceneId() const;
    void setSceneId(int32_t sceneId);
    Spatial* getParent() const;
    const std::string& getName() const;
    const bool hasChildren() const;
    std::vector<std::shared_ptr<Spatial>> getChildren() const;
    std::shared_ptr<Spatial> addChild(std::shared_ptr<Spatial> s);
    std::shared_ptr<Spatial> removeChild(const std::string& name);
    void removeFromParent();
    void setChangeCb(std::function<void()>&& cb);
    const Transform& getLocalTransform();
    void setLocalTransform(const glm::vec3& p, const glm::vec3& s, const glm::quat& r);
    void setLocalTransform(const Transform& t);
    Transform& getWorldTransform();
    void forceUpdateTransform();
    const glm::vec3& getPosition();
    const glm::vec3& getScale();
    const glm::quat& getRotation();
    void setLocalPosition(const glm::vec3& p);
    void setLocalScale(const glm::vec3& s);
    void setLocalRotation(const glm::quat& q);

private:
    void updateWorldTransforms();
    void checkDoTransformUpdate();
};