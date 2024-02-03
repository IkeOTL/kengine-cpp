#pragma once
#include <kengine/io/MemoryMappedFile.hpp>
#include <iostream>
#include <memory>

// 
class AssetData {
public:
    // need to add byte count or something. maybe implement javas ByteBuffer?
    virtual const unsigned char* data() = 0;
    virtual const unsigned int length() = 0;

    virtual ~AssetData() = default;
};

class AssetIO {
public:
    virtual std::unique_ptr<std::istream> load(const std::string& key) = 0;
    virtual std::unique_ptr<std::ostream> save(const std::string& key) = 0;
    virtual std::unique_ptr<AssetData> loadBuffer(const std::string& key) = 0;
};

class FileSystemAssetIO : AssetIO {
public:
    std::unique_ptr<std::istream> load(const std::string& key) override;
    std::unique_ptr<std::ostream> save(const std::string& key) override;
    std::unique_ptr<AssetData> loadBuffer(const std::string& key) override;
};

class MemoryMappedFileAssetData : public AssetData {
private:
    std::unique_ptr<MemoryMappedFile> mmapFile;

public:
    MemoryMappedFileAssetData(std::unique_ptr<MemoryMappedFile>&& mmapFile)
        : mmapFile(std::move(mmapFile)) {}

    const unsigned int length() override {
        return 1337;
    }

    const unsigned char* data() override {
        return mmapFile->data();
    }
};