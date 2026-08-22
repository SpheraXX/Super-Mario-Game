#include "Model/Enemy/Enemy.h"

#include "Model/Core/GameManager.h"
#include "Model/Core/LogManager.h"
#include "Model/Core/World.h"
#include "Model/Map/TileMap.h"
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
    if (!alive) {
        // Dying bodies still integrate: Character::update runs the death fall (pop up,
        // then fall through the world) so the level bounds can despawn the body. An early
        // return here would freeze a shell-killed body in place forever — still rendered,
        // collision-free, never removed.
        Character::update(deltaTime);
        return;
    }

    if (stompLockout > 0.0f) {
        stompLockout -= deltaTime;
    }

    if (isStomped) {
        // A squished body is a corpse: it neither thinks (updateAI) nor moves. Skipping
        // Character::update here is what makes the flattened sprite sit exactly where the
        // enemy fell — gravity would otherwise keep pulling it (a squished Goomba is an
        // isTrigger body, so the tile pass skips it, never grounds it, and it sinks into
        // the floor while pretending to play the death bounce). Only the despawn countdown
        // runs; die() then lets the level bounds remove the body.
        despawnTimer -= deltaTime;
        if (despawnTimer <= 0.0f) {
            die();
        }
        return;
    }

    updateAI(deltaTime);

    // Turn around before walking off a ledge: probe the cell just ahead of the feet. A
    // ground cell ('G' or a static block cell, which is walkable) keeps the direction;
    // a pit, a map edge, or anything else flips it. Without this the enemies marched
    // into the first pit and were gone before the player ever reached them.
    if (mapPtr && isGrounded && velocity.x != 0.0f) {
        const float aheadX = velocity.x > 0.0f
            ? getPosition().x + hitbox.offset.x + hitbox.width + 1.0f
            : getPosition().x + hitbox.offset.x - 1.0f;
        const float probeY = getPosition().y + hitbox.offset.y + hitbox.height + 2.0f;
        const std::size_t col = static_cast<std::size_t>(aheadX / TileMap::TileWidth);
        const std::size_t row = TileMap::Rows - 1 - static_cast<std::size_t>(probeY / TileMap::TileHeight);

        bool walkable = false;
        if (col < mapPtr->getColumns() && row < TileMap::Rows) {
            const char ahead = mapPtr->getTile(row, col);
            walkable = ahead == 'G' || ahead == 'C' || ahead == 'B' || ahead == '#';
        }
        if (!walkable) {
            setDirection(-getDirection());
            velocity.x = -velocity.x;
        }
    }

    updateAttack(deltaTime);
    Character::update(deltaTime);
}

void Enemy::stompedBy(Entity& player) {
    if (!acceptsPlayerContact()) {
        return;
    }
    stompLockout = StompLockoutTime;
    onStomped(player);
}

void Enemy::holdStompLockout() {
    stompLockout = StompLockoutTime;
}

bool Enemy::acceptsPlayerContact() const {
    // A squished body is a corpse counting down to its despawn: it can neither be stomped
    // again nor hurt the player it is lying under.
    return stompLockout <= 0.0f && !isStomped;
}

void Enemy::onStomped(Entity& /* player */) {
    isStomped = true;
    despawnTimer = 1.0f; // Default 1 second before despawning after stomped
    // The body freezes on the spot: no walk speed, no drift. Subclasses that shrink
    // (Goomba) or change shape (Koopa's shell) still adjust velocity in their own
    // onStomped, but every squished enemy stops moving here.
    velocity = {0.0f, 0.0f};
    awardScore();
}

void Enemy::onHit(Entity& /* source */) {
    // Knocked out (e.g. by a spinning shell): pop up and fall away.
    beginDying(true);
    awardScore();
}

void Enemy::awardScore() const {
    LogManager::instance().info("Enemy defeated");
    GameManager::instance().addScore(getScoreValue());
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
