#include <kengine/io/AssetIO.hpp>

std::unique_ptr<AssetData> FileSystemAssetIO::loadBuffer(const std::string& key) {
    auto mmapFile = std::make_unique<MemoryMappedFile>(key);
    mmapFile->map();

    auto derived = std::make_unique<MemoryMappedFileAssetData>(std::move(mmapFile));

    return std::unique_ptr<AssetData>(static_cast<AssetData*>(derived.release()));
}