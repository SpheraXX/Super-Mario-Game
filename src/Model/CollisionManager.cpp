#include "Model/CollisionManager.h"
#include "Model/Entity.h"
#include "Model/TileMap.h"
#include "Model/Player.h"
#include "Model/Enemy.h"

namespace model {

CollisionManager::CollisionManager(TileMap* tileMap) : tileMap(tileMap) {}

void CollisionManager::update(std::vector<Entity*>& entities, float deltaTime) {
    // Pass 1: Entity vs TileMap
    for (auto* entity : entities) {
        if (!entity || !entity->isActive) continue;
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

        if (col < TileMap::Columns && row < TileMap::Rows) {
            if (tileMap->getTile(row, col) != '.') {
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

        if (col < TileMap::Columns && row < TileMap::Rows) {
            if (tileMap->getTile(row, col) != '.') {
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

        if (col < TileMap::Columns && row < TileMap::Rows) {
            if (tileMap->getTile(row, col) != '.') {
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

        if (col < TileMap::Columns && row < TileMap::Rows) {
            if (tileMap->getTile(row, col) != '.') {
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
        if (!a || !a->isActive || a->hitbox.isTrigger) continue;

        for (std::size_t j = i + 1; j < entities.size(); ++j) {
            Entity* b = entities[j];
            if (!b || !b->isActive || b->hitbox.isTrigger) continue;

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
    // Notify both entities of the collision
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

    // Resolve Player vs Enemy specifically
    Player* p = dynamic_cast<Player*>(&a);
    Enemy* e = dynamic_cast<Enemy*>(&b);
    if (!p && !e) {
        p = dynamic_cast<Player*>(&b);
        e = dynamic_cast<Enemy*>(&a);
        if (p && e) {
            // Swap sides mentally to process logic from Player's perspective
            CollisionType playerSide = (p == &a) ? sideA : sideB;
            if (playerSide == CollisionType::Bottom) {
                // Player landed on top of enemy
                e->onStomped(*p);
                // Player bounce logic
                Vector2 vel = p->getVelocity();
                vel.y = -350.0f; // Bounce force
                p->setVelocity(vel);
            } else {
                // Player hit enemy from side or bottom
                // Enemy's onHit or Player's takeDamage based on state
                // e.g. player->takeDamage(e->damageValue)
            }
        }
    }
}

}
