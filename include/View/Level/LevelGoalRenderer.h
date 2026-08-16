#ifndef VIEW_LEVEL_LEVELGOALRENDERER_H
#define VIEW_LEVEL_LEVELGOALRENDERER_H

#include "View/Base/EntityRenderer.h"
#include "View/Base/SpritePainter.h"

namespace model {
class LevelGoal;
}

namespace view {

// Draws a LevelGoal as a static pole + gold ball + pennant, the same blocks.png artwork
// the old animated flagpole used to end a level, minus the slide-down cinematic: the goal
// has no cinematic of its own, so the pennant simply sits at rest.
//
// The visual pole is shorter than the entity's own hitbox on purpose (see LevelScene's
// spawn case: the trigger box is several cells tall so a jumping player cannot skip over
// it) — the marker reads as a normal-height pole while the taller box underneath still
// catches contact reliably.
class LevelGoalRenderer : public TypedEntityRenderer<model::LevelGoal> {
public:
    LevelGoalRenderer();

protected:
    void renderTyped(sf::RenderTarget& window, const model::LevelGoal& goal,
                     const RenderContext& ctx) const override;

private:
    SpritePainter painter;
};

}

#endif
