#include "View/Level/PipeRenderer.h"

#include "Model/Level/Pipe.h"
#include "Model/Map/TileMap.h"

#include <SFML/Graphics/RenderTarget.hpp>

#include <algorithm>

namespace view {

namespace {
// The 16x16 pipe tiles in super_mario_asset.png (inherited source scale). The cap is a
// two-piece pair: the left half at (96,192) and the right half (with the mouth) at
// (112,192); a 1-wide pipe (fallback spawn) uses only the right half, which is the
// classic single-tile cap. The plain body tile at (128,192) fills the rows below.
constexpr int CapLeftTileX = 96;
constexpr int CapRightTileX = 112;
constexpr int CapTileY = 192;
constexpr int BodyTileX = 128;
constexpr int BodyTileY = 192;
constexpr int TilePx = 16;
}

PipeRenderer::PipeRenderer()
    : painter("assets/super_mario_asset.png") {
}

void PipeRenderer::renderTyped(sf::RenderTarget& window, const model::Pipe& pipe,
                               const RenderContext& /* ctx */) const {
    if (!painter.isLoaded()) {
        return;
    }

    const float x0 = pipe.getPosition().x;
    const float y0 = pipe.getPosition().y;
    const float tile = static_cast<float>(model::TileMap::TileWidth);
    const int cols = std::max(1, static_cast<int>(pipe.getSize().x / tile));
    const int rows = std::max(1, static_cast<int>(pipe.getSize().y / tile));

    for (int row = 0; row < rows; ++row) {
        const int tileY = (row == 0) ? CapTileY : BodyTileY;
        for (int col = 0; col < cols; ++col) {
            // Cap row: a 2-wide pipe composes the left half (col 0) + right half; a
            // 1-wide pipe falls back to the single-tile right half (the classic cap).
            const int tileX = (row == 0) ? ((cols >= 2 && col == 0) ? CapLeftTileX : CapRightTileX)
                                         : BodyTileX;
            painter.drawCell(window, {{tileX, tileY}, {TilePx, TilePx}},
                             {x0 + col * tile, y0 + row * tile});
        }
    }
}

}