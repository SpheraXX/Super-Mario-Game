#ifndef VIEW_KOOPARENDERER_H
#define VIEW_KOOPARENDERER_H

#include "View/Base/SpriteEntityRenderer.h"

namespace model {
class Koopa;
}

namespace view {

// Draws Koopas from the enemies spritesheet (walking or shell frame).
class KoopaRenderer : public SpriteEntityRenderer<model::Koopa> {
public:
    KoopaRenderer();

protected:
    void renderTyped(sf::RenderTarget& window, const model::Koopa& koopa,
                     const RenderContext& ctx) const override;

    // A shell-killed Koopa pops upside-down (walking frame or shell, whichever it died in).
    bool flipWhenDying() const override { return true; }
};

}

#endif
