#pragma once
#include <iostream>
#include <Windows.h>

class MemoryMappedFile {
private:
    std::string fileName;

    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    LARGE_INTEGER fileSize{};
    HANDLE mapHandle = nullptr;
    LPVOID mappedView = nullptr;

public:
    MemoryMappedFile(std::string fileName)
        : fileName(fileName) {}

    ~MemoryMappedFile() {
        unmap();
    }

    void map();
    void unmap();
    const unsigned char* data() const;
    uint64_t length() const;
};