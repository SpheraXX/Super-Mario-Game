#include "Model/Core/CollisionManager.h"
#include "Model/Entity.h"
#include "Model/Map/TileMap.h"
#include "Model/Player/Player.h"
#include "Model/Player/Player.h"

namespace model {

namespace {
// A contact must have real area, but never rely on exact floating-point equality for it.
constexpr float ContactEpsilon = 0.01f;

CollisionType invertCollisionSide(CollisionType side) {
    switch (side) {
        case CollisionType::Top: return CollisionType::Bottom;
        case CollisionType::Bottom: return CollisionType::Top;
        case CollisionType::Left: return CollisionType::Right;
        case CollisionType::Right: return CollisionType::Left;
        default: return CollisionType::None;
    }
}

float horizontalOverlap(const Entity& first, const Entity& second) {
    const float firstLeft = first.getPosition().x + first.hitbox.offset.x;
    const float firstRight = firstLeft + first.hitbox.width;
    const float secondLeft = second.getPosition().x + second.hitbox.offset.x;
    const float secondRight = secondLeft + second.hitbox.width;
    return std::min(firstRight, secondRight) - std::max(firstLeft, secondLeft);
}

// Stomps are directional contacts.  Choosing the smallest AABB overlap is unreliable at
// either edge of an enemy: there the horizontal overlap is naturally tiny even though the
// player arrived from above.  Use vertical motion and faces instead, while still requiring
// a non-zero horizontal overlap.
bool isStompFromAbove(const Entity& player, const Entity& enemy) {
    if (player.getVelocity().y < -ContactEpsilon) return false;
    if (horizontalOverlap(player, enemy) <= ContactEpsilon) return false;

    const float playerTop = player.getPosition().y + player.hitbox.offset.y;
    const float playerBottom = playerTop + player.hitbox.height;
    const float enemyTop = enemy.getPosition().y + enemy.hitbox.offset.y;
    const float maxPenetration = enemy.hitbox.height * 0.5f + ContactEpsilon;

    // The player must still be predominantly above the enemy.  The half-height bound
    // prevents a lateral collision while falling from being misclassified as a stomp.
    return playerTop <= enemyTop + ContactEpsilon
        && playerBottom >= enemyTop - ContactEpsilon
        && playerBottom <= enemyTop + maxPenetration;
}

bool hasStableTopContact(const Entity& player, const Entity& blocker) {
    const float playerBottom = player.getPosition().y + player.hitbox.offset.y + player.hitbox.height;
    const float blockerTop = blocker.getPosition().y + blocker.hitbox.offset.y;

    // Keep standing support stable when the player is effectively on the block's top
    // face but float rounding leaves a tiny gap or tiny overlap.
    if (player.getVelocity().y < 0.0f) return false;
    if (playerBottom < blockerTop - 2.0f || playerBottom > blockerTop + 4.0f) return false;
    return horizontalOverlap(player, blocker) > ContactEpsilon;
}

// Reclassify a solid-block contact from the mover's own motion. The smaller-overlap
// heuristic (calculateSide) misreads a head bump near a block's corner — thin horizontal
// overlap against a deeper vertical one — as a side hit, which would shove the mover
// past the block. When the mover's head has just crossed the block's underside (or the
// feet just crossed its top) the vertical side is unambiguous, so honour it.
CollisionType solidSideFromMotion(const Entity& mover, const Entity& blocker, CollisionType moverSide) {
    const float blockTop = blocker.getPosition().y + blocker.hitbox.offset.y;
    const float blockHeight = blocker.hitbox.height;
    const float blockBottom = blockTop + blockHeight;
    const float moverTop = mover.getPosition().y + mover.hitbox.offset.y;
    const float moverBottom = moverTop + mover.hitbox.height;

    const float vy = mover.getVelocity().y;

    // Standing support: if the mover is already resting very close to the block's top
    // face, keep treating it as a landing. This matters for entity blocks like coin
    // blocks, where a tiny overlap can otherwise flip the contact to a side hit for one
    // frame and make jump input feel delayed.
    if (vy >= 0.0f && moverBottom >= blockTop - 1.0f && moverBottom <= blockTop + 4.0f) {
        return CollisionType::Bottom;
    }

    // Head bump: the mover is rising and its top face has just crossed the block's
    // underside (blockBottom). The band is wide on the inside because the head moves at
    // full jump speed, so the contact frame can sink ~10-12px into the block; a narrow
    // band misses that frame and edge hits on coin blocks — and a jump centred on the
    // seam of two adjacent blocks — then look more horizontal than vertical and shove
    // the mover sideways.
    if (vy < -ContactEpsilon && horizontalOverlap(mover, blocker) > ContactEpsilon
        && moverTop >= blockBottom - 14.0f - ContactEpsilon
        && moverTop <= blockBottom + 2.0f + ContactEpsilon) {
        return CollisionType::Top;
    }
    // Landing: the feet are inside the block, only just past its top face.
    if (vy > ContactEpsilon && moverBottom > blockTop - ContactEpsilon && moverBottom < blockBottom
        && moverBottom - blockTop < blockHeight * 0.5f) {
        return CollisionType::Bottom;
    }
    return moverSide;
}
}

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
        std::size_t row = TileMap::Rows - 1 - static_cast<std::size_t>(footY / TileMap::TileHeight);
        // Sample the whole foot, not just the center column, so a ledge under half of
        // the entity still counts as ground.
        std::size_t colLeft = static_cast<std::size_t>((leftX + 1.0f) / TileMap::TileWidth);
        std::size_t colCenter = static_cast<std::size_t>(centerX / TileMap::TileWidth);
        std::size_t colRight = static_cast<std::size_t>((rightX - 1.0f) / TileMap::TileWidth);

