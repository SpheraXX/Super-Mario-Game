#ifndef VIEW_LEVEL_FLAGPOLERENDERER_H
#define VIEW_LEVEL_FLAGPOLERENDERER_H

#include "View/Base/EntityRenderer.h"
#include "View/Base/SpritePainter.h"

namespace model {
class FlagPole;
}

namespace view {

// Draws a FlagPole from blocks.png: the pole stretched onto the tall thin box, a gold
// ball on top and a gold pennant whose slide-down is driven by the clear cinematic.
class FlagPoleRenderer : public TypedEntityRenderer<model::FlagPole> {
public:
    FlagPoleRenderer();

protected:
    void renderTyped(sf::RenderTarget& window, const model::FlagPole& pole,
                     const RenderContext& ctx) const override;

private:
    SpritePainter painter;
};

}

#endif