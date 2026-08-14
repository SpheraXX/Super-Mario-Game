#include "Model/Item/Item.h"

namespace model {

Item::Item(Vector2 position, Vector2 size)
    : Character(position, size) {
    // A distinct layer so future collision routing can recognise collectibles without
    // type checks. The current onCollision hook does not actually need it, but it keeps
    // the layer table honest and costs nothing.
    hitbox.layer = CollisionLayer::Item;
}

void Item::update(float deltaTime) {
    // Emergence gate: while the pop-out is running the item is inert and physics-free;
    // it moves only by the rise. The first frame after it has fully cleared the block
    // the pop state is dropped, the subclass gets its one-shot hook, and normal
    // behaviour resumes on the same frame.
    if (emergence) {
        if (emergence->advance(*this, deltaTime)) {
            return;
        }
        emergence.reset();
        onEmergenceComplete();
    }
    updateBehavior(deltaTime);
}

void Item::updateBehavior(float deltaTime) {
    Character::update(deltaTime);
}

void Item::onEmergenceComplete() {
}

bool Item::drawsBehindTerrain() const {
    // While the item is inside (or emerging through) its block it must not overdraw the
    // block face; once fully clear it renders like any other character.
    return emergence && !emergence->isDone();
}

void Item::beginEmergence(Vector2 blockPosition, Vector2 blockSize) {
    emergence = std::make_unique<ItemEmergence>();
    emergence->begin(blockPosition, blockSize);
    // The rise takes full control of motion; the pop is the only thing that can follow
    // an item right after spawn (when the constructor may already have set a velocity).
    velocity = {0.0f, 0.0f};
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