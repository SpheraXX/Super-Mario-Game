#ifndef MODEL_LEVEL_CHAINTRIGGER_H
#define MODEL_LEVEL_CHAINTRIGGER_H

#include "Model/Entity.h"

namespace model {

// The axe at the end of a castle. Touching it cuts every chain tile in the area loose at
// once, so the bridge under Bowser vanishes and anything standing on it drops.
//
// A trigger, not a solid: the player runs THROUGH it, exactly as LevelGoal works. It is
// deliberately not the level goal itself — a castle still ends on its own 'E' — because
// the two want different footprints and, in the original, the axe and the level's end are
// separate beats.
//
// It fires once. Re-arming it would let a player who walked back over the spot ask the
// world to erase an already-empty set of tiles every frame.
class ChainTrigger : public Entity {
public:
    ChainTrigger(Vector2 position, Vector2 size);

    void onTriggerEnter(Entity& other) override;

    bool isTriggered() const { return triggered; }

private:
    bool triggered = false;
};

}

#endif
