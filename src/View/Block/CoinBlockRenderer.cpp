#include "View/Block/CoinBlockRenderer.h"
#include "View/AssetManager.h"

#include "View/Base/EntityRenderUtils.h"
#include "View/Base/RenderContext.h"
#include "Model/Block/CoinBlock.h"
#include "Model/World/WorldType.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <cmath>

namespace view {

namespace {
// Atlas coordinates of the coin-block tiles in blocks.png (16x16 source tiles).
// Each theme owns its own row: the gold (overworld) row has the ? block at col 5 and
// the used block 4 tiles further right; the teal (underwater) row repeats the same
// layout but with the teal ? at col 5 and the solid teal used block at col 8.
constexpr int QuestionBlockAtlasCol = 5;
constexpr int UsedBlockAtlasColGold = 9;
constexpr int UsedBlockAtlasColTeal = 8;
constexpr int GoldAtlasRow = 7;
constexpr int TealAtlasRow = 8;
}

CoinBlockRenderer::CoinBlockRenderer()
    : texturePtr(&AssetManager::instance().getTexture("assets/blocks.png")) {
}

void CoinBlockRenderer::renderTyped(sf::RenderTarget& window,
                                    const model::CoinBlock& coinBlock,
                                    const RenderContext& ctx) const {
    if (!texturePtr) return;

    const bool underwater = (ctx.worldType == model::WorldType::Underwater);
    const int blockRow = underwater ? TealAtlasRow : GoldAtlasRow;
    // The ? block while it still holds a coin, the plain used block afterwards.
    const int usedBlockCol = underwater ? UsedBlockAtlasColTeal : UsedBlockAtlasColGold;
    const int atlasCol = coinBlock.isOpened() ? usedBlockCol : QuestionBlockAtlasCol;

    // Block tiles are 16x16 source pixels (one tile), unlike the 16x32 character frames.
    sf::Sprite sprite(*texturePtr);
    sprite.setTextureRect({{atlasCol * 16, blockRow * 16}, {16, 16}});
    sprite.setScale({SpriteScaleX, SpriteScaleY});
    sprite.setOrigin({0.0f, 0.0f});
    sprite.setPosition({std::round(coinBlock.getPosition().x),
                        std::round(coinBlock.getPosition().y - coinBlock.getBounceOffsetY())});
    window.draw(sprite);
}

}
