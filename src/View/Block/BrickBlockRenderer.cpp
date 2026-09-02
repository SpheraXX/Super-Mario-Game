#include "View/Block/BrickBlockRenderer.h"
#include "View/AssetManager.h"

#include "View/Base/EntityRenderUtils.h"
#include "View/Base/RenderContext.h"
#include "View/Block/BlockAtlas.h"
#include "Model/Block/BrickBlock.h"
#include "Model/World/WorldType.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <cmath>

namespace view {

BrickBlockRenderer::BrickBlockRenderer()
    : texturePtr(&AssetManager::instance().getTexture("assets/super_mario_asset.png")) {
}

void BrickBlockRenderer::renderTyped(sf::RenderTarget& window,
                                     const model::BrickBlock& brickBlock,
                                     const RenderContext& ctx) const {
    if (!texturePtr) return;

    // The brick rect is solid artwork edge to edge, so unlike the scenery it needs no
    // colour key: there is no backdrop inside it to mask out.
    const sf::Vector2i origin = atlas::brickOrigin(ctx.worldType);

    sf::Sprite sprite(*texturePtr);
    sprite.setTextureRect({origin, {16, 16}});
    sprite.setScale({SpriteScaleX, SpriteScaleY});
    sprite.setOrigin({0.0f, 0.0f});
    sprite.setPosition({std::round(brickBlock.getPosition().x),
                        std::round(brickBlock.getPosition().y - brickBlock.getBounceOffsetY())});
    window.draw(sprite);
}

}
