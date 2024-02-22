#include <kengine/io/AssetIO.hpp>

std::unique_ptr<std::istream> FileSystemAssetIO::load(const std::string& key) {
    throw std::runtime_error("Not implemented");
}

std::unique_ptr<std::ostream> FileSystemAssetIO::save(const std::string& key) {
    throw std::runtime_error("Not implemented");
}

std::unique_ptr<AssetData> FileSystemAssetIO::loadBuffer(const std::string& key) {
    auto mmapFile = std::make_unique<MemoryMappedFile>(key);
    mmapFile->map();
    return std::make_unique<MemoryMappedFileAssetData>(std::move(mmapFile));
}