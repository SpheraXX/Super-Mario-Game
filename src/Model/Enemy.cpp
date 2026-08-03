#include "Model/Enemy.h"

namespace model {

Enemy::Enemy(Vector2 position, Vector2 size)
    : Character(position, size),
      damageValue(1),
      isStomped(false),
      despawnTimer(0.0f) {
    hitbox.layer = CollisionLayer::Enemy;
}

void Enemy::update(float deltaTime) {
    if (!alive) return;

    // Dying bodies just fall; no AI, no stomp state, no squish timer.
    if (isDying()) {
        Character::update(deltaTime);
        return;
    }

    updateAI(deltaTime);
    Character::update(deltaTime);

    if (isStomped) {
        despawnTimer -= deltaTime;
        if (despawnTimer <= 0.0f) {
            die();
        }
    }
}

void Enemy::onStomped(Entity& /* player */) {
    isStomped = true;
    despawnTimer = 1.0f; // Default 1 second before despawning after stomped
}

void Enemy::onHit(Entity& /* source */) {
    // Knocked out (e.g. by a spinning shell): pop up and fall away.
    beginDying(true);
}

int Enemy::getDamageValue() const {
    return damageValue;
}

bool Enemy::isSquished() const {
    return isStomped;
}

}
