#pragma once

class EngineConfig {
public:
    static EngineConfig& getInstance() {
        static EngineConfig instance;
        return instance;
    }

    bool isDebugRenderingEnabled() const {
        return debugRenderingEnabled;
    }

    void setDebugRenderingEnabled(bool value) {
        debugRenderingEnabled = value;
    }

    std::string getAssetRoot() const {
        return assetRoot;
    }

    void setAssetRoot(std::string path) {
        assetRoot = path;
    }

private:
    EngineConfig() = default;

    std::string assetRoot = "res/";
    bool debugRenderingEnabled = false;
};