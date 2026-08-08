#include "View/Player/PlayerRenderer.h"

#include "Model/Player/Player.h"

#include <SFML/Graphics/RenderTarget.hpp>

namespace view {

namespace {
// mario-luigi.png: the small-Mario poses are 16x16 frames sharing one row, each drawn
// facing right (hence the `true` passed to the base renderer, which mirrors them when
// Mario walks left).
constexpr int SmallMarioRow = 272;
constexpr int SmallMarioSize = 16;

// x offset of each pose within that row.
constexpr int StandFrameX = 176;
constexpr int WalkFrameX = 80;
constexpr int RunFrameX = 112;
constexpr int JumpFrameX = 144;

int frameXFor(model::AnimState state) {
    switch (state) {
        case model::AnimState::Walk:
            return WalkFrameX;
        case model::AnimState::Run:
            return RunFrameX;
        // Rising and falling share the airborne pose; there is no separate falling frame.
        case model::AnimState::Jump:
        case model::AnimState::Fall:
            return JumpFrameX;
        // Small Mario has no death frame in this sheet yet, so death falls back to standing.
        case model::AnimState::Die:
        case model::AnimState::Idle:
        default:
            return StandFrameX;
    }
}
}

PlayerRenderer::PlayerRenderer()
    : SpriteEntityRenderer("assets/mario-luigi.png", /*sourceFacesRight=*/true) {
}

void PlayerRenderer::renderTyped(sf::RenderTarget& window, const model::Player& player,
                                 const RenderContext& /* ctx */) const {
    // Big Mario is a 16x32 frame elsewhere in this sheet; wire it up here once the Super/
    // Fire states actually resize the player.
    drawCharacterFrame(window, player,
                       {{frameXFor(player.getAnimState()), SmallMarioRow},
                        {SmallMarioSize, SmallMarioSize}});
}

}
