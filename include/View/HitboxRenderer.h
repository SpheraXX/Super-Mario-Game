#ifndef VIEW_HITBOXRENDERER_H
#define VIEW_HITBOXRENDERER_H

#include <SFML/Graphics/RenderTarget.hpp>

namespace model {
class Entity;
class TileMap;
}

namespace view {

// Debug overlay: outlines collision bounds so they can be compared against the art they
// belong to. Drawn in world space, after the entities themselves.
class HitboxRenderer {
public:
    // Entity hitboxes (red).
    void render(sf::RenderTarget& target, const model::Entity& entity) const;

    // Solid map tiles (yellow). Tiles carry no Hitbox object — CollisionManager resolves
    // them straight against the grid — so their bounds are reconstructed from the cell.
    void renderTiles(sf::RenderTarget& target, const model::TileMap& map) const;
};

}

#endif
