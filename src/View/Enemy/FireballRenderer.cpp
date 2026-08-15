#include "View/Enemy/FireballRenderer.h"

#include "Model/Projectile/MarioFireball.h"
#include "View/Enemy/EnemyAtlas.h"

#include <SFML/Graphics/RenderTarget.hpp>

#include <iterator>

namespace view {

namespace {
// Time each roll pose is shown. At the ball's travel speed this lands roughly a frame per
// few pixels of travel, which reads as a fast tumble without strobing.
constexpr float FrameDuration = 0.08f;
}

FireballRenderer::FireballRenderer()
    : SpriteEntityRenderer(atlas::FireballSheet, /*sourceFacesRight=*/true) {
}

void FireballRenderer::renderTyped(sf::RenderTarget& window,
                                   const model::MarioFireball& ball,
                                   const RenderContext& /* ctx */) const {
    const std::size_t index =
        static_cast<std::size_t>(ball.getAnimationClock() / FrameDuration)
        % std::size(atlas::FireballRoll);
    drawCharacterFrame(window, ball, atlas::FireballRoll[index]);
}

}
