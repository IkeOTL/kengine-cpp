#include <kengine/DebugContext.hpp>

namespace ke {
    void DebugContext::storeIntValue(std::string key, int v) {
        intMap[key] = v;
    }

    int DebugContext::getIntValue(std::string key) {
        return intMap[key];
    }
} // namespace ke