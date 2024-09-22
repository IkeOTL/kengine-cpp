#include <kengine/io/AssetIO.hpp>
#include <kengine/EngineConfig.hpp>
#include <filesystem>

std::unique_ptr<std::istream> FileSystemAssetIO::load(const std::string& key) {
    throw std::runtime_error("Not implemented");
}

std::unique_ptr<std::ostream> FileSystemAssetIO::save(const std::string& key) {
    throw std::runtime_error("Not implemented");
}

std::unique_ptr<AssetData> FileSystemAssetIO::loadBuffer(const std::string& key) {
    auto assetPath = std::filesystem::path(EngineConfig::getInstance().getAssetRoot()) / key;

    auto mmapFile = std::make_unique<MemoryMappedFile>(assetPath.string());
    mmapFile->map();
    return std::make_unique<MemoryMappedFileAssetData>(std::move(mmapFile));
}