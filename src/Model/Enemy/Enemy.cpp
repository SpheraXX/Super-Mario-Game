#include "Model/Enemy/Enemy.h"

#include "Model/Core/GameManager.h"
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

    updateAI(deltaTime);

    // Turn around before walking off a ledge: probe the cell just ahead of the feet. A
    // ground cell ('G' or a static block cell, which is walkable) keeps the direction;
    // a pit, a map edge, or anything else flips it. Without this the enemies marched
    // into the first pit and were gone before the player ever reached them.
    if (mapPtr && isGrounded && velocity.x != 0.0f) {
        const float aheadX = velocity.x > 0.0f
            ? getPosition().x + hitbox.offset.x + hitbox.width + 2.0f
            : getPosition().x + hitbox.offset.x - 2.0f;
        const float probeY = getPosition().y + hitbox.offset.y + hitbox.height + 4.0f;
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
    awardScore();
}

void Enemy::onHit(Entity& /* source */) {
    // Knocked out (e.g. by a spinning shell): pop up and fall away.
    beginDying(true);
    awardScore();
}

void Enemy::awardScore() const {
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
