#pragma once

class Hashable {
public:
    virtual size_t hashCode() const noexcept = 0;
};