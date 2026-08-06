#include "View/Enemy/KoopaRenderer.h"

#include "Model/Enemy/Koopa.h"

#include <SFML/Graphics/RenderTarget.hpp>

namespace view {

namespace {
// Same tight-frame window as the Goomba: art occupies rows 4-19 of each 16x32 atlas cell.
constexpr int EnemyFrameTop = 4;
constexpr int EnemyFrameSize = 16;

// Placeholder atlas columns; update once the real enemies spritesheet is finalised.
// (Column 2 currently holds a Goomba frame, so the Koopa renders as one for now.)
constexpr int KoopaFrameCol = 2;
constexpr int KoopaShellFrameCol = 3;
}

KoopaRenderer::KoopaRenderer() : SpriteEntityRenderer("assets/enemies.png") {
}

void KoopaRenderer::renderTyped(sf::RenderTarget& window, const model::Koopa& koopa) const {
    const int frameCol = koopa.isShell() ? KoopaShellFrameCol : KoopaFrameCol;
    drawCharacterFrame(window, koopa,
                       {{frameCol * 16, EnemyFrameTop}, {EnemyFrameSize, EnemyFrameSize}});
}

}
