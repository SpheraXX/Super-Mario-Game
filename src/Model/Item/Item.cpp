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
    emergence = std::make_unique<VerticalSlide>();
    emergence->begin(getPosition().y, blockPosition.y - blockSize.y, VerticalSlide::RiseSpeed);
    // The rise takes full control of motion; the pop is the only thing that can follow
    // an item right after spawn (when the constructor may already have set a velocity).
    velocity = {0.0f, 0.0f};
}

void Item::onCollision(Entity& other, CollisionType side) {
    (void)side;
    // While the item is still emerging through its block it is not collectible. The bump
    // frame's pre-push overlap puts the player's head inside the block cell, which also
    // contains the freshly spawned item, and the star's bigger box makes that window wider
    // — collecting on the spot would make the item vanish before it ever rolls out (it
    // reads as "the star killed the mushroom"). The item only becomes collectible once the
    // rise has cleared the block face.
    if (emergence) return;
    // The only interaction an item cares about is being picked up by the player.
    if (other.hitbox.layer == CollisionLayer::Player) {
        onCollect(other);
    }
}

void Item::onCollect(Entity& /* collector */) {
}

}