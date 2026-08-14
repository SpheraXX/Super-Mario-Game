#ifndef MODEL_MUSHROOM_H
#define MODEL_MUSHROOM_H

#include "Model/Item/Item.h"

namespace model {

// Super Mushroom: pops out of a ? block and walks along the ground like a tiny Goomba
// (gravity + wall turns, but it happily walks off ledges). Picking it up turns a Small
// player into Super; a player that is already powered up earns 1000 points instead.
class Mushroom : public Item {
public:
    Mushroom(Vector2 position, int direction = 1);

    // The Item emergence gate calls this once the mushroom has fully cleared its block,
    // so it starts walking out of the block rather than drifting.
    void updateBehavior(float deltaTime) override;
    void onEmergenceComplete() override;
    void onTileCollision(char tile, CollisionType side) override;
    void onCollect(Entity& collector) override;

private:
    static constexpr float WalkSpeed = 30.0f;
};

}

#endif
