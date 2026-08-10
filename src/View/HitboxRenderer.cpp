#include "View/HitboxRenderer.h"

#include "Model/Entity.h"
#include "Model/Map/TileMap.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

namespace view {

namespace {
// Negative thickness draws the border inside the shape, so an outline marks the exact
// collision bounds instead of extending a pixel past them.
constexpr float OutlineThickness = -1.0f;

sf::RectangleShape makeOutline(sf::Vector2f position, sf::Vector2f size, sf::Color color) {
    sf::RectangleShape outline(size);
    outline.setPosition(position);
    outline.setFillColor(sf::Color::Transparent);
    outline.setOutlineColor(color);
    outline.setOutlineThickness(OutlineThickness);
    return outline;
}
}

void HitboxRenderer::render(sf::RenderTarget& target, const model::Entity& entity) const {
    const model::Vector2 position = entity.getPosition();
    const model::Hitbox& hitbox = entity.hitbox;

    target.draw(makeOutline(
        {position.x + hitbox.offset.x, position.y + hitbox.offset.y},
        {hitbox.width, hitbox.height},
        sf::Color::Red));
}

void HitboxRenderer::renderTiles(sf::RenderTarget& target, const model::TileMap& map) const {
    const float tileWidth = static_cast<float>(model::TileMap::TileWidth);
    const float tileHeight = static_cast<float>(model::TileMap::TileHeight);

    for (std::size_t row = 0; row < map.getRows(); ++row) {
        for (std::size_t column = 0; column < map.getColumns(); ++column) {
            if (!model::TileMap::isSolidTile(map.getTile(row, column))) {
                continue;
            }

            // Same row-to-world flip the tile renderer uses: row 0 is the bottom of the map.
            target.draw(makeOutline(
                {column * tileWidth, (map.getRows() - row - 1) * tileHeight},
                {tileWidth, tileHeight},
                sf::Color::Yellow));
        }
    }
}

}
