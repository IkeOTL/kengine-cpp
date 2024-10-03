#include <kengine/io/AssetIO.hpp>
#include <kengine/EngineConfig.hpp>
#include <filesystem>
#include <kengine/io/AssetSerializer.hpp>

std::unique_ptr<std::ostream> FileSystemAssetIO::save(const std::string& key) {
    // open stream create smart pointer and add deleter so the stream flushes and closes as expected
    throw std::runtime_error("Not implemented");
}

std::unique_ptr<AssetData> FileSystemAssetIO::load(const std::string& key) {
    auto assetPath = std::filesystem::path(EngineConfig::getInstance().getAssetRoot()) / key;

    auto mmapFile = std::make_unique<MemoryMappedFile>(assetPath.string());
    mmapFile->map();
    return std::make_unique<MemoryMappedFileAssetData>(std::move(mmapFile));
}