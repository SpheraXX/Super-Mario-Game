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
    // shell (23 world units tall down to 16), so the frame and the box stay in proportion
    // without the renderer having to compensate.
    if (koopa.isDying()) {
        // A Koopa dies as its shell: the death-bounce body shows the shell frame, tucked
        // down to the foot line of the 23-tall body box (16-tall shell, 7px offset) — the
        // same drop the model performs when entering the shell state. Purely presentational:
        // the model still dies exactly like any other enemy (bounce + score, no state
        // change), so a walking Koopa keeps walking right up until the hit that kills it.
        drawCharacterFrame(window, koopa, atlas::KoopaShell,
                           model::Vector2{16.0f, 16.0f}, model::Vector2{0.0f, 7.0f});
    } else if (koopa.isShell()) {
        drawCharacterFrame(window, koopa, atlas::KoopaShell);
    } else if (koopa.isWinged()) {
        drawCharacterFrame(window, koopa, atlas::KoopaParatroopa);
    } else {
        drawCharacterFrame(window, koopa, atlas::Koopa);
    }
}

}
