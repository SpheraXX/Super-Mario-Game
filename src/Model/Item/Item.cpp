#include "Model/Item/Item.h"

namespace model {

Item::Item(Vector2 position, Vector2 size)
    : Character(position, size) {
    // A distinct layer so future collision routing can recognise collectibles without
    // type checks. The current onCollision hook does not actually need it, but it keeps
    // the layer table honest and costs nothing.
    hitbox.layer = CollisionLayer::Item;
}

void Item::onCollision(Entity& other, CollisionType side) {
    (void)side;
    // The only interaction an item cares about is being picked up by the player.
    if (other.hitbox.layer == CollisionLayer::Player) {
        onCollect(other);
    }
}

void Item::onCollect(Entity& /* collector */) {
}

}
