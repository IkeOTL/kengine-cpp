#pragma once
#include <kengine/io/MemoryMappedFile.hpp>
#include <iostream>
#include <memory>

namespace ke {
    class AssetSerializer;

    //
    class AssetData {
    public:
        // need to add byte count or something. maybe implement javas ByteBuffer?
        virtual const unsigned char* data() const = 0;
        virtual uint64_t length() const = 0;

        virtual ~AssetData() = default;
    };

    class AssetIO {
    public:
        virtual std::unique_ptr<std::ostream> save(const std::string& key) = 0;
        virtual std::unique_ptr<AssetData> load(const std::string& key) = 0;
    };

    class FileSystemAssetIO : public AssetIO {
    public:
        inline static std::unique_ptr<AssetIO> create() {
            return std::make_unique<FileSystemAssetIO>();
        }

        std::unique_ptr<std::ostream> save(const std::string& key) override;
        std::unique_ptr<AssetData> load(const std::string& key) override;
    };

    class MemoryMappedFileAssetData : public AssetData {
    private:
        std::unique_ptr<MemoryMappedFile> mmapFile;

    public:
        MemoryMappedFileAssetData(std::unique_ptr<MemoryMappedFile>&& mmapFile)
            : mmapFile(std::move(mmapFile)) {}

        uint64_t length() const override {
            return mmapFile->length();
        }

        const unsigned char* data() const override {
            return mmapFile->data();
        }
    };
} // namespace ke