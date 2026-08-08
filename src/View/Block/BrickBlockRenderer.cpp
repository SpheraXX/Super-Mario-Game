#include "View/Block/BrickBlockRenderer.h"

#include "View/Base/EntityRenderUtils.h"
#include "Model/Block/BrickBlock.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <cmath>

namespace view {

namespace {
// Atlas coordinates of the brick tile in blocks.png (16x16 source tiles).
constexpr int BrickAtlasCol = 17;
constexpr int BrickAtlasRow = 7;
}

BrickBlockRenderer::BrickBlockRenderer()
    : textureLoaded(texture.loadFromFile("assets/blocks.png")) {
    texture.setSmooth(false);
}

void BrickBlockRenderer::renderTyped(sf::RenderTarget& window,
                                     const model::BrickBlock& brickBlock,
                                     const RenderContext& /* ctx */) const {
    if (!textureLoaded) return;

    sf::Sprite sprite(texture);
    sprite.setTextureRect({{BrickAtlasCol * 16, BrickAtlasRow * 16}, {16, 16}});
    sprite.setScale({SpriteScaleX, SpriteScaleY});
    sprite.setOrigin({0.0f, 0.0f});
    sprite.setPosition({std::round(brickBlock.getPosition().x),
                        std::round(brickBlock.getPosition().y - brickBlock.getBounceOffsetY())});
    window.draw(sprite);
}

}
