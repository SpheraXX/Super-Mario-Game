#ifndef VIEW_TILEMAPRENDERER_H
#define VIEW_TILEMAPRENDERER_H

#include "Model/Map/TileMap.h"
#include "Model/World/WorldType.h"
#include "View/Base/SpritePainter.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace view {

// Draws the tile grid for one area, themed by the landscape the map declared.
//
// Every landscape's tiles come out of the SAME shared sheet, which stores them as a 2x2
// arrangement of four identical layouts (overworld / underground on one row, castle /
// underwater on the next). That is why a world is described here by just two origins —
// where its block set starts and where its scenery set starts — instead of a coordinate
// per tile per world: the layout within a quadrant is the same everywhere, so the offsets
// are written once and the origin selects the palette.
class TileMapRenderer {
public:
    // `worldType` selects the landscape's graphics theme: ground, stair block and scenery
    // are all taken from that landscape's quadrant of the sheet.
    explicit TileMapRenderer(const std::string& tilesetPath,
                             model::WorldType worldType = model::WorldType::Overworld);

    void render(sf::RenderTarget& window, const model::TileMap& map) const;

private:
    // One piece of a tile's artwork, placed in CELLS relative to the marker cell the map
    // author wrote. `cellUp` is positive upward to match the map's row order (row 0 is the
    // bottom row). An ordinary tile is a single part at {0, 0}; a bush is three parts on
    // one row, and a hill is nine parts across three rows.
    struct TilePart {
        sf::IntRect rect;
        int cellRight = 0;
        int cellUp = 0;
    };

    // The registry references each tileset's SpritePainter by path; unordered_map keeps
    // element addresses stable across inserts, so these pointers stay valid.
    struct TileEntry {
        const SpritePainter* tileset = nullptr;
        std::vector<TilePart> parts;
        // Scenery is drawn in an earlier pass than terrain. Bushes and hills spill outside
        // their own cell, so without the split they could paint over ground or blocks that
        // ought to stand in front of them.
        bool behindTerrain = false;
    };

    // Where one landscape's artwork begins in the shared sheet. Within a quadrant:
    //   blocks   brick at (blockX, blockY), solid block +34x, stair +17y
    //   scenery  bush row at (sceneryX, sceneryY), hill top +17y, hill bottom +34y
    struct WorldAtlas {
        int blockX;
        int blockY;
        int sceneryX;
        int sceneryY;
    };
    static WorldAtlas atlasFor(model::WorldType worldType);

    void loadTileset(const std::string& tilesetPath);
    SpritePainter& tilesetFor(const std::string& tilesetPath);

    // Register a one-cell tile, optionally keying its backdrop to transparent.
    void registerTile(char symbol, const std::string& tilesetPath, const sf::IntRect& rect,
                      std::optional<sf::Color> transparentColor = std::nullopt,
                      bool behindTerrain = false);
    // Register a tile whose artwork covers several cells around its marker.
    void registerComposite(char symbol, const std::string& tilesetPath,
                           std::vector<TilePart> parts,
                           std::optional<sf::Color> transparentColor,
                           bool behindTerrain);

    std::unordered_map<std::string, SpritePainter> tilesets;
    std::unordered_map<char, TileEntry> tileRects;
};

}

#endif
