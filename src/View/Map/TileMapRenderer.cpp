#include "View/Map/TileMapRenderer.h"

#include "Model/Map/TileMap.h"

#include <SFML/Graphics/Sprite.hpp>

#include <stdexcept>

namespace view {

namespace {
const std::string MarioAssetPath = "assets/super_mario_asset.png";
const sf::Color SceneryBackdrop(148, 148, 255);
}

TileMapRenderer::TileMapRenderer(const std::string& tilesetPath, model::WorldType worldType) {
    loadTileset(tilesetPath);
    loadTileset(MarioAssetPath);

    // Indestructible ground block. Note: 'C' (CoinBlock) and '#' (BrickBlock) are NOT
    // registered here anymore — those symbols spawn as entities and render themselves.
    // The world theme picks which atlas rectangle stands in for the ground (all are
    // 16x16 crops of existing tiles): the classic brown tile, a teal underwater tile,
    // or a gray castle tile.
    switch (worldType) {
        case model::WorldType::Underwater:
            registerTile('G', tilesetPath, 21 * 16, 13 * 16, 16, 16);
            break;
        case model::WorldType::Castle:
            registerTile('G', tilesetPath, 19 * 16, 14 * 16, 16, 16);
            break;
        case model::WorldType::Overworld:
        default:
            registerTile('G', MarioAssetPath, 0, 16, 16, 16);
            break;
    }
    // Background scenery: passable (see TileMap::isSolidTile), color-keyed off the
    // tileset's blue backdrop.
    registerTile(model::TileMap::CloudSymbol, MarioAssetPath, 344, 632, 3 * 16, 2 * 16, SceneryBackdrop);
    registerTile(model::TileMap::SmallTreeSymbol, MarioAssetPath, 264, 656, 16, 2 * 16, SceneryBackdrop);
    // The goal castle is painted into the padded completion zone by the controller
    // from its 21-tile sheet. The symbols (see TileMap::CastleSymbols) were chosen to
    // not clash with the map's own symbols, and are mapped row-major over the castle's
    // 5x5 silhouette: the upper two rows are the 3-wide tower (atlas x = 40..72), the
    // lower three rows are the 5-wide base (atlas x = 24..88). The centre-bottom pair
    // is the entrance. The two unpainted corner cells of the tower rows stay air.
    {
        static constexpr int SheetX[model::TileMap::CastleTiles] =
            {40, 56, 72, 40, 56, 72,
             24, 40, 56, 72, 88,
             24, 40, 56, 72, 88,
             24, 40, 56, 72, 88};
        static constexpr int SheetY[model::TileMap::CastleTiles] =
            {696, 696, 696, 712, 712, 712,
             728, 728, 728, 728, 728,
             744, 744, 744, 744, 744,
             760, 760, 760, 760, 760};
        for (std::size_t i = 0; i < model::TileMap::CastleTiles; ++i) {
            registerTile(model::TileMap::CastleSymbols[i], MarioAssetPath,
                         SheetX[i], SheetY[i], 16, 16);
        }
    }
}

void TileMapRenderer::loadTileset(const std::string& tilesetPath) {
    if (tilesets.count(tilesetPath) > 0) {
        return;
    }

    SpritePainter& tileset = tilesets[tilesetPath];
    if (!tileset.load(tilesetPath)) {
        tilesets.erase(tilesetPath);
        throw std::runtime_error("Could not load tileset: " + tilesetPath);
    }
}

SpritePainter& TileMapRenderer::tilesetFor(const std::string& tilesetPath) {
    const auto found = tilesets.find(tilesetPath);
    if (found == tilesets.end()) {
        throw std::runtime_error("Tileset not loaded: " + tilesetPath);
    }
    return found->second;
}

void TileMapRenderer::registerTile(char symbol, const std::string& tilesetPath, unsigned int x, unsigned int y,
                                    unsigned int width, unsigned int height,
                                    std::optional<sf::Color> transparentColor) {
    SpritePainter& tileset = tilesetFor(tilesetPath);

    const sf::IntRect area(
        { static_cast<int>(x), static_cast<int>(y) },
        { static_cast<int>(width), static_cast<int>(height) }
    );

    if (transparentColor.has_value()) {
        tileset.applyColorKey(area, *transparentColor);
    }

    tileRects[symbol] = TileEntry{ &tileset, area };
}

void TileMapRenderer::render(sf::RenderTarget& window, const model::TileMap& map) const {
    for (std::size_t row = 0; row < map.getRows(); ++row) {
        for (std::size_t column = 0; column < map.getColumns(); ++column) {
            const char symbol = map.getTile(row, column);
            if (symbol == '.') {
                continue;
            }

            const auto found = tileRects.find(symbol);
            if (found == tileRects.end()) {
                continue;
            }
            const TileEntry& entry = found->second;

            // A tile larger than one cell is anchored at its top-left cell and extends
            // right/down from there; the map file leaves the cells it covers empty.
            entry.tileset->drawCell(window, entry.rect, {
                static_cast<float>(column * model::TileMap::TileWidth),
                static_cast<float>((map.getRows() - row - 1) * model::TileMap::TileHeight)
            });
        }
    }
}

}