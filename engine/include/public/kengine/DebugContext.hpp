#pragma once
#include <unordered_map>
#include <string>

class DebugContext {
private:
    std::unordered_map<std::string, int> intMap;
public:
    void storeIntValue(std::string key, int v);
    int getIntValue(std::string key);

};