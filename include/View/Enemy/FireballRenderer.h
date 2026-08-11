#ifndef VIEW_FIREBALLRENDERER_H
#define VIEW_FIREBALLRENDERER_H

#include "View/Base/SpriteEntityRenderer.h"

namespace model {
class MarioFireball;
}

namespace view {

// Draws Mario's rolling fireball from enemies.png: the four 7x7 poses in atlas::FireballRoll
// cycle on a fixed timer driven by the ball's own clock. No per-pose drawing logic beyond
// that, and no mirroring concern — the roll reads the same both ways.
class FireballRenderer : public SpriteEntityRenderer<model::MarioFireball> {
public:
    FireballRenderer();

protected:
    void renderTyped(sf::RenderTarget& window, const model::MarioFireball& ball,
                     const RenderContext& ctx) const override;
};

}

#endif
