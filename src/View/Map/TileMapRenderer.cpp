#include "View/Map/TileMapRenderer.h"

#include "Model/Map/TileMap.h"

#include <SFML/Graphics/Sprite.hpp>

#include <stdexcept>
#include <utility>

namespace view {

namespace {
const std::string MarioAssetPath = "assets/super_mario_asset.png";

// The backdrop every tile on this sheet is cut out of...
const sf::Color SceneryBackdrop(148, 148, 255);
// ...except the kelp, which sits on its own blue patch. This colour must be keyed ONLY on
// the kelp's rect: the underwater hill's BODY is painted the very same blue, so keying it
// across the sheet (or across the hill) would erase the hill entirely.
const sf::Color KelpBackdrop(66, 66, 255);

constexpr int Cell = 16;   // artwork size of one tile
constexpr int Pitch = 17;  // 16px of art + a 1px gutter, the sheet's grid step

// Offsets within a landscape's BLOCK set, relative to its brick.
constexpr int SolidBlockDX = 2 * Pitch;  // the plain block sits two steps right of the brick
constexpr int StairDY = Pitch;           // the stair block sits one step below it

// Rows within a landscape's SCENERY set, relative to the bush row.
constexpr int HillTopDY = Pitch;
constexpr int HillBottomDY = 2 * Pitch;
constexpr int KelpDX = 3 * Pitch;  // kelp follows the bush's three cells on the same row
}

// Verified against the artwork: the four scenery quadrants have pixel-identical
// silhouettes and differ only in palette, so one set of offsets serves every landscape.
TileMapRenderer::WorldAtlas TileMapRenderer::atlasFor(model::WorldType worldType) {
    switch (worldType) {
        case model::WorldType::Underground: return {147, 16, 164, 213};
        case model::WorldType::Underwater:  return {147, 100, 164, 297};
        case model::WorldType::Castle:      return {0, 100, 0, 297};
        case model::WorldType::Overworld:
        default:                            return {0, 16, 0, 213};
    }
}

TileMapRenderer::TileMapRenderer(const std::string& tilesetPath, model::WorldType worldType) {
    loadTileset(tilesetPath);
    loadTileset(MarioAssetPath);

    const WorldAtlas atlas = atlasFor(worldType);
    const auto blockRect = [&atlas](int dx, int dy) {
        return sf::IntRect({atlas.blockX + dx, atlas.blockY + dy}, {Cell, Cell});
    };
    const auto sceneryRect = [&atlas](int dx, int dy) {
        return sf::IntRect({atlas.sceneryX + dx, atlas.sceneryY + dy}, {Cell, Cell});
    };

    // Terrain. 'C' (CoinBlock) and '#' (BrickBlock) are NOT registered here — those spawn
    // as entities and render themselves. These block rects are solid artwork edge to edge,
    // with no backdrop pixels inside them, so unlike the scenery they need no colour key.
    registerTile('G', MarioAssetPath, blockRect(SolidBlockDX, 0));
    registerTile(model::TileMap::StairSymbol, MarioAssetPath, blockRect(0, StairDY));

    // A bush is three cells wide and centred on the cell the author marked, so the marker
    // reads as "the bush is here" rather than "the bush starts here".
    registerComposite(model::TileMap::BushSymbol, MarioAssetPath,
                      {{sceneryRect(0, 0), -1, 0},
                       {sceneryRect(Pitch, 0), 0, 0},
                       {sceneryRect(2 * Pitch, 0), 1, 0}},
                      SceneryBackdrop, true);

    // A hill is a 5-3-1 pyramid. Its marker is the middle of the BOTTOM row, so the author
    // places it on the ground row and the hill grows upward from there. The middle row
    // deliberately re-uses the bottom row's 1st, 3rd and 5th tiles — the sheet has no
    // separate artwork for it.
    {
        std::vector<TilePart> hill;
        for (int i = 0; i < 5; ++i) {
            hill.push_back({sceneryRect(i * Pitch, HillBottomDY), i - 2, 0});
        }
        hill.push_back({sceneryRect(0 * Pitch, HillBottomDY), -1, 1});
        hill.push_back({sceneryRect(2 * Pitch, HillBottomDY), 0, 1});
        hill.push_back({sceneryRect(4 * Pitch, HillBottomDY), 1, 1});
        hill.push_back({sceneryRect(2 * Pitch, HillTopDY), 0, 2});
        registerComposite(model::TileMap::HillSymbol, MarioAssetPath, std::move(hill),
                          SceneryBackdrop, true);
    }

    // Kelp is underwater-only, and is an ordinary one-cell tile: a tall strand is simply
    // several kelp cells stacked in the map file, which keeps the height in the author's
    // hands and out of the renderer's.
    if (worldType == model::WorldType::Underwater) {
        registerTile(model::TileMap::KelpSymbol, MarioAssetPath, sceneryRect(KelpDX, 0),
                     KelpBackdrop, true);
    }

    // Cloud and tree are shared across every landscape: the sheet has no per-world variants
    // of them, they are crops out of a scene rather than members of the 2x2 tile grid. Both
    // are anchored at their top-left cell and extend right/down, unlike the bush and hill.
    registerTile(model::TileMap::CloudSymbol, MarioAssetPath,
                 sf::IntRect({344, 632}, {3 * Cell, 2 * Cell}), SceneryBackdrop, true);
    registerTile(model::TileMap::SmallTreeSymbol, MarioAssetPath,
                 sf::IntRect({264, 656}, {Cell, 2 * Cell}), SceneryBackdrop, true);

    // Pipes are built from one-cell tiles rather than a single oversized sprite. Collision
    // is decided per cell (TileMap::isSolidTile reads one char), so a pipe drawn as one wide
    // sprite would leave its right-hand column passable and everything would walk into it.
    //
    // Standing pipe, two cells wide and as tall as the author writes:
    //     PQ      <- mouth
    //     pq      <- shaft, repeated
    registerTile('P', MarioAssetPath, sf::IntRect({119, 196}, {Cell, Cell}), SceneryBackdrop);
    registerTile('Q', MarioAssetPath, sf::IntRect({136, 196}, {Cell, Cell}), SceneryBackdrop);
    registerTile('p', MarioAssetPath, sf::IntRect({119, 213}, {Cell, Cell}), SceneryBackdrop);
    registerTile('q', MarioAssetPath, sf::IntRect({136, 213}, {Cell, Cell}), SceneryBackdrop);

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
                         sf::IntRect({SheetX[i], SheetY[i]}, {Cell, Cell}));
        }
    }

    // Every colour key above only edited the CPU-side image; push the result to the GPU
    // once here rather than re-uploading the whole sheet after each keyed rect.
    for (auto& entry : tilesets) {
        entry.second.commitColorKeys();
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

void TileMapRenderer::registerTile(char symbol, const std::string& tilesetPath,
                                   const sf::IntRect& rect,
                                   std::optional<sf::Color> transparentColor,
                                   bool behindTerrain) {
    registerComposite(symbol, tilesetPath, {{rect, 0, 0}}, transparentColor, behindTerrain);
}

void TileMapRenderer::registerComposite(char symbol, const std::string& tilesetPath,
                                        std::vector<TilePart> parts,
                                        std::optional<sf::Color> transparentColor,
                                        bool behindTerrain) {
    SpritePainter& tileset = tilesetFor(tilesetPath);

    if (transparentColor.has_value()) {
        for (const TilePart& part : parts) {
            tileset.applyColorKey(part.rect, *transparentColor);
        }
    }

    tileRects[symbol] = TileEntry{&tileset, std::move(parts), behindTerrain};
}

void TileMapRenderer::render(sf::RenderTarget& window, const model::TileMap& map) const {
    const int rows = static_cast<int>(map.getRows());

    // Two passes: scenery first, then terrain. Bushes and hills are painted around their
    // marker rather than inside it, so drawing everything in one grid order would let a
    // hill overwrite ground tiles that are supposed to be in front of it.
    for (int pass = 0; pass < 2; ++pass) {
        const bool drawingScenery = (pass == 0);

        for (int row = 0; row < rows; ++row) {
            for (int column = 0; column < static_cast<int>(map.getColumns()); ++column) {
                const char symbol = map.getTile(static_cast<std::size_t>(row),
                                                static_cast<std::size_t>(column));
                if (symbol == '.') {
                    continue;
                }

                const auto found = tileRects.find(symbol);
                if (found == tileRects.end()) {
                    continue;
                }
                const TileEntry& entry = found->second;
                if (entry.behindTerrain != drawingScenery) {
                    continue;
                }

                for (const TilePart& part : entry.parts) {
                    // A part sits `cellRight` cells to the right and `cellUp` cells above
                    // the marker. Row 0 is the bottom row, so moving up the map means
                    // moving down the screen coordinate.
                    entry.tileset->drawCell(window, part.rect, {
                        static_cast<float>((column + part.cellRight)
                                           * static_cast<int>(model::TileMap::TileWidth)),
                        static_cast<float>((rows - row - 1 - part.cellUp)
                                           * static_cast<int>(model::TileMap::TileHeight))
                    });
                }
            }
        }
    }
}

}
