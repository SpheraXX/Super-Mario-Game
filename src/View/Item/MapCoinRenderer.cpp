#include "View/Item/MapCoinRenderer.h"

#include "Model/Item/MapCoin.h"
#include "View/Item/ItemAtlas.h"

#include <SFML/Graphics/RenderTarget.hpp>

#include <iterator>

namespace view {

namespace {
// Time each pose is shown. Slower than the fireball's tumble: the three coin frames share
// one silhouette and differ only in palette, so a fast cycle strobes rather than animates.
constexpr float FrameDuration = 0.15f;
}

MapCoinRenderer::MapCoinRenderer()
    // The coin lives on the main Mario sheet, which has no alpha channel and keys its flat
    // backdrop out at load — same as the block-flourish coin.
    : SpriteEntityRenderer(atlas::MarioAssetSheet, atlas::MarioAssetColorKey,
                           /*sourceFacesRight=*/true) {
}

void MapCoinRenderer::renderTyped(sf::RenderTarget& window, const model::MapCoin& coin,
                                  const RenderContext& /* ctx */) const {
    const std::size_t index =
        static_cast<std::size_t>(coin.getAnimationClock() / FrameDuration)
        % std::size(atlas::CoinSpin);
    drawCharacterFrame(window, coin, atlas::CoinSpin[index]);
}

}
