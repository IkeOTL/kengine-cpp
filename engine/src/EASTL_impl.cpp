#include <cstddef>
#include <new>

void* operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line) {
    return ::operator new[](size);
}

void* operator new[](std::size_t size, std::size_t, std::size_t, const char* file, int line, unsigned int, const char*, int) {
    return ::operator new[](size);
}