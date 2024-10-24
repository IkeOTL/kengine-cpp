#pragma once
#include <unordered_map>
#include <functional>

namespace ke {
    class MapHasher {
    public:
        template <typename Map>
        static size_t hash(const Map& map) {
            using K = typename Map::key_type;
            using V = typename Map::mapped_type;
            using KeyHasher = typename Map::hasher;

            KeyHasher keyHasher{};
            std::hash<V> valueHasher;

            size_t combined_hash = 0;
            for (const auto& pair : map) {
                size_t key_hash = keyHasher(pair.first);
                size_t value_hash = valueHasher(pair.second);
                combined_hash += key_hash ^ (value_hash + 0x9e3779b9 + (key_hash << 6) + (key_hash >> 2));
            }
            return combined_hash;
        }
    };
} // namespace ke