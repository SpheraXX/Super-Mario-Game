#include "Model/Enemy/Enemy.h"

#include "Model/Core/World.h"
#include "Model/Projectile/Projectile.h"

namespace model {

Enemy::Enemy(Vector2 position, Vector2 size)
    : Character(position, size),
      damageValue(1),
      isStomped(false),
      despawnTimer(0.0f) {
    hitbox.layer = CollisionLayer::Enemy;
}

void Enemy::update(float deltaTime) {
    // Dying bodies just fall; no AI, no stomp state, no squish timer.
    //
    // This has to be tested before `alive`, not after: beginDying() clears `alive` at the
    // same time as it sets the dying flag, so an earlier `if (!alive) return` swallowed the
    // death fall entirely. The body then froze in mid-air and was never reclaimed, because
    // the level only removes a dying entity once it drops past the bottom of the world.
    if (isDying()) {
        Character::update(deltaTime);
        return;
    }

    if (!alive) return;

    updateAI(deltaTime);
    updateAttack(deltaTime);
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

std::unique_ptr<Projectile> Enemy::createProjectile() {
    return nullptr;  // most enemies do not attack
}

void Enemy::updateAttack(float deltaTime) {
    // A stomped enemy is mid-despawn and stops attacking.
    if (attackCooldown <= 0.0f || isStomped || world == nullptr) return;

    attackTimer -= deltaTime;
    if (attackTimer > 0.0f) return;

    attackTimer = attackCooldown;
    if (auto projectile = createProjectile()) {
        world->spawn(std::move(projectile));
    }
}

const Entity* Enemy::findPlayer() const {
    return world ? world->getPlayer() : nullptr;
}

}
