#include "View/Block/CoinBlockRenderer.h"

#include "View/Base/EntityRenderUtils.h"
#include "View/Base/RenderContext.h"
#include "Model/Block/CoinBlock.h"
#include "Model/World/WorldType.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <cmath>
#include <cstdint>

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

// Coin sprite in super_mario_asset.png (16x16 source tile, gold circle with outline).
constexpr int CoinAtlasX = 96;
constexpr int CoinAtlasY = 532;
}

CoinBlockRenderer::CoinBlockRenderer()
    : textureLoaded(texture.loadFromFile("assets/blocks.png")),
      coinTextureLoaded(coinTexture.loadFromFile("assets/super_mario_asset.png")) {
    texture.setSmooth(false);
    coinTexture.setSmooth(false);
}

void CoinBlockRenderer::renderTyped(sf::RenderTarget& window,
                                    const model::CoinBlock& coinBlock,
                                    const RenderContext& ctx) const {
    if (!textureLoaded) return;

    const bool underwater = (ctx.worldType == model::WorldType::Underwater);
    const int blockRow = underwater ? TealAtlasRow : GoldAtlasRow;
    // The ? block while it still holds a coin, the plain used block afterwards.
    const int usedBlockCol = underwater ? UsedBlockAtlasColTeal : UsedBlockAtlasColGold;
    const int atlasCol = coinBlock.isOpened() ? usedBlockCol : QuestionBlockAtlasCol;

    // Block tiles are 16x16 source pixels (one tile), unlike the 16x32 character frames.
    sf::Sprite sprite(texture);
    sprite.setTextureRect({{atlasCol * 16, blockRow * 16}, {16, 16}});
    sprite.setScale({SpriteScaleX, SpriteScaleY});
    sprite.setOrigin({0.0f, 0.0f});
    sprite.setPosition({std::round(coinBlock.getPosition().x),
                        std::round(coinBlock.getPosition().y - coinBlock.getBounceOffsetY())});
    window.draw(sprite);

    // The collected coin rises ~1.5 tiles out of the block and fades away as the pop
    // animation finishes. The model owns the timer; the renderer only draws it.
    if (coinTextureLoaded && coinBlock.isCoinPopping()) {
        const float progress = coinBlock.getCoinPopProgress();
        const float rise = progress * 24.0f;  // world units (one tile = 16)
        sf::Sprite coin(coinTexture);
        coin.setTextureRect({{CoinAtlasX, CoinAtlasY}, {16, 16}});
        coin.setScale({SpriteScaleX, SpriteScaleY});
        coin.setOrigin({0.0f, 0.0f});
        coin.setPosition({std::round(coinBlock.getPosition().x),
                          std::round(coinBlock.getPosition().y - rise)});
        if (progress > 0.75f) {  // fade out over the last quarter
            const float fade = (1.0f - progress) / 0.25f;
            coin.setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(fade * 255.0f)));
        }
        window.draw(coin);
    }
}

}
