#include "View/Level/FirebarBallRenderer.h"

#include "Model/Level/FirebarBall.h"
#include "View/Base/MiscAtlas.h"

#include <cmath>

namespace view {

FirebarBallRenderer::FirebarBallRenderer()
    : SpriteEntityRenderer<model::FirebarBall>(atlas::MiscSheet, atlas::MiscColorKey) {
}

void FirebarBallRenderer::renderTyped(sf::RenderTarget& window,
                                      const model::FirebarBall& ball,
                                      const RenderContext& /* ctx */) const {
    // Map the sweep angle onto the four quarter-turn poses. fmod can return a negative
    // remainder for a bar spinning the other way, so the result is normalised before it
    // indexes the array.
    constexpr float TwoPi = 6.283185307f;
    constexpr int Frames = 4;
    float turns = std::fmod(ball.getSpinAngle(), TwoPi) / TwoPi;
    if (turns < 0.0f) turns += 1.0f;
    int index = static_cast<int>(turns * Frames);
    if (index >= Frames) index = Frames - 1;

    drawCharacterFrame(window, ball, atlas::FirebarBall[index]);
}

}
