#ifndef MODEL_LEVEL_FLAGPOLE_H
#define MODEL_LEVEL_FLAGPOLE_H

#include "Model/Entity.h"

namespace model {

// The end-of-level flagpole (SMB 1-1 style). It carries a tall TRIGGER hitbox: touching
// it marks the level as complete through the onTriggerEnter hook (fired by the trigger
// pass in CollisionManager). The pole itself is passable — the level ends the moment
// Mario reaches it, so it never needs to block movement.
class FlagPole : public Entity {
public:
    FlagPole(Vector2 position, Vector2 size);

    void onTriggerEnter(Entity& other) override;

    bool isTouched() const;

private:
    bool touched = false;
};

}

#endif
