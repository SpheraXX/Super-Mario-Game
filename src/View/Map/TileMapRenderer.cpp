#include "View/Map/TileMapRenderer.h"

#include "Model/Map/TileMap.h"
#include "View/Block/BlockAtlas.h"

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
// The block origin is the shared atlas::brickOrigin (the brick IS each quadrant's origin;
// see View/Block/BlockAtlas.h) so it can never drift out of step with BrickBlockRenderer.
TileMapRenderer::WorldAtlas TileMapRenderer::atlasFor(model::WorldType worldType) {
    const sf::Vector2i brick = atlas::brickOrigin(worldType);
    switch (worldType) {
        case model::WorldType::Underground: return {brick.x, brick.y, 164, 213};
        case model::WorldType::Underwater:  return {brick.x, brick.y, 164, 297};
        case model::WorldType::Castle:      return {brick.x, brick.y, 0, 297};
        case model::WorldType::Overworld:
        default:                            return {brick.x, brick.y, 0, 213};
    }
}

TileMapRenderer::TileMapRenderer(const std::string& tilesetPath, model::WorldType worldType)
    : worldType(worldType) {
    loadTileset(tilesetPath);
    loadTileset(MarioAssetPath);

    const WorldAtlas atlas = atlasFor(worldType);
    const auto sceneryRect = [&atlas](int dx, int dy) {
        return sf::IntRect({atlas.sceneryX + dx, atlas.sceneryY + dy}, {Cell, Cell});
    };

    // Terrain. 'C' (CoinBlock) and '#' (BrickBlock) are NOT registered here — those spawn
    // as entities and render themselves. These block rects are solid artwork edge to edge,
    // with no backdrop pixels inside them, so unlike the scenery they need no colour key.
    //
    // groundOrigin points to the beveled-stone tile; brickOrigin points to the brick-mortar tile.
    // Previously the two were swapped (the atlas block origin was set from brickOrigin and then
    // SolidBlockDX was applied to reach what is actually the BRICK column).  Now the ground tile
    // is addressed directly via atlas::groundOrigin so the correct artwork is shown.
    const sf::Vector2i groundPx = atlas::groundOrigin(worldType);
    registerTile('G', MarioAssetPath,
                 sf::IntRect({groundPx.x,           groundPx.y},          {Cell, Cell}));
    registerTile(model::TileMap::StairSymbol, MarioAssetPath,
                 sf::IntRect({groundPx.x,           groundPx.y + StairDY},{Cell, Cell}));

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

    // Horizontal pipe lower body (see TileMap::HorizontalPipeSymbol): unlike the standing
    // pipe above, this is a single WYSIWYG 4x2 blit anchored at its top-left cell, not a
    // per-cell tiling -- the art is a one-off end shape, not a repeatable column. Drawn in
    // the terrain pass (not behindTerrain), like the standing pipe: it is solid geometry,
    // not backdrop.
    registerTile(model::TileMap::HorizontalPipeSymbol, MarioAssetPath,
                 sf::IntRect({192, 656}, {4 * Cell, 2 * Cell}), SceneryBackdrop);

    // The castle bridge deck. Keyed on BLACK, not on SceneryBackdrop like its neighbours:
    // this cell is cut from a castle scene, whose sky is black, so black is what has to
    // drop out for the chain links to read as links rather than as a solid plank. Keying
    // is per-rect (see SpritePainter::applyColorKey), so this cannot disturb the tiles
    // around it that legitimately paint in black.
    registerTile(model::TileMap::ChainSymbol, MarioAssetPath,
                 sf::IntRect({548, 476}, {Cell, Cell}), sf::Color::Black);

    // The block a firebar turns on. Solid artwork edge to edge, like the other blocks, so
    // it needs no colour key. It is a tile rather than part of the firebar entity because
    // the block is what the player stands on and the bar is what burns him — two different
    // jobs that happen to share a cell.
    registerTile(model::TileMap::FirebarSymbol, MarioAssetPath,
                 sf::IntRect({580, 476}, {Cell, Cell}));

    // The goal castle: two multi-cell images, not a 21-symbol grid of one-cell tiles. The
    // artwork is laid out as exactly these two rects, and the castle is pure backdrop now
    // that LevelGoal ('E') ends the level, so there was nothing the per-cell version bought
    // that was worth most of the alphabet.
    //
    //   upper  tower, 3x2 cells, atlas (40,696)..(87,727)
    //   lower  base,  5x3 cells, atlas (24,728)..(103,775)
    //
    // Keyed on SceneryBackdrop and drawn behind terrain, like the cloud and the tree: the
    // tower rect spans the full 3 cells, so its two unpainted top corners are backdrop that
    // has to be punched out rather than blitted as a blue block.
    registerTile(model::TileMap::CastleUpperSymbol, MarioAssetPath,
                 sf::IntRect({40, 696}, {3 * Cell, 2 * Cell}), SceneryBackdrop, true);
    registerTile(model::TileMap::CastleLowerSymbol, MarioAssetPath,
                 sf::IntRect({24, 728}, {5 * Cell, 3 * Cell}), SceneryBackdrop, true);

    // Molten lava (see TileMap::LavaTopSymbol/LavaSymbol): the top cell is the wave-crest
    // surface, keyed against the same SceneryBackdrop as everything else on this sheet —
    // the solid red fill below the crest is real artwork, not backdrop, so it is left
    // alone. The body cell is plain, edge-to-edge solid colour and needs no keying at all.
    // Purely decorative terrain: never in isSolidTile, so it never blocks or supports
    // anything, exactly like Cloud/SmallTree above.
    registerTile(model::TileMap::LavaTopSymbol, MarioAssetPath,
                 sf::IntRect({616, 728}, {Cell, Cell}), SceneryBackdrop);
    registerTile(model::TileMap::LavaSymbol, MarioAssetPath,
                 sf::IntRect({616, 744}, {Cell, Cell}));

    // The underwater surface wave has no map symbol — see the pre-pass in render() — but
    // still needs its backdrop keyed once here, on the same SceneryBackdrop as the lava
    // crest (both tiles share it; the user-facing spec calls this out explicitly).
    if (worldType == model::WorldType::Underwater) {
        underwaterWaveTileset = &tilesetFor(MarioAssetPath);
        underwaterWaveRect = sf::IntRect({616, 688}, {Cell, Cell});
        tilesetFor(MarioAssetPath).applyColorKey(underwaterWaveRect, SceneryBackdrop);
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

    // The underwater surface wave (see TileMapRenderer's ctor): painted across every
    // column of the second-to-top row, but only where the author left that cell empty —
    // a map that reserves the row for its own ceiling terrain is left alone.
    if (underwaterWaveTileset && rows >= 2) {
        const std::size_t waveRow = map.getRows() - 2;
        for (std::size_t column = 0; column < map.getColumns(); ++column) {
            if (map.getTile(waveRow, column) != '.') {
                continue;
            }
            const model::Vector2 origin = model::TileMap::tileOrigin(waveRow, column);
            underwaterWaveTileset->drawCell(window, underwaterWaveRect, {origin.x, origin.y});
        }
    }

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
