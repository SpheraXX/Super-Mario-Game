#include "Model/Projectile/Hammer.h"

namespace model {

// One tile: 16x16 source artwork, drawn 1:1.
Hammer::Hammer(Vector2 position, Entity* owner, int direction)
    : Projectile(position, {16.0f, 16.0f}, owner) {
    velocity = {ThrowSpeedX * static_cast<float>(direction), ThrowSpeedY};
    setDirection(direction);
    setGravityScale(ArcGravityScale);
}

}
