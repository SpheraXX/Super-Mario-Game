#ifndef MODEL_LEVEL_CASTLE_H
#define MODEL_LEVEL_CASTLE_H

#include "Model/Entity.h"

namespace model {

// The goal castle at the far right of every level (spawned in the 16-tile padded zone).
// Solid and static: it caps the map so the player — who has already completed the level
// at the flagpole in front of it — cannot walk out of the world.
class Castle : public Entity {
public:
    Castle(Vector2 position, Vector2 size);

    bool isSolid() const override;
};

}

#endif
