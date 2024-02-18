#pragma once
#include <kengine/Transform.hpp>
#include <memory>
#include <string>
#include <vector>
#include <functional>

class Spatial : public std::enable_shared_from_this<Spatial> {
private:
    int sceneId;
    std::function<void()> changeCb;

    std::shared_ptr<Spatial> parent = nullptr;
    std::string name;

    std::unordered_map<std::string, std::shared_ptr<Spatial>> children;

    Transform localTransform;
    Transform worldTransform;

    bool transformDirty = true;

protected:
    Spatial(int sceneId, const std::string& name) : sceneId(sceneId), name(name) {}

public:
    Spatial(const std::string& name) : Spatial(-1, name) {}
    ~Spatial() = default;

    int getSceneId() const;
    void setSceneId(int sceneId);
    Spatial* getParent() const;
    const std::string& getName() const;
    std::vector<std::shared_ptr<Spatial>> getChildren() const;
    std::shared_ptr<Spatial> addChild(std::shared_ptr<Spatial> s);
    std::shared_ptr<Spatial> removeChild(const std::string& name);
    void removeFromParent();
    void setChangeCb(std::function<void()>&& cb);
    Transform& getLocalTransform();
    void setLocalTransform(const Transform& t);
    const Transform& getWorldTransform();
    void forceUpdateTransform();
    void setLocalPosition(const glm::vec3& p);
    void setLocalScale(const glm::vec3& s);
    void setLocalRotation(const glm::quat& q);

private:
    void updateWorldTransforms();
    void checkDoTransformUpdate();
};