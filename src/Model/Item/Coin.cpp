#include "Model/Item/Coin.h"

namespace model {

Coin::Coin(Vector2 position)
    : Item(position, {16.0f, 16.0f}),
      lifetime(Lifetime) {
    velocity = {0.0f, PopSpeed};
    // Purely decorative: no entity pair should ever resolve against it.
    hitbox.isTrigger = true;
}

void Coin::update(float deltaTime) {
    // Character::update supplies gravity and integration, which is the whole arc.
    Character::update(deltaTime);

    lifetime -= deltaTime;
    if (lifetime <= 0.0f) {
        isActive = false;
    }
}

void Coin::onCollect(Entity& /* collector */) {
    // Deliberately empty. See the class comment: the bump already paid.
}

}
