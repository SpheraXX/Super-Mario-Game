#ifndef VIEW_TILEMAPRENDERER_H
#define VIEW_TILEMAPRENDERER_H

#include "Model/TileMap.h"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <string>
#include <unordered_map>

namespace view {

class TileMapRenderer {
public:
    explicit TileMapRenderer(const std::string& tilesetPath);

    void registerTile(char symbol, unsigned int atlasColumn, unsigned int atlasRow);
    void render(sf::RenderWindow& window, const model::TileMap& map) const;

private:
    static constexpr unsigned int SourceTileSize = 16;

    sf::Texture tilesetTexture;
    std::unordered_map<char, sf::IntRect> tileRects;
};

}

#endif
