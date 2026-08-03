#include "View/GoombaRenderer.h"

#include "Model/Goomba.h"

#include <SFML/Graphics/RenderWindow.hpp>

namespace view {

namespace {
// Placeholder atlas columns; update once the real enemies spritesheet is finalised.
constexpr int GoombaFrameCol = 0;
constexpr int GoombaSquishedFrameCol = 1;
}

GoombaRenderer::GoombaRenderer() : SpriteEntityRenderer("assets/enemies.png") {
}

void GoombaRenderer::renderTyped(sf::RenderWindow& window, const model::Goomba& goomba) const {
    const int frameCol = goomba.isSquished() ? GoombaSquishedFrameCol : GoombaFrameCol;
    drawCharacterFrame(window, goomba, {{frameCol * 16, 0}, {16, 32}});
}

}
