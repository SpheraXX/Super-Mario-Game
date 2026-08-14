#include "Model/Item/Mushroom.h"
#include "Model/Player/Player.h"

namespace model {

Mushroom::Mushroom(Vector2 position, int direction)
    : Item(position, {16.0f, 16.0f}) {
    setDirection(direction);
    velocity.x = WalkSpeed * direction;
}

void Mushroom::updateBehavior(float deltaTime) {
    // Keep walking in the current direction; Character::update applies gravity + movement.
    velocity.x = WalkSpeed * getDirection();
    Character::update(deltaTime);
}

void Mushroom::onEmergenceComplete() {
    // Fully clear of the block: start walking away from the side the player bumped from.
    velocity.x = WalkSpeed * getDirection();
}

void Mushroom::onTileCollision(char /* tile */, CollisionType side) {
    // Bounced off a wall: turn around (the tile pass already stopped horizontal motion).
    if (side == CollisionType::Left || side == CollisionType::Right) {
        setDirection(-getDirection());
    }
}

void Mushroom::onCollect(Entity& collector) {
    if (auto* player = dynamic_cast<Player*>(&collector)) {
        // SMB1 behaviour: an extra mushroom when already powered up is worth 1000 points.
        if (player->getState().isSuper() || player->getState().isFire() ||
            player->getState().isStar()) {
            player->addScore(1000);
        } else {
            player->becomeSuper();
        }
    }
    // Consumed: the level reclaims it on the next pass.
    isActive = false;
}

}
