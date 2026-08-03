#include "View/KoopaRenderer.h"

#include "Model/Koopa.h"

#include <SFML/Graphics/RenderWindow.hpp>

namespace view {

namespace {
// Placeholder atlas columns; update once the real enemies spritesheet is finalised.
constexpr int KoopaFrameCol = 2;
constexpr int KoopaShellFrameCol = 3;
}

KoopaRenderer::KoopaRenderer() : SpriteEntityRenderer("assets/enemies.png") {
}

void KoopaRenderer::renderTyped(sf::RenderWindow& window, const model::Koopa& koopa) const {
    const int frameCol = koopa.isShell() ? KoopaShellFrameCol : KoopaFrameCol;
    drawCharacterFrame(window, koopa, {{frameCol * 16, 0}, {16, 32}});
}

}
