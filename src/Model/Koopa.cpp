#include "Model/Koopa.h"
#include "Model/Player.h"

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

void Koopa::onStomped(Player& player) {
    if (state == KoopaState::Walking) {
        state = KoopaState::ShellIdle;
        velocity.x = 0.0f;
        hitbox.height = 16.0f; // Shrink to shell size
        Vector2 sz = getSize();
        sz.y = 16.0f;
        setSize(sz); // Wait, setSize doesn't exist in Entity, let me check. Entity has getSize but no setSize. I'll need to modify Entity.h to add setSize or just access size directly if protected. Wait, size is private in Entity. I'll just adjust position.
        // I will add setSize to Entity or just use a workaround. Actually, let's just add setSize to Entity.
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

void Koopa::onCollision(Entity& other, CollisionType /* side */) {
    if (state == KoopaState::ShellSpinning) {
        if (auto* enemy = dynamic_cast<Enemy*>(&other)) {
            // Note: avoid hitting self
            if (enemy != this) {
                enemy->onHit(*this);
            }
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
