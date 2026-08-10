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

namespace view {

class TileMapRenderer {
public:
    // `worldType` selects the world's graphics theme: the ground tile and any themed
    // tiles are registered from the matching tiles of the atlas (placeholder art).
    explicit TileMapRenderer(const std::string& tilesetPath,
                             model::WorldType worldType = model::WorldType::Overworld);

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
    // The registry references each tileset's SpritePainter by path; unordered_map keeps
    // element addresses stable across inserts, so these pointers stay valid.
    struct TileEntry {
        const SpritePainter* tileset = nullptr;
        sf::IntRect rect;
    };

    SpritePainter& tilesetFor(const std::string& tilesetPath);

    std::unordered_map<std::string, SpritePainter> tilesets;
    std::unordered_map<char, TileEntry> tileRects;
};

}

#endif