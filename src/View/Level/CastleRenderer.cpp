#include "View/Level/CastleRenderer.h"

#include "Model/Level/Castle.h"
#include "Model/Map/TileMap.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <cmath>

namespace view {

namespace {
// Atlas coordinates in blocks.png (16x16 source tiles).
constexpr int BrickTileX = 17 * 16;  // 272
constexpr int BrickTileY = 7 * 16;   // 112
constexpr int DoorTileX = 21 * 16;   // 336 (teal)
constexpr int DoorTileY = 13 * 16;   // 208
}

CastleRenderer::CastleRenderer()
    : textureLoaded(texture.loadFromFile("assets/blocks.png")) {
    texture.setSmooth(false);
}

void CastleRenderer::renderTyped(sf::RenderTarget& window,
                                 const model::Castle& castle) const {
    if (!textureLoaded) return;

    const float x0 = castle.getPosition().x;
    const float y0 = castle.getPosition().y;
    const float tile = static_cast<float>(model::TileMap::TileWidth);
    const int cols = std::max(1, static_cast<int>(castle.getSize().x / tile));
    const int rows = std::max(1, static_cast<int>(castle.getSize().y / tile));
    const int doorCol = cols / 2;
    const int doorRowTop = rows - 2;

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            const bool isDoor = (row >= doorRowTop) && (col == doorCol);
            sf::Sprite tileSprite(texture);
            tileSprite.setTextureRect({{isDoor ? DoorTileX : BrickTileX,
                                        isDoor ? DoorTileY : BrickTileY},
                                       {16, 16}});
            tileSprite.setScale({2.0f, 2.0f});
            tileSprite.setPosition({std::round(x0 + col * tile), std::round(y0 + row * tile)});
            window.draw(tileSprite);
        }
    }
}

}
