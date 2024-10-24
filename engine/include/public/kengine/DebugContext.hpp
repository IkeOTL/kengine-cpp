#pragma once
#include <unordered_map>
#include <string>
#include <memory>

namespace ke {
    class DebugContext {
    private:
        std::unordered_map<std::string, int> intMap;
    public:

        inline static std::unique_ptr<DebugContext> create() {
            return std::make_unique<DebugContext>();
        }

        void storeIntValue(std::string key, int v);
        int getIntValue(std::string key);

    };
} // namespace ke