        bool hit = false;
        if (row < TileMap::Rows) {
            if (colLeft < tileMap->getColumns() && TileMap::isSolidTile(tileMap->getTile(row, colLeft))) hit = true;
            if (colCenter < tileMap->getColumns() && TileMap::isSolidTile(tileMap->getTile(row, colCenter))) hit = true;
            if (colRight < tileMap->getColumns() && TileMap::isSolidTile(tileMap->getTile(row, colRight))) hit = true;
        }
        if (hit) {
            pos.y = (TileMap::Rows - 1 - row) * TileMap::TileHeight - hb.height - hb.offset.y;
            vel.y = 0.0f;
            entity->isGrounded = true;
            entity->onTileCollision(tileMap->getTile(row, colCenter), CollisionType::Bottom);
        }
    }

    // Check upwards
    if (vel.y < 0.0f) {
        std::size_t row = TileMap::Rows - 1 - static_cast<std::size_t>(headY / TileMap::TileHeight);
        // Choose a ceiling from the player's centre lane.  In a narrow `#C#` shaft the
        // side tiles are walls beside the CoinBlock, not ceiling beneath it.  Sampling
        // the whole head turns a sub-pixel lateral drift into a false wall hit and moves
        // the player down before the CoinBlock entity can receive its bump.
        std::size_t colCenter = static_cast<std::size_t>(centerX / TileMap::TileWidth);

        if (row < TileMap::Rows && colCenter < tileMap->getColumns()
            && TileMap::isSolidTile(tileMap->getTile(row, colCenter))) {
            pos.y = (TileMap::Rows - row) * TileMap::TileHeight - hb.offset.y;
            vel.y = 0.0f;
            entity->onTileCollision(tileMap->getTile(row, colCenter), CollisionType::Top);
        }
    }

    // Check left
    if (vel.x < 0.0f) {
        // Recompute after the vertical pass may have moved pos.y. Sample the rows the
        // body actually occupies: footY - 1, not footY, so the ground tile under a
        // standing entity is never mistaken for a wall.
        float headY = pos.y + hb.offset.y;
        float footY = pos.y + hb.offset.y + hb.height;
        std::size_t col = static_cast<std::size_t>(leftX / TileMap::TileWidth);
        std::size_t rowHead = TileMap::Rows - 1 - static_cast<std::size_t>(headY / TileMap::TileHeight);
        std::size_t rowFoot = TileMap::Rows - 1 - static_cast<std::size_t>((footY - 1.0f) / TileMap::TileHeight);

        // Sample both the head and the foot rows, not just the middle of the body.
        std::size_t hitRow = TileMap::Rows;
        if (col < tileMap->getColumns()) {
            if (rowHead < TileMap::Rows && TileMap::isSolidTile(tileMap->getTile(rowHead, col))) hitRow = rowHead;
            else if (rowFoot < TileMap::Rows && TileMap::isSolidTile(tileMap->getTile(rowFoot, col))) hitRow = rowFoot;
        }
        if (hitRow < TileMap::Rows) {
            pos.x = (col + 1) * TileMap::TileWidth - hb.offset.x;
            vel.x = 0.0f;
            entity->onTileCollision(tileMap->getTile(hitRow, col), CollisionType::Left);
        }
    }

    // Check right
    if (vel.x > 0.0f) {
        // Recompute after the vertical pass may have moved pos.y. Sample the rows the
        // body actually occupies: footY - 1, not footY, so the ground tile under a
        // standing entity is never mistaken for a wall.
        float headY = pos.y + hb.offset.y;
        float footY = pos.y + hb.offset.y + hb.height;
        std::size_t col = static_cast<std::size_t>(rightX / TileMap::TileWidth);
        std::size_t rowHead = TileMap::Rows - 1 - static_cast<std::size_t>(headY / TileMap::TileHeight);
        std::size_t rowFoot = TileMap::Rows - 1 - static_cast<std::size_t>((footY - 1.0f) / TileMap::TileHeight);

        // Sample both the head and the foot rows, not just the middle of the body.
        std::size_t hitRow = TileMap::Rows;
        if (col < tileMap->getColumns()) {
            if (rowHead < TileMap::Rows && TileMap::isSolidTile(tileMap->getTile(rowHead, col))) hitRow = rowHead;
            else if (rowFoot < TileMap::Rows && TileMap::isSolidTile(tileMap->getTile(rowFoot, col))) hitRow = rowFoot;
        }
        if (hitRow < TileMap::Rows) {
            pos.x = col * TileMap::TileWidth - hb.width - hb.offset.x;
            vel.x = 0.0f;
            entity->onTileCollision(tileMap->getTile(hitRow, col), CollisionType::Right);
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
            } else {
                const bool aIsPlayer = a->hitbox.layer == CollisionLayer::Player;
                const bool bIsPlayer = b->hitbox.layer == CollisionLayer::Player;
                if (aIsPlayer != bIsPlayer) {
                    Entity* player = aIsPlayer ? a : b;
                    Entity* blocker = aIsPlayer ? b : a;
                    if (blocker->isSolid() && hasStableTopContact(*player, *blocker)) {
                        resolveEntityInteraction(*blocker, *player, CollisionType::Top);
                    }
                }
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
    const bool aIsPlayer = a.hitbox.layer == CollisionLayer::Player;
    const bool bIsPlayer = b.hitbox.layer == CollisionLayer::Player;
    const bool exactlyOnePlayer = aIsPlayer != bIsPlayer;

    // Correct the contact side BEFORE the hooks see it, so both the bump reaction (the
    // block's onCollision) and the push-out agree it was a vertical contact.
    if (exactlyOnePlayer) {
        Entity* player = aIsPlayer ? &a : &b;
        Entity* other = aIsPlayer ? &b : &a;
        if (other->isSolid()) {
            const CollisionType playerSide = aIsPlayer ? sideA : invertCollisionSide(sideA);
            const CollisionType resolved = solidSideFromMotion(*player, *other, playerSide);
            sideA = aIsPlayer ? resolved : invertCollisionSide(resolved);
        } else if (other->hitbox.layer == CollisionLayer::Enemy && isStompFromAbove(*player, *other)) {
            // Apply the directional stomp classification before either collision hook
            // sees the side.  This keeps both halves of the enemy equally stompable.
            sideA = aIsPlayer ? CollisionType::Bottom : CollisionType::Top;
        }
    }

    // Notify both entities of the collision: each one reacts through its own hooks
    // (e.g. CoinBlock collects its coin when bumped from below).
    a.onCollision(b, sideA);
    const CollisionType sideB = invertCollisionSide(sideA);
    b.onCollision(a, sideB);

    if (a.hitbox.layer == CollisionLayer::Projectile || b.hitbox.layer == CollisionLayer::Projectile) {
        // Projectiles settle their own outcome in Projectile::onCollision, which already ran
        // above — they check who fired them, so the same class serves Bowser and Mario.
        // Deliberately no stomp branch: landing on a hammer must not bounce the player.
        return;
    }

    // Enemy resolution only concerns the player (two enemies pass through each other).
    if (exactlyOnePlayer) {
        Entity* player = aIsPlayer ? &a : &b;
        Entity* other = aIsPlayer ? &b : &a;
        const CollisionType playerSide = aIsPlayer ? sideA : sideB;
        if (other->hitbox.layer == CollisionLayer::Enemy) {
            if (auto* hero = dynamic_cast<Player*>(player); hero && hero->isStar()) {
                // Star power: any contact defeats any enemy — no stomp requirement, so
                // Spiny's spikes and Bowser fall to it too.
                other->onHit(*player);
            } else if (playerSide == CollisionType::Bottom && other->isStompable()) {
                // Player landed on top of the enemy: squash it and bounce.
                other->onStomped(*player);
                player->setVelocity({player->getVelocity().x, -350.0f}); // Bounce force
            } else {
                // Player hit the enemy from the side or below — or landed on something that
                // cannot be stomped at all (Spiny's spikes, Bowser): take damage.
                player->takeDamage(other->getDamageValue());
            }
            return;
        }
    }

    // Solids stop whoever touches them — the player, an enemy, or a freshly-spawned
    // mushroom. Only the player used to be pushed out; without this, an item fell straight
    // through the block instead of landing on its top and walking off the edge.
    if (a.isSolid() != b.isSolid()) {
        Entity* mover = a.isSolid() ? &b : &a;
        Entity* blocker = a.isSolid() ? &a : &b;
        const CollisionType moverSide = a.isSolid() ? invertCollisionSide(sideA) : sideA;
        const CollisionType resolved = solidSideFromMotion(*mover, *blocker, moverSide);

        pushOutOfBlock(*mover, *blocker, resolved);
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
