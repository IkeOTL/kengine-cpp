#include <kengine/io/AssetIO.hpp>

std::unique_ptr<AssetData> FileSystemAssetIO::loadBuffer(const std::string& key) {
    auto mmapFile = std::make_unique<MemoryMappedFile>(key);
    mmapFile->map();
    return std::make_unique<AssetData>(
        std::make_unique<MemoryMappedFileAssetData>(std::move(mmapFile))
    );
}