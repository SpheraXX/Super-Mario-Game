#include "View/Level/ChainTriggerRenderer.h"

#include "Model/Level/ChainTrigger.h"
#include "View/Base/MiscAtlas.h"

#include <SFML/Graphics/RenderTarget.hpp>

#include <cmath>

namespace view {

ChainTriggerRenderer::ChainTriggerRenderer()
    : painter(atlas::CastleSheet) {
    // Key black only over the axe's own cell. The rest of this sheet uses (148,148,255)
    // as its backdrop and paints real artwork in black, so keying black sheet-wide would
    // punch holes through half the tiles drawn from it elsewhere.
    painter.applyColorKey(atlas::ChainAxe, atlas::CastleColorKey);
    painter.commitColorKeys();
}

void ChainTriggerRenderer::renderTyped(sf::RenderTarget& window,
                                       const model::ChainTrigger& trigger,
                                       const RenderContext& /* ctx */) const {
    if (!painter.isLoaded() || trigger.isTriggered()) {
        return;
    }
    painter.draw(window, atlas::ChainAxe,
                 {std::round(trigger.getPosition().x), std::round(trigger.getPosition().y)},
                 {1.0f, 1.0f});
}

}
