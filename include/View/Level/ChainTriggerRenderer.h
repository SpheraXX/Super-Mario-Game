#ifndef VIEW_LEVEL_CHAINTRIGGERRENDERER_H
#define VIEW_LEVEL_CHAINTRIGGERRENDERER_H

#include "View/Base/EntityRenderer.h"
#include "View/Base/SpritePainter.h"

namespace model {
class ChainTrigger;
}

namespace view {

// Draws the axe. Uses SpritePainter rather than SpriteEntityRenderer because a
// ChainTrigger is a plain Entity, not a Character: it has no facing and no death pose, so
// the character-frame path has nothing to offer it.
//
// The axe stops drawing once it has been taken, matching the original — leaving it on
// screen next to a bridge that is already gone reads as a switch that did not work.
class ChainTriggerRenderer : public TypedEntityRenderer<model::ChainTrigger> {
public:
    ChainTriggerRenderer();

protected:
    void renderTyped(sf::RenderTarget& window, const model::ChainTrigger& trigger,
                     const RenderContext& ctx) const override;

private:
    SpritePainter painter;
};

}

#endif
