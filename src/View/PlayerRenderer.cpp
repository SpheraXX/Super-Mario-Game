#include "View/PlayerRenderer.h"

#include "Model/Player.h"

#include <SFML/Graphics/RenderWindow.hpp>

namespace view {

namespace {
// The player's source frame is 16x32 (width x height). The atlas column below is a
// PLACEHOLDER shared by every animation state — switch on player.getAnimState() once the
// real spritesheet layout is finalised by the graphics teammate.
constexpr int PlayerFrameCol = 0;
}

PlayerRenderer::PlayerRenderer() : SpriteEntityRenderer("assets/characters.png") {
}

void PlayerRenderer::renderTyped(sf::RenderWindow& window, const model::Player& player) const {
    drawCharacterFrame(window, player, {{PlayerFrameCol * 16, 0}, {16, 32}});
}

}
