#include "View/Block/BrickBlockRenderer.h"

#include "View/Base/EntityRenderUtils.h"
#include "View/Base/RenderContext.h"
#include "Model/Block/BrickBlock.h"
#include "Model/World/WorldType.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <cmath>

namespace view {

namespace {
// The brick lives in the same shared sheet as the terrain tiles, and in the same per-world
// 2x2 arrangement (see TileMapRenderer::atlasFor) — the brick IS each quadrant's origin.
// Kept in step with that table: if one moves, so must the other.
sf::Vector2i brickOriginFor(model::WorldType worldType) {
    switch (worldType) {
        case model::WorldType::Underground: return {147, 16};
        case model::WorldType::Underwater:  return {147, 100};
        case model::WorldType::Castle:      return {0, 100};
        case model::WorldType::Overworld:
        default:                            return {0, 16};
    }
}
}

BrickBlockRenderer::BrickBlockRenderer()
    : textureLoaded(texture.loadFromFile("assets/super_mario_asset.png")) {
    texture.setSmooth(false);
}

void BrickBlockRenderer::renderTyped(sf::RenderTarget& window,
                                     const model::BrickBlock& brickBlock,
                                     const RenderContext& ctx) const {
    if (!textureLoaded) return;

    // The brick rect is solid artwork edge to edge, so unlike the scenery it needs no
    // colour key: there is no backdrop inside it to mask out.
    const sf::Vector2i origin = brickOriginFor(ctx.worldType);

    sf::Sprite sprite(texture);
    sprite.setTextureRect({origin, {16, 16}});
    sprite.setScale({SpriteScaleX, SpriteScaleY});
    sprite.setOrigin({0.0f, 0.0f});
    sprite.setPosition({std::round(brickBlock.getPosition().x),
                        std::round(brickBlock.getPosition().y - brickBlock.getBounceOffsetY())});
    window.draw(sprite);
}

}
