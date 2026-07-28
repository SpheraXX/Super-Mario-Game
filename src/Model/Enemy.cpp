#include "Model/Enemy.h"
#include "Model/Player.h"

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
    
    updateAI(deltaTime);
    Character::update(deltaTime);
    
    if (isStomped) {
        despawnTimer -= deltaTime;
        if (despawnTimer <= 0.0f) {
            die();
        }
    }
}

void Enemy::onStomped(Player& /* player */) {
    isStomped = true;
    despawnTimer = 1.0f; // Default 1 second before despawning after stomped
}

void Enemy::onHit(Entity& /* source */) {
    die();
}

}
