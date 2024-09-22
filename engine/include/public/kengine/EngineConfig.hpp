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

private:
    EngineConfig() = default;

    bool debugRenderingEnabled = false;
};