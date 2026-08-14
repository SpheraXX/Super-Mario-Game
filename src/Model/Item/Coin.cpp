#include "Model/Item/Coin.h"

namespace model {

Coin::Coin(Vector2 position)
    : Item(position, {16.0f, 16.0f}),
      spawnY(position.y) {
    velocity = {0.0f, PopSpeed};
    // Purely decorative: no entity pair should ever resolve against it.
    hitbox.isTrigger = true;
}

void Coin::update(float deltaTime) {
    // Character::update supplies gravity and integration, which is the whole arc.
    Character::update(deltaTime);

    // The arc has a floor: once the coin is falling and reaches the height it popped
    // from, the flourish is over. Leaving it unclamped would let it fall through the
    // very block that produced it.
    if (velocity.y > 0.0f && getPosition().y >= spawnY) {
        isActive = false;
    }
}

void Coin::onCollect(Entity& /* collector */) {
    // Deliberately empty. See the class comment: the bump already paid.
}

}
