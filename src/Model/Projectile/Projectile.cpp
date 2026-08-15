#include "Model/Projectile/Projectile.h"

#include "Model/Enemy/Enemy.h"

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

    // isTarget() has already established the other side's collision layer, and that layer
    // is the contract for these casts: only Enemy subclasses carry the Enemy layer, and
    // only the player carries the Player layer. onHit/takeDamage are Enemy/Character
    // capabilities rather than Entity ones — a pipe is never knocked out.
    if (firedByPlayer()) {
        // Enemies are knocked out rather than damaged: they have no health pool.
        if (auto* enemy = dynamic_cast<Enemy*>(&other)) {
            enemy->onHit(*this);
        }
    } else {
        if (auto* character = dynamic_cast<Character*>(&other)) {
            character->takeDamage(damageValue);
        }
    }
    expire();
}

void Projectile::expire() {
    isActive = false;
}

}
