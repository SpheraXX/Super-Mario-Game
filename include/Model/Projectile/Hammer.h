#ifndef MODEL_HAMMER_H
#define MODEL_HAMMER_H

#include "Model/Projectile/Projectile.h"

namespace model {

// Thrown by a Hammer Bro: launched up and forward, then falling under gravity. It ignores
// terrain, matching the original's hammers arcing over and through brick structures, and
// leaves play when it drops out of the world or off the camera.
class Hammer : public Projectile {
public:
    Hammer(Vector2 position, Entity* owner, int direction);

    bool usesTileCollision() const override { return false; }

private:
    static constexpr float ThrowSpeedX = 90.0f;
    static constexpr float ThrowSpeedY = -420.0f;
};

}

#endif
