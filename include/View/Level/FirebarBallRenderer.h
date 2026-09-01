#ifndef VIEW_LEVEL_FIREBARBALLRENDERER_H
#define VIEW_LEVEL_FIREBARBALLRENDERER_H

#include "View/Base/SpriteEntityRenderer.h"

namespace model {
class FirebarBall;
}

namespace view {

// Draws one firebar flame, choosing its rotation frame from the ball's own sweep angle so
// the flame's art turns with the arm rather than flickering on a timer of its own. The
// four frames are a quarter turn apart, which is exactly the resolution the sheet has.
class FirebarBallRenderer : public SpriteEntityRenderer<model::FirebarBall> {
public:
    FirebarBallRenderer();

protected:
    void renderTyped(sf::RenderTarget& window, const model::FirebarBall& ball,
                     const RenderContext& ctx) const override;
};

}

#endif
