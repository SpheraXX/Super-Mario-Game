#include "View/TileMapRenderer.h"

#include <cmath>
#include <stdexcept>

namespace view {

TileMapRenderer::TileMapRenderer(const std::string& tilesetPath) {
    if (!tilesetTexture.loadFromFile(tilesetPath)) {
        throw std::runtime_error("Could not load tileset: " + tilesetPath);
    }
    tilesetTexture.setSmooth(false);

    registerTile('#', 17, 7);
    registerTile('C', 5, 7);
}

void TileMapRenderer::registerTile(char symbol, unsigned int atlasColumn, unsigned int atlasRow) {
    tileRects[symbol] = sf::IntRect(
        {
            static_cast<int>(atlasColumn * SourceTileSize),
            static_cast<int>(atlasRow * SourceTileSize)
        },
        {
            static_cast<int>(SourceTileSize),
            static_cast<int>(SourceTileSize)
        }
    );
}

void TileMapRenderer::render(sf::RenderWindow& window, const model::TileMap& map) const {
    sf::Sprite tileSprite(tilesetTexture);
    tileSprite.setScale({
        static_cast<float>(model::TileMap::TileWidth / SourceTileSize),
        static_cast<float>(model::TileMap::TileHeight / SourceTileSize)
    });

    for (std::size_t row = 0; row < map.getRows(); ++row) {
        for (std::size_t column = 0; column < map.getColumns(); ++column) {
            const char symbol = map.getTile(row, column);
            if (symbol == '.') {
                continue;
            }

            const auto tileRect = tileRects.find(symbol);
            if (tileRect == tileRects.end()) {
                continue;
            }

            tileSprite.setTextureRect(tileRect->second);
            tileSprite.setPosition({
                static_cast<float>(column * model::TileMap::TileWidth),
                static_cast<float>((map.getRows() - row - 1) * model::TileMap::TileHeight)
            });
            window.draw(tileSprite);
        }
    }
}

}
