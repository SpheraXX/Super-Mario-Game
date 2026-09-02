#ifndef VIEW_BLOCK_BRICKSHARDRENDERER_H
#define VIEW_BLOCK_BRICKSHARDRENDERER_H

#include "View/Base/EntityRenderer.h"

#include <SFML/Graphics/Texture.hpp>

namespace model {
class BrickShard;
}

namespace view {

// Draws a BrickShard as the matching 8x8 quadrant of the brick it broke off of, themed by
// the world exactly like BrickBlockRenderer (see view::atlas::brickOrigin) — the shard's
// artwork is never anything but a crop of the very brick it came from.
class BrickShardRenderer : public TypedEntityRenderer<model::BrickShard> {
public:
    BrickShardRenderer();

protected:
    void renderTyped(sf::RenderTarget& window, const model::BrickShard& shard,
                     const RenderContext& ctx) const override;

private:
    sf::Texture texture;
    bool textureLoaded;
};

}

#endif
