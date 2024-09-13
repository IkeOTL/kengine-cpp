#pragma once
#include <string>
#include <chrono>
#include <unordered_map>
#include <kengine/Logger.hpp>

class ProfileNode {
public:
    ProfileNode(const std::string& name)
        : name(name), totalTime(0), callCount(0) {}

    void Start() {
        startTime = std::chrono::high_resolution_clock::now();
    }

    void Stop() {
        auto endTime = std::chrono::high_resolution_clock::now();
        totalTime += std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
        callCount++;
    }

    const std::string& GetName() const { return name; }
    int64_t GetTotalTime() const { return totalTime; }
    int32_t GetCallCount() const { return callCount; }

    void Reset() {
        totalTime = 0;
        callCount = 0;
    }

private:
    std::string name;
    std::chrono::high_resolution_clock::time_point startTime;
    int64_t totalTime;  // in microseconds
    int32_t callCount;
};

class ProfileManager {
public:
    static ProfileManager& Instance() {
        static ProfileManager instance;
        return instance;
    }

    ProfileNode& GetProfileNode(const std::string& name) {
        if (profileNodes.find(name) == profileNodes.end()) {
            profileNodes[name] = ProfileNode(name);
        }
        return profileNodes[name];
    }

    void PrintResults() {
        auto& logger = kengine::LogManager::getLogger();

        logger.info("=== Profiling Results ===");
        for (const auto& pair : profileNodes) {
            const ProfileNode& node = pair.second;

            logger.info(std::format("{}: {} microseconds over {} calls.",
                node.GetName(),
                node.GetTotalTime(),
                node.GetCallCount()));
        }
    }

    void Reset() {
        for (auto& pair : profileNodes) {
            pair.second.Reset();
        }
    }

private:
    std::unordered_map<std::string, ProfileNode> profileNodes;
};