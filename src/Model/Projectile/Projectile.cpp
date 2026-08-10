#include "Model/Projectile/Projectile.h"

namespace model {

Projectile::Projectile(Vector2 position, Vector2 size, Entity* owner)
    : Character(position, size),
      owner(owner),
      damageValue(1) {
    hitbox.layer = CollisionLayer::Projectile;
}

bool Projectile::firedByPlayer() const {
    return owner != nullptr && owner->hitbox.layer == CollisionLayer::Player;
}

bool Projectile::isTarget(const Entity& other) const {
    if (&other == owner) return false;
    // Projectiles pass through each other rather than trading hits.
    if (other.hitbox.layer == CollisionLayer::Projectile) return false;

    return firedByPlayer() ? other.hitbox.layer == CollisionLayer::Enemy
                           : other.hitbox.layer == CollisionLayer::Player;
}

void Projectile::onCollision(Entity& other, CollisionType /* side */) {
    if (!isActive || !isTarget(other)) return;

    if (firedByPlayer()) {
        // Enemies are knocked out rather than damaged: they have no health pool.
        other.onHit(*this);
    } else {
        other.takeDamage(damageValue);
    }
    expire();
}

void Projectile::expire() {
    isActive = false;
}

}
