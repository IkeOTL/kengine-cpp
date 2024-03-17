#pragma once
#include <glm/vec2.hpp>
#include <cstdint>

class TileSheet {
public:
    enum TileCorner {
        TOP_LEFT,
        TOP_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_RIGHT
    };

private:
    const uint32_t textureWidth, textureHeight;
    const uint32_t tileWidth, tileHeight;
    const uint32_t spacing = 0, margin = 0;

    TileSheet(uint32_t textureWidth, uint32_t textureHeight, uint32_t tileWidth, uint32_t tileHeight)
        : textureWidth(textureWidth), textureHeight(textureHeight), tileWidth(tileWidth), tileHeight(tileHeight) {}

    /*
       UV coords
        0    3
        |    |
        |    |
        1----2
     */
    glm::vec2 getUv(int tileId, TileCorner corner) {
        auto tileX = tileId % (textureWidth / tileWidth);
        auto tileY = tileId / (textureWidth / tileWidth);

        auto offsetX = (tileX * (tileWidth + spacing) + margin) / static_cast<float>(textureWidth);
        auto offsetY = (tileY * (tileHeight + spacing) + margin) / static_cast<float>(textureHeight);
        auto width = tileWidth / static_cast<float>(textureWidth);
        auto height = tileHeight / static_cast<float>(textureHeight);

        switch (corner) {
        case TileSheet::TileCorner::TOP_LEFT:
            return glm::vec2(offsetX, offsetY);
        case TileSheet::TileCorner::BOTTOM_LEFT:
            return  glm::vec2(offsetX, offsetY + height);
        case TileSheet::TileCorner::BOTTOM_RIGHT:
            return  glm::vec2(offsetX + width, offsetY + height);
        default:
            return  glm::vec2(offsetX + width, offsetY);
        };
    }
};