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

void Mushroom::onBlockHitFromBelow() {
    // Standing on a block that Mario bumps from below: turn around, like a wall turn.
    setDirection(-getDirection());
    velocity.x = WalkSpeed * getDirection();
}

void Mushroom::onCollect(Entity& collector) {
    // All power-up policy lives in Player::applyPowerUp (the size axis vs the ability
    // slot, plus the redundant-power-up points). The item just hands the collect over.
    if (auto* player = dynamic_cast<Player*>(&collector)) {
        player->applyPowerUp(PlayerPowerUp::Mushroom);
    }
    // Consumed: the level reclaims it on the next pass.
    isActive = false;
}

}
