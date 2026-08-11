#include "View/Enemy/KoopaRenderer.h"

#include "Model/Enemy/Koopa.h"
#include "View/Enemy/EnemyAtlas.h"

#include <SFML/Graphics/RenderTarget.hpp>

namespace view {

KoopaRenderer::KoopaRenderer()
    : SpriteEntityRenderer(atlas::EnemySheet, atlas::EnemyColorKey) {
}

void KoopaRenderer::renderTyped(sf::RenderTarget& window, const model::Koopa& koopa,
                                const RenderContext& /* ctx */) const {
    // One renderer covers the whole demotion ladder, because it is all one entity:
    // Paratroopa -> walking Koopa -> shell. The model shrinks its own box when it enters the
    // shell (64 world units tall down to 32), so the frame and the box stay in proportion
    // without the renderer having to compensate.
    if (koopa.isShell()) {
        drawCharacterFrame(window, koopa, atlas::KoopaShell);
    } else if (koopa.isWinged()) {
        drawCharacterFrame(window, koopa, atlas::KoopaParatroopa);
    } else {
        drawCharacterFrame(window, koopa, atlas::Koopa);
    }
}

}
