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
    static constexpr float ThrowSpeedX = 45.0f;
    static constexpr float ThrowSpeedY = -210.0f;
    // A hammer is lobbed, not dropped. Under full gravity this launch speed peaks after a
    // quarter second and lands about a tile and a half away, so the hammer appeared to fall
    // out of the Bro's hand the instant it left it; at this scale the same launch arcs
    // roughly four tiles up and travels close to four across, which is the lazy overhand
    // throw the original has. Composes with the world's gravity like any other character.
    static constexpr float ArcGravityScale = 0.4f;
};

}

#endif
