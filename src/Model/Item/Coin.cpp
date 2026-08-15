#include "Model/Item/Coin.h"

#include "Model/World/WorldTheme.h"

namespace model {

Coin::Coin(Vector2 position)
    : Item(position, {16.0f, 16.0f}),
      spawnY(position.y) {
    // The pop velocity is seeded on the first update: it depends on the world, which may
    // not be attached yet in the constructor. The ctor constant is only the fallback.
    // Purely decorative: no entity pair should ever resolve against it.
    hitbox.isTrigger = true;
}

void Coin::update(float deltaTime) {
    // Seed the pop velocity once, from the world's coin pop (1.5x its death bounce).
    // The world is attached by the time any coin updates, so the constant only ever
    // fires for world-less test bodies.
    if (!popInitialized) {
        velocity.y = worldPtr ? worldPtr->getCoinPopSpeed() : PopSpeed;
        popInitialized = true;
    }
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
