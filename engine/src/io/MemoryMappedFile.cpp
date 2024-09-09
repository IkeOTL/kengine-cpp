#include <kengine/io/MemoryMappedFile.hpp>


const unsigned char* MemoryMappedFile::data() const {
    return static_cast<const unsigned char*>(mappedView);
}

uint64_t MemoryMappedFile::length() const {
    return static_cast<std::uint64_t>(fileSize.QuadPart);
}

void MemoryMappedFile::map() {
    fileHandle = CreateFile(
        fileName.c_str(),
        GENERIC_READ, // Desired access
        FILE_SHARE_READ, // Share mode
        NULL, // Security attributes
        OPEN_EXISTING, // Creation disposition
        FILE_ATTRIBUTE_NORMAL, // Flags and attributes
        NULL); // Template file

    if (fileHandle == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Failed to create file handle.");

    GetFileSizeEx(fileHandle, &fileSize);

    mapHandle = CreateFileMapping(
        fileHandle,
        NULL, // Default security
        PAGE_READONLY, // Read/write access control
        0, // Maximum object size (high-order DWORD)
        0, // Maximum object size (low-order DWORD)
        NULL); // Name of mapping object

    if (mapHandle == NULL) {
        CloseHandle(fileHandle);
        throw std::runtime_error("Failed to create file mapping handle.");
    }

    mappedView = MapViewOfFile(
        mapHandle,
        FILE_MAP_READ, // Read/write permission
        0, // File offset high
        0, // File offset low
        0); // Number of bytes to map to the view

    if (mappedView == NULL) {
        CloseHandle(mapHandle);
        CloseHandle(fileHandle);
        throw std::runtime_error("Failed to create map view of file handle.");
    }
}

void MemoryMappedFile::unmap() {
    if (mappedView != nullptr) {
        UnmapViewOfFile(mappedView);
        mappedView = nullptr;
    }

    if (mapHandle != nullptr) {
        CloseHandle(mapHandle);
        mapHandle = nullptr;
    }

    if (fileHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(fileHandle);
        fileHandle = INVALID_HANDLE_VALUE;
    }
}