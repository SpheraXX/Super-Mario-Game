#include "View/Level/PipeRenderer.h"

#include "Model/Level/Pipe.h"
#include "Model/Map/TileMap.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <cmath>

namespace view {

namespace {
// The 16x16 pipe tiles in super_mario_asset.png (inherited source scale).
// Entrance cap at (112,192) (the opening), plain body tile at (128,192) below it.
constexpr int EntranceTileX = 112;
constexpr int EntranceTileY = 192;
constexpr int BodyTileX = 128;
constexpr int BodyTileY = 192;
constexpr int TilePx = 16;
}

PipeRenderer::PipeRenderer()
    : textureLoaded(texture.loadFromFile("assets/super_mario_asset.png")) {
    texture.setSmooth(false);
}

void PipeRenderer::renderTyped(sf::RenderTarget& window, const model::Pipe& pipe,
                               const RenderContext& /* ctx */) const {
    if (!textureLoaded) return;

    const float x0 = pipe.getPosition().x;
    const float y0 = pipe.getPosition().y;
    const float tile = static_cast<float>(model::TileMap::TileWidth);
    const int cols = std::max(1, static_cast<int>(pipe.getSize().x / tile));
    const int rows = std::max(1, static_cast<int>(pipe.getSize().y / tile));

    for (int row = 0; row < rows; ++row) {
        const int tileX = (row == 0) ? EntranceTileX : BodyTileX;
        const int tileY = (row == 0) ? EntranceTileY : BodyTileY;
        for (int col = 0; col < cols; ++col) {
            sf::Sprite sprite(texture);
            sprite.setTextureRect({{tileX, tileY}, {TilePx, TilePx}});
            sprite.setScale({2.0f, 2.0f});
            sprite.setPosition({std::round(x0 + col * tile), std::round(y0 + row * tile)});
            window.draw(sprite);
        }
    }
}

}