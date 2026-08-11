#include "Model/Projectile/Fireball.h"

namespace model {

// Wide and flat: 24x8 source artwork at 2x.
Fireball::Fireball(Vector2 position, Entity* owner, int direction)
    : Projectile(position, {48.0f, 16.0f}, owner) {
    setGravityScale(0.0f);
    velocity = {TravelSpeed * static_cast<float>(direction), 0.0f};
    setDirection(direction);
}

}
