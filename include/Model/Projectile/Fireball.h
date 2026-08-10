#ifndef MODEL_FIREBALL_H
#define MODEL_FIREBALL_H

#include "Model/Projectile/Projectile.h"

namespace model {

// Bowser's fire breath: travels horizontally at a constant height, unaffected by gravity or
// terrain. Mario's bouncing fireball is a separate class (MarioFireball) because it falls
// under gravity, resolves against tiles, and rolls — but who a projectile damages is always
// decided by its owner, not its type.
class Fireball : public Projectile {
public:
    Fireball(Vector2 position, Entity* owner, int direction);

    bool usesTileCollision() const override { return false; }

private:
    static constexpr float TravelSpeed = 200.0f;
};

}

#endif
