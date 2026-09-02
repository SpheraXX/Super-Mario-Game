#include "View/Level/FlagPoleRenderer.h"

#include "Model/Level/FlagPole.h"

#include <SFML/Graphics/RenderTarget.hpp>

namespace view {

namespace {
// Atlas coordinates in blocks.png (16x16 source tiles).
// Teal pipe-ish tile -> the pole; gold tile -> ball + pennant.
constexpr int PoleTileX = 21 * 16;  // 336
constexpr int PoleTileY = 13 * 16;  // 208
constexpr int GoldTileX = 14 * 16;  // 224
constexpr int GoldTileY = 10 * 16;  // 160
}

FlagPoleRenderer::FlagPoleRenderer()
    : painter("assets/blocks.png") {
}

void FlagPoleRenderer::renderTyped(sf::RenderTarget& window,
                                   const model::FlagPole& pole,
                                   const RenderContext& /* ctx */) const {
    if (!painter.isLoaded()) {
        return;
    }

    const sf::Vector2f pos{pole.getPosition().x, pole.getPosition().y};
    const sf::Vector2f size{pole.getSize().x, pole.getSize().y};

    // The pole: the source tile is stretched onto the entity's thin, tall box.
    painter.draw(window, {{PoleTileX, PoleTileY}, {16, 16}}, pos,
                 {size.x / 16.0f, size.y / 16.0f});

    // Gold ball on top (centred over the 8px-wide pole).
    painter.drawCell(window, {{GoldTileX, GoldTileY}, {16, 16}},
                     {pos.x - 4.0f, pos.y - 32.0f});

    // Pennant waving on the pole. During the level-clear cinematic the pennant slides
    // down the pole with the player (slideProgress 0..1) — from a third of the way up at
    // rest to just above the ground when the clear play ends.
    const float pennantOffset = size.y * (0.4f + 0.55f * pole.getSlideProgress());
    painter.drawCell(window, {{GoldTileX, GoldTileY}, {16, 16}},
                     {pos.x + 4.0f, pos.y + pennantOffset});
}

}