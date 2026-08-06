#include "Model/Core/CollisionManager.h"
#include "Model/Entity.h"
#include "Model/Map/TileMap.h"

namespace model {

CollisionManager::CollisionManager(TileMap* tileMap) : tileMap(tileMap) {}

void CollisionManager::update(std::vector<Entity*>& entities, float deltaTime) {
    // Pass 1: Entity vs TileMap. Dying bodies pop through the world, so they skip it, as do
    // entities that drive their own position (fliers, projectiles that pass through walls).
    for (auto* entity : entities) {
        if (!entity || !entity->isActive || entity->isDying()) continue;
        if (!entity->usesTileCollision()) continue;
        entity->isGrounded = false;
        processTileCollisions(entity, deltaTime);
    }

    // Pass 2: Entity vs Entity
    processEntityCollisions(entities);
}

void CollisionManager::processTileCollisions(Entity* entity, float /* deltaTime */) {
    if (!tileMap) return;

    Vector2 pos = entity->getPosition();
    Hitbox& hb = entity->hitbox;
    Vector2 vel = entity->getVelocity();
    
    // Simplistic AABB resolution against tiles
    float footY = pos.y + hb.offset.y + hb.height;
    float headY = pos.y + hb.offset.y;
    float leftX = pos.x + hb.offset.x;
    float rightX = pos.x + hb.offset.x + hb.width;
    float centerY = pos.y + hb.offset.y + hb.height / 2.0f;
    float centerX = pos.x + hb.offset.x + hb.width / 2.0f;

    // Check downwards
    if (vel.y >= 0.0f) {
        std::size_t col = static_cast<std::size_t>(centerX / TileMap::TileWidth);
        std::size_t row = TileMap::Rows - 1 - static_cast<std::size_t>(footY / TileMap::TileHeight);

        if (col < tileMap->getColumns() && row < TileMap::Rows) {
            if (TileMap::isSolidTile(tileMap->getTile(row, col))) {
                pos.y = (TileMap::Rows - 1 - row) * TileMap::TileHeight - hb.height - hb.offset.y;
                vel.y = 0.0f;
                entity->isGrounded = true;
                entity->onTileCollision(tileMap->getTile(row, col), CollisionType::Bottom);
            }
        }
    }

    // Check upwards
    if (vel.y < 0.0f) {
        std::size_t col = static_cast<std::size_t>(centerX / TileMap::TileWidth);
        std::size_t row = TileMap::Rows - 1 - static_cast<std::size_t>(headY / TileMap::TileHeight);

        if (col < tileMap->getColumns() && row < TileMap::Rows) {
            if (TileMap::isSolidTile(tileMap->getTile(row, col))) {
                pos.y = (TileMap::Rows - row) * TileMap::TileHeight - hb.offset.y;
                vel.y = 0.0f;
                entity->onTileCollision(tileMap->getTile(row, col), CollisionType::Top);
            }
        }
    }

    // Check left
    if (vel.x < 0.0f) {
        std::size_t col = static_cast<std::size_t>(leftX / TileMap::TileWidth);
        std::size_t row = TileMap::Rows - 1 - static_cast<std::size_t>(centerY / TileMap::TileHeight);

        if (col < tileMap->getColumns() && row < TileMap::Rows) {
            if (TileMap::isSolidTile(tileMap->getTile(row, col))) {
                pos.x = (col + 1) * TileMap::TileWidth - hb.offset.x;
                vel.x = 0.0f;
                entity->onTileCollision(tileMap->getTile(row, col), CollisionType::Left);
            }
        }
    }

    // Check right
    if (vel.x > 0.0f) {
        std::size_t col = static_cast<std::size_t>(rightX / TileMap::TileWidth);
        std::size_t row = TileMap::Rows - 1 - static_cast<std::size_t>(centerY / TileMap::TileHeight);

        if (col < tileMap->getColumns() && row < TileMap::Rows) {
            if (TileMap::isSolidTile(tileMap->getTile(row, col))) {
                pos.x = col * TileMap::TileWidth - hb.width - hb.offset.x;
                vel.x = 0.0f;
                entity->onTileCollision(tileMap->getTile(row, col), CollisionType::Right);
            }
        }
    }

    entity->setPosition(pos);
    entity->setVelocity(vel);
}

void CollisionManager::processEntityCollisions(std::vector<Entity*>& entities) {
    for (std::size_t i = 0; i < entities.size(); ++i) {
        Entity* a = entities[i];
        if (!a || !a->isActive || a->hitbox.isTrigger || a->isDying()) continue;

        for (std::size_t j = i + 1; j < entities.size(); ++j) {
            Entity* b = entities[j];
            if (!b || !b->isActive || b->hitbox.isTrigger || b->isDying()) continue;

            if (a->hitbox.intersects(b->hitbox, a->getPosition(), b->getPosition())) {
                CollisionType sideA = calculateSide(*a, *b);
                resolveEntityInteraction(*a, *b, sideA);
            }
        }
    }
}

CollisionType CollisionManager::calculateSide(const Entity& a, const Entity& b) const {
    Vector2 overlap = a.hitbox.getOverlap(b.hitbox, a.getPosition(), b.getPosition());
    
    // The smaller overlap axis dictates the collision side
    if (overlap.x < overlap.y) {
        // Horizontal collision
        float aCenterX = a.getPosition().x + a.hitbox.offset.x + a.hitbox.width / 2.0f;
        float bCenterX = b.getPosition().x + b.hitbox.offset.x + b.hitbox.width / 2.0f;
        return (aCenterX < bCenterX) ? CollisionType::Right : CollisionType::Left;
    } else {
        // Vertical collision
        float aCenterY = a.getPosition().y + a.hitbox.offset.y + a.hitbox.height / 2.0f;
        float bCenterY = b.getPosition().y + b.hitbox.offset.y + b.hitbox.height / 2.0f;
        return (aCenterY < bCenterY) ? CollisionType::Bottom : CollisionType::Top;
    }
}

void CollisionManager::resolveEntityInteraction(Entity& a, Entity& b, CollisionType sideA) {
    // Notify both entities of the collision: each one reacts through its own hooks
    // (e.g. CoinBlock collects its coin when bumped from below).
    a.onCollision(b, sideA);
    
    // Invert side for b
    CollisionType sideB = CollisionType::None;
    switch (sideA) {
        case CollisionType::Top: sideB = CollisionType::Bottom; break;
        case CollisionType::Bottom: sideB = CollisionType::Top; break;
        case CollisionType::Left: sideB = CollisionType::Right; break;
        case CollisionType::Right: sideB = CollisionType::Left; break;
        default: break;
    }
    b.onCollision(a, sideB);

    // Routing uses the collision layers instead of concrete types. Exactly one of the
    // two entities is the player; the other decides how the pair interacts.
    const bool aIsPlayer = a.hitbox.layer == CollisionLayer::Player;
    const bool bIsPlayer = b.hitbox.layer == CollisionLayer::Player;
    if (aIsPlayer == bIsPlayer) return;

    Entity* player = aIsPlayer ? &a : &b;
    Entity* other = aIsPlayer ? &b : &a;
    const CollisionType playerSide = (player == &a) ? sideA : sideB;

    if (other->hitbox.layer == CollisionLayer::Projectile) {
        // Projectiles settle their own outcome in Projectile::onCollision, which already ran
        // above — they check who fired them, so the same class serves Bowser and Mario.
        // Deliberately no stomp branch: landing on a hammer must not bounce the player.
        return;
    }

    if (other->hitbox.layer == CollisionLayer::Enemy) {
        if (playerSide == CollisionType::Bottom && other->isStompable()) {
            // Player landed on top of the enemy: squash it and bounce.
            other->onStomped(*player);
            player->setVelocity({player->getVelocity().x, -350.0f}); // Bounce force
        } else {
            // Player hit the enemy from the side or below — or landed on something that
            // cannot be stomped at all (Spiny's spikes, Bowser): take damage.
            player->takeDamage(other->getDamageValue());
        }
    } else if (other->isSolid()) {
        // Solid blocks stop the player (the bumped block reacts through onCollision).
        pushOutOfBlock(*player, *other, playerSide);
    }
}

void CollisionManager::pushOutOfBlock(Entity& mover, const Entity& blocker, CollisionType moverSide) {
    // Side semantics: moverSide is the side of the mover that is in contact, i.e.
    // Bottom = the mover's bottom face rests on the blocker's top (stand on it),
    // Top = the mover's top face hit the blocker's bottom (bump, stop the ascent).
    const Vector2 blockerPos = blocker.getPosition();
    const Hitbox& blockerBox = blocker.hitbox;

    Vector2 newPos = mover.getPosition();
    switch (moverSide) {
        case CollisionType::Bottom:
            newPos.y = blockerPos.y + blockerBox.offset.y - mover.hitbox.offset.y - mover.hitbox.height;
            mover.setVelocity({mover.getVelocity().x, 0.0f});
            mover.isGrounded = true;
            break;
        case CollisionType::Top:
            newPos.y = blockerPos.y + blockerBox.offset.y + blockerBox.height - mover.hitbox.offset.y;
            mover.setVelocity({mover.getVelocity().x, 0.0f});
            break;
        case CollisionType::Right:
            // The mover's right face is in contact: it sits to the LEFT of the blocker,
            // so its right edge is pinned to the blocker's left edge.
            newPos.x = blockerPos.x + blockerBox.offset.x - mover.hitbox.offset.x - mover.hitbox.width;
            mover.setVelocity({0.0f, mover.getVelocity().y});
            break;
        case CollisionType::Left:
            // The mover's left face is in contact: it sits to the RIGHT of the blocker.
            newPos.x = blockerPos.x + blockerBox.offset.x + blockerBox.width - mover.hitbox.offset.x;
            mover.setVelocity({0.0f, mover.getVelocity().y});
            break;
        default:
            return;
    }
    mover.setPosition(newPos);
}

}
