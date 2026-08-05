#include "View/Block/CoinBlockRenderer.h"

#include "View/Base/EntityRenderUtils.h"
#include "Model/Block/CoinBlock.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <cmath>

namespace view {

namespace {
// Atlas coordinates of the coin-block tiles in blocks.png (16x16 source tiles).
constexpr int QuestionBlockAtlasCol = 5;
constexpr int UsedBlockAtlasCol = 6;
constexpr int BlockAtlasRow = 7;
}

CoinBlockRenderer::CoinBlockRenderer() : textureLoaded(texture.loadFromFile("assets/blocks.png")) {
    texture.setSmooth(false);
}

void CoinBlockRenderer::renderTyped(sf::RenderTarget& window,
                                    const model::CoinBlock& coinBlock) const {
    if (!textureLoaded) return;

    // (5, 7) is the same coin-block tile the map renderer uses for 'C' tiles; once the
    // coin is collected the block swaps to the plain used block at (6, 7).
    const int atlasCol = coinBlock.hasCoin() ? QuestionBlockAtlasCol : UsedBlockAtlasCol;

    // Block tiles are 16x16 source pixels (one tile), unlike the 16x32 character frames.
    sf::Sprite sprite(texture);
    sprite.setTextureRect({{atlasCol * 16, BlockAtlasRow * 16}, {16, 16}});
    sprite.setScale({SpriteScaleX, SpriteScaleY});
    sprite.setOrigin({0.0f, 0.0f});
    sprite.setPosition({std::round(coinBlock.getPosition().x),
                        std::round(coinBlock.getPosition().y)});
    window.draw(sprite);
}

}
