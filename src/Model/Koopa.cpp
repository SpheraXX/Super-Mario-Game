#include "Model/Koopa.h"

namespace model {

Koopa::Koopa(Vector2 position)
    : Enemy(position, {16.0f, 24.0f}), // Koopa is taller
      state(KoopaState::Walking),
      shellSpeed(SpinSpeed) {
    velocity.x = -WalkSpeed;
    setDirection(-1);
}

void Koopa::updateAI(float /* deltaTime */) {
    switch (state) {
        case KoopaState::Walking:
            velocity.x = WalkSpeed * getDirection();
            break;
        case KoopaState::ShellIdle:
            velocity.x = 0.0f;
            break;
        case KoopaState::ShellSpinning:
            velocity.x = shellSpeed * getDirection();
            break;
    }
}

void Koopa::onStomped(Entity& player) {
    if (state == KoopaState::Walking) {
        state = KoopaState::ShellIdle;
        velocity.x = 0.0f;
        hitbox.height = 16.0f; // Shrink to shell size
        Vector2 sz = getSize();
        sz.y = 16.0f;
        setSize(sz);
    } else if (state == KoopaState::ShellIdle) {
        state = KoopaState::ShellSpinning;
        // Kick direction based on player relative position
        float playerCenter = player.getPosition().x + player.getSize().x / 2.0f;
        float koopaCenter = getPosition().x + getSize().x / 2.0f;
        setDirection(playerCenter < koopaCenter ? 1 : -1);
        velocity.x = shellSpeed * getDirection();
    } else if (state == KoopaState::ShellSpinning) {
        state = KoopaState::ShellIdle;
        velocity.x = 0.0f;
    }
}

bool Koopa::isShell() const {
    return state != KoopaState::Walking;
}

void Koopa::onCollision(Entity& other, CollisionType /* side */) {
    if (state == KoopaState::ShellSpinning) {
        // A spinning shell knocks out any other enemy it touches.
        if (&other != this && other.hitbox.layer == CollisionLayer::Enemy) {
            other.onHit(*this);
        }
    }
}

void Koopa::onTileCollision(char /* tile */, CollisionType side) {
    if (side == CollisionType::Left || side == CollisionType::Right) {
        setDirection(-getDirection());
        if (state == KoopaState::Walking) {
            velocity.x = WalkSpeed * getDirection();
        } else if (state == KoopaState::ShellSpinning) {
            velocity.x = shellSpeed * getDirection();
        }
    }
}

}
