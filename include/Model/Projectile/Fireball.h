#ifndef MODEL_FIREBALL_H
#define MODEL_FIREBALL_H

#include "Model/Projectile/Projectile.h"

namespace model {

// Bowser's fire breath: travels horizontally at a constant height, unaffected by gravity or
// terrain. The same class will serve Mario's fireball later with a non-zero gravity scale —
// who it damages is decided by its owner, not by its type.
class Fireball : public Projectile {
public:
    Fireball(Vector2 position, Entity* owner, int direction);

    bool usesTileCollision() const override { return false; }

private:
    static constexpr float TravelSpeed = 200.0f;
};

}

#endif
