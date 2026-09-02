#include "View/Level/LevelGoalRenderer.h"

#include "Model/Level/LevelGoal.h"

#include <SFML/Graphics/RenderTarget.hpp>

namespace view {

namespace {
// Atlas coordinates in blocks.png (16x16 source tiles) — the same tiles the retired
// FlagPoleRenderer used: a teal pipe-ish tile for the pole, a gold tile for ball + pennant.
constexpr int PoleTileX = 21 * 16;  // 336
constexpr int PoleTileY = 13 * 16;  // 208
constexpr int GoldTileX = 14 * 16;  // 224
constexpr int GoldTileY = 10 * 16;  // 160

// Visual size of the pole, independent of the entity's (taller, catch-anything) trigger
// box — see the header comment.
constexpr float PoleWidth = 4.0f;
constexpr float PoleHeight = 48.0f;
}

LevelGoalRenderer::LevelGoalRenderer()
    : painter("assets/blocks.png") {
}

void LevelGoalRenderer::renderTyped(sf::RenderTarget& window,
                                    const model::LevelGoal& goal,
                                    const RenderContext& /* ctx */) const {
    if (!painter.isLoaded()) {
        return;
    }

    // The pole stands on the ground the marked cell sits on — the BOTTOM of the entity's
    // box — and rises PoleHeight from there, regardless of how tall the trigger itself is.
    const float groundY = goal.getPosition().y + goal.getSize().y;
    const float poleX = goal.getPosition().x + (goal.getSize().x - PoleWidth) / 2.0f;
    const float poleTopY = groundY - PoleHeight;

    painter.draw(window, {{PoleTileX, PoleTileY}, {16, 16}}, {poleX, poleTopY},
                 {PoleWidth / 16.0f, PoleHeight / 16.0f});

    // Gold ball on top (centred over the pole).
    painter.drawCell(window, {{GoldTileX, GoldTileY}, {16, 16}}, {poleX - 4.0f, poleTopY - 32.0f});

    // Pennant at rest, a third of the way down the pole — the old flagpole's idle position
    // before a slide cinematic ever moved it.
    painter.drawCell(window, {{GoldTileX, GoldTileY}, {16, 16}},
                     {poleX + 4.0f, poleTopY + PoleHeight * 0.4f});
}

}
