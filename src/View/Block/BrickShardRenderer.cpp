#include "View/Block/BrickShardRenderer.h"

#include "View/Base/RenderContext.h"
#include "View/Block/BlockAtlas.h"
#include "Model/Block/BrickShard.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <cmath>

namespace view {

BrickShardRenderer::BrickShardRenderer()
    : textureLoaded(texture.loadFromFile("assets/super_mario_asset.png")) {
    texture.setSmooth(false);
}

void BrickShardRenderer::renderTyped(sf::RenderTarget& window,
                                     const model::BrickShard& shard,
                                     const RenderContext& ctx) const {
    if (!textureLoaded) return;

    const sf::Vector2i brickOrigin = atlas::brickOrigin(ctx.worldType);
    const int quadrant = shard.getQuadrant();
    // Quadrants read left-to-right, top-to-bottom over the brick's 16x16 art: 0/1 are the
    // top half, 2/3 the bottom half; even indices are the left half, odd the right half.
    const sf::Vector2i offset{(quadrant % 2) * 8, (quadrant / 2) * 8};

    sf::Sprite sprite(texture);
    sprite.setTextureRect({brickOrigin + offset, {8, 8}});
    sprite.setPosition({std::round(shard.getPosition().x), std::round(shard.getPosition().y)});
    window.draw(sprite);
}

}
