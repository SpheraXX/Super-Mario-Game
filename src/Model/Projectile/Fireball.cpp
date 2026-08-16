#include "Model/Projectile/Fireball.h"

namespace model {

// Wide and flat: 24x8 source artwork, drawn 1:1.
Fireball::Fireball(Vector2 position, Entity* owner, int direction)
    : Projectile(position, {24.0f, 8.0f}, owner) {
    setGravityScale(0.0f);
    velocity = {TravelSpeed * static_cast<float>(direction), 0.0f};
    setDirection(direction);
}

}
