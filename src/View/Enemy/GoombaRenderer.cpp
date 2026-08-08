#include "View/Enemy/GoombaRenderer.h"

#include "Model/Enemy/Goomba.h"

#include <SFML/Graphics/RenderTarget.hpp>

namespace view {

namespace {
// The enemy artwork sits in rows 4-19 of each 16x32 atlas cell, with empty padding below.
// The frame must bound the art tightly, otherwise that padding offsets the sprite from the
// hitbox (which is what made the enemies look misplaced).
constexpr int EnemyFrameTop = 4;
constexpr int EnemyFrameSize = 16;

// Placeholder atlas columns; update once the real enemies spritesheet is finalised.
constexpr int GoombaFrameCol = 0;
constexpr int GoombaSquishedFrameCol = 1;
}

GoombaRenderer::GoombaRenderer() : SpriteEntityRenderer("assets/enemies.png") {
}

void GoombaRenderer::renderTyped(sf::RenderTarget& window, const model::Goomba& goomba,
                                 const RenderContext& /* ctx */) const {
    // A dying body shows the squished frame too, so a shell knock-out visibly changes
    // the sprite while it pops up and falls away (a dead goomba has no walk animation).
    const int frameCol =
        (goomba.isSquished() || goomba.isDying()) ? GoombaSquishedFrameCol : GoombaFrameCol;
    drawCharacterFrame(window, goomba,
                       {{frameCol * 16, EnemyFrameTop}, {EnemyFrameSize, EnemyFrameSize}});
}

}
