#ifndef MODEL_SPINYEGG_H
#define MODEL_SPINYEGG_H

#include "Model/Projectile/Projectile.h"

namespace model {

// Dropped by Lakitu. Falls under gravity, hurts the player on the way down, and is replaced
// by a Spiny the moment it touches the ground.
//
// That hatch is the reason World::spawn takes an Entity rather than a Projectile: a
// projectile's terminal act here is to produce an enemy.
class SpinyEgg : public Projectile {
public:
    SpinyEgg(Vector2 position, Entity* owner, float throwVelocityX);

    void onTileCollision(char tile, CollisionType side) override;

private:
    void hatch();
};

}

#endif
