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

    void update(float deltaTime) override;
    void onTileCollision(char tile, CollisionType side) override;
    void onCollect(Entity& collector) override;

private:
    static constexpr float WalkSpeed = 60.0f;
};

}

#endif
