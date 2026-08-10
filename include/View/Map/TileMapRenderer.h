#ifndef VIEW_TILEMAPRENDERER_H
#define VIEW_TILEMAPRENDERER_H

#include "Model/Map/TileMap.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <optional>
#include <string>
#include <unordered_map>

namespace view {

class TileMapRenderer {
public:
    explicit TileMapRenderer(const std::string& tilesetPath);

    // Loads an additional tileset image so tiles can be registered against it.
    // Loading the same path twice is a no-op.
    void loadTileset(const std::string& tilesetPath);

    // x, y, width, height are pixel coordinates/size within the named tileset image.
    // transparentColor, if given, is masked to transparent within just this tile's
    // region (not the whole tileset), so different tiles can key different background colors.
    void registerTile(char symbol, const std::string& tilesetPath, unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height,
                       std::optional<sf::Color> transparentColor = std::nullopt);
    void render(sf::RenderTarget& window, const model::TileMap& map) const;

private:
    // Base atlas unit used only to derive the render scale factor (TileWidth/SourceTileSize):
    // a tile registered with width/height that are multiples of this ends up covering that
    // many map cells on screen.
    static constexpr unsigned int SourceTileSize = 16;

    // The sheet stores its backdrop in several near-identical shades (e.g. 146,146,251 and
    // 148,148,255), so the color key matches within a tolerance instead of exactly —
    // otherwise stray backdrop pixels survive as a visible fringe. Safe because the closest
    // actual artwork color sits far outside this radius.
    static constexpr int ColorKeyTolerance = 16;

    struct Tileset {
        sf::Image image;
        sf::Texture texture;
    };

    struct TileEntry {
        // Points into `tilesets`; unordered_map keeps element addresses stable across inserts.
        const Tileset* tileset = nullptr;
        sf::IntRect rect;
    };

    Tileset& tilesetFor(const std::string& tilesetPath);
    static void applyColorKey(Tileset& tileset, const sf::IntRect& area, sf::Color transparentColor);
    static void refreshTexture(Tileset& tileset);

    std::unordered_map<std::string, Tileset> tilesets;
    std::unordered_map<char, TileEntry> tileRects;
};

}

#endif
