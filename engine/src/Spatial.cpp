#include <kengine/Spatial.hpp>
#include <stdexcept>

int Spatial::getSceneId() const {
    return sceneId;
}

void Spatial::setSceneId(int sceneId) {
    this->sceneId = sceneId;
}

Spatial* Spatial::getParent() const {
    return parent.get();
}

const std::string& Spatial::getName() const {
    return name;
}

std::vector<std::shared_ptr<Spatial>> Spatial::getChildren() const {
    std::vector<std::shared_ptr<Spatial>> childList;
    for (const auto& pair : children)
        childList.push_back(pair.second);
    return childList;
}

std::shared_ptr<Spatial> Spatial::addChild(std::shared_ptr<Spatial> s) {
    if (!s)
        return nullptr;

    auto it = children.find(s->getName());
    if (it != children.end())
        it->second->removeFromParent();

    s->removeFromParent();
    s->parent = shared_from_this();
    s->forceUpdateTransform();
    children[s->getName()] = s;
    return s;
}

std::shared_ptr<Spatial> Spatial::removeChild(const std::string& name) {
    auto it = children.find(name);
    if (it == children.end())
        return nullptr;

    auto& removed = it->second;
    children.erase(it);
    if (removed)
        removed->parent = nullptr;

    return removed;
}

void Spatial::removeFromParent() {
    if (parent == nullptr)
        return;

    parent->removeChild(name);
    forceUpdateTransform();
}

void Spatial::setChangeCb(std::function<void()>&& cb) {
    changeCb = std::move(cb);
    if (changeCb)
        changeCb();
}

const Transform& Spatial::getLocalTransform() const {
    return localTransform;
}

void Spatial::setLocalTransform(const Transform& t) {
    localTransform = t;
    forceUpdateTransform();
}

Transform& Spatial::getWorldTransform() {
    checkDoTransformUpdate();
    return worldTransform;
}

void Spatial::forceUpdateTransform() {
    if (!transformDirty && changeCb)
        changeCb();

    transformDirty = true;

    for (auto& child : children)
        child.second->forceUpdateTransform();
}

void Spatial::setLocalPosition(const glm::vec3& p) {
    localTransform.setPosition(p);
    forceUpdateTransform();
}

void Spatial::setLocalScale(const glm::vec3& s) {
    localTransform.setScale(s);
    forceUpdateTransform();
}

void Spatial::setLocalRotation(const glm::quat& q) {
    localTransform.setRotation(q);
    forceUpdateTransform();
}


void Spatial::updateWorldTransforms() {
    if (parent == nullptr) {
        worldTransform = localTransform;
        worldTransform.updateTransform();
        return;
    }

    if (parent->transformDirty)
        throw std::runtime_error("Transform should not be dirty at this point");

    worldTransform = localTransform;
    worldTransform.updateTransform(parent->getWorldTransform());

    transformDirty = false;
}

void Spatial::checkDoTransformUpdate() {
    if (!transformDirty)
        return;

    Spatial* stack[100]{};
    Spatial* rootNode = this;
    int i = 0;
    while (true) {
        Spatial* hisParent = rootNode->parent.get();
        if (hisParent == nullptr) {
            rootNode->worldTransform = rootNode->localTransform;
            rootNode->worldTransform.updateTransform();
            rootNode->transformDirty = false;
            i--;
            break;
        }

        stack[i] = rootNode;

        if (!hisParent->transformDirty)
            break;

        rootNode = hisParent;
        i++;
    }

    for (int j = i; j >= 0; j--)
        stack[j]->updateWorldTransforms();
}