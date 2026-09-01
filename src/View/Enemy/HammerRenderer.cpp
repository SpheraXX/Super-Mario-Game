#include "View/Enemy/HammerRenderer.h"

#include "Model/Projectile/Hammer.h"
#include "View/Base/MiscAtlas.h"

#include <cmath>

namespace view {

HammerRenderer::HammerRenderer()
    : SpriteEntityRenderer<model::Hammer>(atlas::MiscSheet, atlas::HammerColorKey) {
}

void HammerRenderer::renderTyped(sf::RenderTarget& window, const model::Hammer& hammer,
                                 const RenderContext& /* ctx */) const {
    constexpr int Frames = 4;
    float turns = std::fmod(hammer.getFlightTime(), SpinSeconds) / SpinSeconds;
    if (turns < 0.0f) turns += 1.0f;
    int index = static_cast<int>(turns * Frames);
    if (index >= Frames) index = Frames - 1;

    const sf::IntRect frame = atlas::HammerSpin[index];
    const model::Vector2 drawSize{static_cast<float>(frame.size.x),
                                  static_cast<float>(frame.size.y)};
    // Centre the pose in the collision box, so the hammer turns about its middle rather
    // than pivoting around its top-left corner as the frame shape changes.
    const model::Vector2 offset{(hammer.getSize().x - drawSize.x) * 0.5f,
                                (hammer.getSize().y - drawSize.y) * 0.5f};

    drawCharacterFrame(window, hammer, frame, drawSize, offset);
}

}
