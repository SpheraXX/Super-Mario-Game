#include "View/Enemy/GoombaRenderer.h"

#include "Model/Enemy/Goomba.h"
#include "View/Enemy/EnemyAtlas.h"

#include <SFML/Graphics/RenderTarget.hpp>

namespace view {

GoombaRenderer::GoombaRenderer()
    : SpriteEntityRenderer(atlas::EnemySheet, atlas::EnemyColorKey) {
}

void GoombaRenderer::renderTyped(sf::RenderTarget& window, const model::Goomba& goomba) const {
    if (!goomba.isSquished()) {
        drawCharacterFrame(window, goomba, atlas::Goomba);
        return;
    }

    // The squished frame is half height (16x8 source). Drawing it over the Goomba's full box
    // would stretch it back to normal size, so it is drawn at its own height and pushed down
    // to sit on the ground where the Goomba's feet were.
    const model::Vector2 size = goomba.getSize();
    const model::Vector2 squishedSize{size.x, size.y / 2.0f};
    drawCharacterFrame(window, goomba, atlas::GoombaStomped, squishedSize,
                       {0.0f, size.y - squishedSize.y});
}

}
