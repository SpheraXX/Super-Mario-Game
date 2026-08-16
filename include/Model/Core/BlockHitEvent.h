#ifndef MODEL_BLOCKHITEVENT_H
#define MODEL_BLOCKHITEVENT_H

#include "Model/Core/CollisionResult.h"

namespace model {

class Entity;

// Dispatched by the collision system when the player bumps a solid block from below.
// Blocks react through Block::onBlockHit(const BlockHitEvent&), which returns whether the
// block actually reacted — the event carries the colliding player, the side of the player
// that made contact, and the head's upward speed at impact, so the block never needs to
// know which concrete player type (or how fast it was moving) it is dealing with.
struct BlockHitEvent {
    Entity& player;
    CollisionType side;
    // Upward speed of the player's head at the moment of contact (>= 0). A head that
    // grazes a block at the very top of a jump has almost no speed left; the collision
    // system only dispatches events for contacts strong enough to count as a bump.
    float upwardSpeed;
};

}

#endif
