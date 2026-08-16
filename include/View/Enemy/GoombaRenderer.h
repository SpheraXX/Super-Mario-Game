#ifndef VIEW_GOOMBARENDERER_H
#define VIEW_GOOMBARENDERER_H

#include "View/Base/SpriteEntityRenderer.h"

namespace model {
class Goomba;
}

namespace view {

// Draws Goombas from the enemies spritesheet (normal or squished frame).
class GoombaRenderer : public SpriteEntityRenderer<model::Goomba> {
public:
    GoombaRenderer();

protected:
    void renderTyped(sf::RenderTarget& window, const model::Goomba& goomba,
                     const RenderContext& ctx) const override;

    // A shell-kicked Goomba pops upside-down; a stomped one is squished (not dying),
    // so its flattened frame is never flipped.
    bool flipWhenDying() const override { return true; }
};

}

#endif
