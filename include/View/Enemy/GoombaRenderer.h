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
    void renderTyped(sf::RenderTarget& window, const model::Goomba& goomba) const override;
};

}

#endif
