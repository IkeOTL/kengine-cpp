#pragma once

namespace ke {
    class Hashable {
    public:
        virtual size_t hashCode() const noexcept = 0;
    };
} // namespace ke