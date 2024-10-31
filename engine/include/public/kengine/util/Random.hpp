#pragma once
#include <iostream>
#include <random>
#include <thread>

namespace ke {
    namespace random {
        inline static thread_local std::mt19937 rng(std::random_device{}());

        /// <returns>a random float between 0 and 1.</returns>
        inline static float rand() {
            std::uniform_real_distribution<float> dist(0.0f, 1.0f);
            return dist(rng);
        }

        inline static int32_t randInt(int32_t from, int32_t to) {
            std::uniform_int_distribution<int32_t> dist(from, to);
            return dist(rng);
        }

        inline static float randFloat(float from, float to) {
            std::uniform_real_distribution<float> dist(from, to);
            return dist(rng);
        }
    } // namespace random
} // namespace ke