#include "View/Map/TileMapRenderer.h"

#include <cstdint>
#include <cstdlib>
#include <stdexcept>

namespace view {

namespace {
const std::string MarioAssetPath = "assets/super_mario_asset.png";
const sf::Color SceneryBackdrop(148, 148, 255);

// Pipe artwork is laid out on a 17px pitch: 16px of art with a 1px separator.
constexpr unsigned int PipeCell = 16;

bool nearlyEqual(sf::Color a, sf::Color b, int tolerance) {
    const auto channelDiff = [](std::uint8_t lhs, std::uint8_t rhs) {
        return std::abs(static_cast<int>(lhs) - static_cast<int>(rhs));
    };
    return channelDiff(a.r, b.r) <= tolerance
        && channelDiff(a.g, b.g) <= tolerance
        && channelDiff(a.b, b.b) <= tolerance;
}
}

TileMapRenderer::TileMapRenderer(const std::string& tilesetPath) {
    loadTileset(tilesetPath);
    loadTileset(MarioAssetPath);

    registerTile('#', tilesetPath, 17 * SourceTileSize, 7 * SourceTileSize, SourceTileSize, SourceTileSize);
    registerTile('C', tilesetPath, 5 * SourceTileSize, 7 * SourceTileSize, SourceTileSize, SourceTileSize);

    // Indestructible ground block.
    registerTile('G', MarioAssetPath, 0, 16, 16, 16);
    // Background scenery: passable (see TileMap::isSolidTile), color-keyed off the
    // tileset's blue backdrop.
    registerTile(model::TileMap::CloudSymbol, MarioAssetPath, 344, 632, 3 * 16, 2 * 16, SceneryBackdrop);
    registerTile(model::TileMap::SmallTreeSymbol, MarioAssetPath, 264, 656, 16, 2 * 16, SceneryBackdrop);

    // Pipes are built from one-cell tiles rather than a single oversized sprite. Collision is
    // decided per cell (TileMap::isSolidTile reads one char), so a pipe drawn as one wide
    // sprite would leave its right-hand column passable and Mario would walk into it.
    //
    // Standing pipe, two cells wide and as tall as the author writes:
    //     PQ      <- mouth
    //     pq      <- shaft, repeated
    registerTile('P', MarioAssetPath, 119, 196, PipeCell, PipeCell, SceneryBackdrop);
    registerTile('Q', MarioAssetPath, 136, 196, PipeCell, PipeCell, SceneryBackdrop);
    registerTile('p', MarioAssetPath, 119, 213, PipeCell, PipeCell, SceneryBackdrop);
    registerTile('q', MarioAssetPath, 136, 213, PipeCell, PipeCell, SceneryBackdrop);

    // Sideways pipe, three cells tall and three long, for the level-exit run. Its right-hand
    // end reuses the standing pipe's right column (Q on top, q below), which is how the two
    // join into the classic L shape.
    //     HIJQ   <- top
    //     KLMq   <- middle, repeat for a taller run
    //     hijq   <- bottom
    registerTile('H', MarioAssetPath, 85, 230, PipeCell, PipeCell, SceneryBackdrop);
    registerTile('I', MarioAssetPath, 102, 230, PipeCell, PipeCell, SceneryBackdrop);
    registerTile('J', MarioAssetPath, 119, 230, PipeCell, PipeCell, SceneryBackdrop);
    registerTile('h', MarioAssetPath, 85, 247, PipeCell, PipeCell, SceneryBackdrop);
    registerTile('i', MarioAssetPath, 102, 247, PipeCell, PipeCell, SceneryBackdrop);
    registerTile('j', MarioAssetPath, 119, 247, PipeCell, PipeCell, SceneryBackdrop);

    // Middle band. The sheet only draws this pipe two cells tall — the cells below the
    // bottom row are empty backdrop, and the next artwork down is the grey palette swap —
    // so there is no true middle row to point at. These alias the top row, which means a
    // three-tall pipe shows the top row's outline as a seam a third of the way down.
    // If a proper middle band exists elsewhere on the sheet, repoint these three and
    // nothing else has to change.
    registerTile('K', MarioAssetPath, 85, 230, PipeCell, PipeCell, SceneryBackdrop);
    registerTile('L', MarioAssetPath, 102, 230, PipeCell, PipeCell, SceneryBackdrop);
    registerTile('M', MarioAssetPath, 119, 230, PipeCell, PipeCell, SceneryBackdrop);
}

void TileMapRenderer::loadTileset(const std::string& tilesetPath) {
    if (tilesets.count(tilesetPath) > 0) {
        return;
    }

    Tileset& tileset = tilesets[tilesetPath];
    if (!tileset.image.loadFromFile(tilesetPath)) {
        tilesets.erase(tilesetPath);
        throw std::runtime_error("Could not load tileset: " + tilesetPath);
    }
    refreshTexture(tileset);
}

TileMapRenderer::Tileset& TileMapRenderer::tilesetFor(const std::string& tilesetPath) {
    const auto found = tilesets.find(tilesetPath);
    if (found == tilesets.end()) {
        throw std::runtime_error("Tileset not loaded: " + tilesetPath);
    }
    return found->second;
}

void TileMapRenderer::registerTile(char symbol, const std::string& tilesetPath, unsigned int x, unsigned int y,
                                    unsigned int width, unsigned int height,
                                    std::optional<sf::Color> transparentColor) {
    Tileset& tileset = tilesetFor(tilesetPath);

    const sf::IntRect area(
        { static_cast<int>(x), static_cast<int>(y) },
        { static_cast<int>(width), static_cast<int>(height) }
    );

    if (transparentColor.has_value()) {
        applyColorKey(tileset, area, *transparentColor);
        refreshTexture(tileset);
    }

    tileRects[symbol] = TileEntry{ &tileset, area };
}

void TileMapRenderer::applyColorKey(Tileset& tileset, const sf::IntRect& area, sf::Color transparentColor) {
    const unsigned int left = static_cast<unsigned int>(area.position.x);
    const unsigned int top = static_cast<unsigned int>(area.position.y);
    const unsigned int width = static_cast<unsigned int>(area.size.x);
    const unsigned int height = static_cast<unsigned int>(area.size.y);

    for (unsigned int py = top; py < top + height; ++py) {
        for (unsigned int px = left; px < left + width; ++px) {
            const sf::Vector2u pixel(px, py);
            if (nearlyEqual(tileset.image.getPixel(pixel), transparentColor, ColorKeyTolerance)) {
                tileset.image.setPixel(pixel, sf::Color::Transparent);
            }
        }
    }
}

void TileMapRenderer::refreshTexture(Tileset& tileset) {
    if (!tileset.texture.loadFromImage(tileset.image)) {
        throw std::runtime_error("Could not upload tileset texture");
    }
    tileset.texture.setSmooth(false);
}

void TileMapRenderer::render(sf::RenderTarget& window, const model::TileMap& map) const {
    const sf::Vector2f scale{
        static_cast<float>(model::TileMap::TileWidth) / static_cast<float>(SourceTileSize),
        static_cast<float>(model::TileMap::TileHeight) / static_cast<float>(SourceTileSize)
    };

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
            sf::Sprite tileSprite(entry.tileset->texture);
            tileSprite.setTextureRect(entry.rect);
            tileSprite.setScale(scale);
            tileSprite.setPosition({
                static_cast<float>(column * model::TileMap::TileWidth),
                static_cast<float>((map.getRows() - row - 1) * model::TileMap::TileHeight)
            });
            window.draw(tileSprite);
        }
    }
}

}
