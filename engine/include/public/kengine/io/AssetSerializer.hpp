#pragma once
#include <cstdint>
#include <string>
#include "AssetIO.hpp"

namespace ke {
    class Serializer {
    protected:
        bool isLittleEndian() {
            uint16_t number = 0x1;
            return *(reinterpret_cast<unsigned char*>(&number)) == 1;
        }

        template<typename T>
        T swapEndian(T value) {
            static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8, "Cannot swap endianess.");

            if constexpr (sizeof(T) == 2) {
                return static_cast<T>((value << 8) | (value >> 8));
            }
            else if constexpr (sizeof(T) == 4) {
                auto temp = *reinterpret_cast<uint32_t*>(&value);
                temp = ((temp >> 24) & 0x000000FF) |
                    ((temp >> 8) & 0x0000FF00) |
                    ((temp << 8) & 0x00FF0000) |
                    ((temp << 24) & 0xFF000000);
                return *reinterpret_cast<T*>(&temp);
            }
            else if constexpr (sizeof(T) == 8) {
                auto temp = *reinterpret_cast<uint64_t*>(&value);
                temp = ((temp >> 56) & 0x00000000000000FF) |
                    ((temp >> 40) & 0x000000000000FF00) |
                    ((temp >> 24) & 0x0000000000FF0000) |
                    ((temp >> 8) & 0x00000000FF000000) |
                    ((temp << 8) & 0x000000FF00000000) |
                    ((temp << 24) & 0x0000FF0000000000) |
                    ((temp << 40) & 0x00FF000000000000) |
                    ((temp << 56) & 0xFF00000000000000);
                return *reinterpret_cast<T*>(&temp);
            }
        }
    };

    class AssetSerializer : public Serializer {
    private:
        std::ostream& output;

    public:
        uint32_t writeInt32(int32_t value);
        uint32_t writeUInt32(uint32_t value);
        uint32_t writeInt64(int64_t value);
        uint32_t writeUInt64(uint64_t value);
        uint32_t writeFloat(float value);
        uint32_t writeDouble(float value);
        uint32_t writeString(std::string value);
    };

    class AssetDeserializer : public Serializer {
    private:
        std::unique_ptr<AssetData> input;
        uint32_t pos = 0;

    public:
        void readInt32(int32_t value);
        void readUInt32(uint32_t value);
        void readInt64(int64_t value);
        void readUInt64(uint64_t value);
        void readFloat(float value);
        void readDouble(float value);
        void readString(std::string value);

        uint32_t position() const {
            return pos;
        }

        uint32_t remaining() const {
            return input->length() - pos;
        }
    };

    class SerializableAsset {
        virtual void serialize(AssetSerializer& output) = 0;
        virtual void deserialize(AssetDeserializer& input) = 0;
    };
} // namespace ke