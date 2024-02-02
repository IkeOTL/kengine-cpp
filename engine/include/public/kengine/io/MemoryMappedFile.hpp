#pragma once
#include <iostream>
#include <Windows.h>

class MemoryMappedFile {
private:
    std::string fileName;

    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    HANDLE mapHandle = nullptr;
    LPVOID mappedView = nullptr;

public:
    ~MemoryMappedFile() {
        unmap();
    }

    void map();
    void unmap();
    char* data() const;
};