#include "Model/CollisionManager.h"
#include "Model/Entity.h"
#include "Model/TileMap.h"
#include "Model/Player.h"
#include "Model/Enemy.h"
#include "Model/Character.h"
#include "Model/Block.h"
#include "Model/CoinBlock.h"
#include "Model/GameManager.h"

namespace model {

namespace {
// Characters in their death animation ignore all collisions: they pop through the
// world until the level removes them.
bool isDyingBody(const Entity* entity) {
    const auto* character = dynamic_cast<const Character*>(entity);
    return character != nullptr && character->isDying();
}
}

CollisionManager::CollisionManager(TileMap* tileMap) : tileMap(tileMap) {}

void CollisionManager::update(std::vector<Entity*>& entities, float deltaTime) {
    // Pass 1: Entity vs TileMap
    for (auto* entity : entities) {
        if (!entity || !entity->isActive || isDyingBody(entity)) continue;
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

        if (col < tileMap->getColumns() && row < TileMap::Rows) {
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

        if (col < tileMap->getColumns() && row < TileMap::Rows) {
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

        if (col < tileMap->getColumns() && row < TileMap::Rows) {
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
        if (!a || !a->isActive || a->hitbox.isTrigger || isDyingBody(a)) continue;

        for (std::size_t j = i + 1; j < entities.size(); ++j) {
            Entity* b = entities[j];
            if (!b || !b->isActive || b->hitbox.isTrigger || isDyingBody(b)) continue;

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
    Player* player = dynamic_cast<Player*>(&a);
    Enemy* enemy = dynamic_cast<Enemy*>(&b);
    if (!player && !enemy) {
        player = dynamic_cast<Player*>(&b);
        enemy = dynamic_cast<Enemy*>(&a);
    }
    if (player && enemy) {
        // Sides from the player's perspective
        const CollisionType playerSide = (player == &a) ? sideA : sideB;
        if (playerSide == CollisionType::Bottom) {
            // Player landed on top of the enemy: squish/stomp it and bounce.
            enemy->onStomped(*player);
            Vector2 vel = player->getVelocity();
            vel.y = -350.0f; // Bounce force
            player->setVelocity(vel);
        } else {
            // Player hit the enemy from the side or from below: take damage.
            player->takeDamage(enemy->getDamageValue());
        }
        return;
    }

    // Resolve Player vs Block: solid blocks stop the player and react to bumps.
    Player* blockPlayer = dynamic_cast<Player*>(&a);
    Block* block = dynamic_cast<Block*>(&b);
    if (!blockPlayer && !block) {
        blockPlayer = dynamic_cast<Player*>(&b);
        block = dynamic_cast<Block*>(&a);
    }
    if (blockPlayer && block && block->isSolid()) {
        const CollisionType playerSide = (blockPlayer == &a) ? sideA : sideB;
        pushOutOfBlock(*blockPlayer, *block, playerSide);
    }
}

void CollisionManager::pushOutOfBlock(Player& player, Block& block, CollisionType playerSide) {
    const Vector2 blockPos = block.getPosition();
    const Hitbox& blockBox = block.hitbox;

    Vector2 newPos = player.getPosition();
    switch (playerSide) {
        case CollisionType::Top:
            // Landed on the block: stand on top of it.
            newPos.y = blockPos.y + blockBox.offset.y - player.hitbox.offset.y - player.hitbox.height;
            player.setVelocity({player.getVelocity().x, 0.0f});
            player.isGrounded = true;
            break;
        case CollisionType::Bottom:
            // Hit the block from below: bump it, stop the ascent.
            newPos.y = blockPos.y + blockBox.offset.y + blockBox.height - player.hitbox.offset.y;
            player.setVelocity({player.getVelocity().x, 0.0f});
            bumpBlock(block);
            break;
        case CollisionType::Left:
            newPos.x = blockPos.x + blockBox.offset.x - player.hitbox.offset.x - player.hitbox.width;
            player.setVelocity({0.0f, player.getVelocity().y});
            break;
        case CollisionType::Right:
            newPos.x = blockPos.x + blockBox.offset.x + blockBox.width - player.hitbox.offset.x;
            player.setVelocity({0.0f, player.getVelocity().y});
            break;
        default:
            return;
    }
    player.setPosition(newPos);
}

void CollisionManager::bumpBlock(Block& block) {
    if (auto* coinBlock = dynamic_cast<CoinBlock*>(&block)) {
        if (coinBlock->hasCoin()) {
            coinBlock->collectCoin();
            GameManager::instance().addScore(200);
        }
    }
}

}
