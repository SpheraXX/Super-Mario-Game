#include "Model/Projectile/Hammer.h"

namespace model {

// One tile: 16x16 source artwork at 2x.
Hammer::Hammer(Vector2 position, Entity* owner, int direction)
    : Projectile(position, {32.0f, 32.0f}, owner) {
    velocity = {ThrowSpeedX * static_cast<float>(direction), ThrowSpeedY};
    setDirection(direction);
}

}
