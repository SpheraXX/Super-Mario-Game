#ifndef MODEL_LEVEL_LEVELGOAL_H
#define MODEL_LEVEL_LEVELGOAL_H

#include "Model/Entity.h"

namespace model {

// The end-of-level marker: author-placed via TileMap::GoalSymbol ('E'), one per map (or
// per area, if an author wants an early exit). Touching it ends the run immediately — see
// LevelScene::update, which reports Event::ClearTriggered the frame isTouched() turns true.
//
// Trigger-only, like the flagpole it replaces: it carries a tall hitbox (see LevelScene's
// spawn case) so a player mid-jump cannot skip over it, but it never blocks movement.
class LevelGoal : public Entity {
public:
    LevelGoal(Vector2 position, Vector2 size);

    void onTriggerEnter(Entity& other) override;

    bool isTouched() const;

private:
    bool touched = false;
};

}

#endif
