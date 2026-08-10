#include "Model/Core/CollisionManager.h"
#include "Model/Core/BlockHitEvent.h"
#include "Model/Entity.h"
#include "Model/Map/TileMap.h"

#include <algorithm>
#include <cmath>

namespace model {

namespace {
// Side-detection bias in the player's favour: a collision counts as horizontal only when
// the horizontal overlap is clearly dominant. Near-ties resolve as vertical, i.e. as a
// stomp — so an enemy that clips the player's feet for one frame cannot flip into a
// side hit (and damage) frame-to-frame.
constexpr float StompBias = 0.75f;

// A tile the player can stand on. The static ground is 'G', and block cells ('C'/'B'/'#')
// always hold a static solid entity, so landing on them is equivalent. Grounding on these
// cells keeps isGrounded stable on top of blocks (gravity + animation never flap).
bool isGroundTile(char symbol) {
    return symbol == 'G' || symbol == 'C' || symbol == 'B' || symbol == '#';
}

// Minimum upward speed (world units/s) for a top-face block contact to count as a bump.
// A head that only grazes a block at the top of a jump is physically stopped (the push-out
// below always applies) but is too weak to open it. Mario's full-hold jump touches a block
// 5 tiles above his feet with only ~162px/s, while a real 4-tile bump lands at ~360px/s —
// this gate is what keeps the "barely 4 blocks" jump from opening blocks 5 tiles up.
constexpr float MinBumpSpeed = 200.0f;

// Landing tolerance for the downward snap. The feet only snap onto a tile when they crossed
// its top edge this frame (foot above the top at the start, on/below it at the end). A
// player brushing a block's side mid-jump has his foot inside the tile without ever having
// been above it — snapping him up there is the "rounded up, standing on the block" bug. The
// epsilon absorbs float noise so the resting-on-ground case (prevFoot == tile top, vel.y = 0)
// keeps grounding every frame.
constexpr float LandingEpsilon = 1.0f;

// Rest tolerance for standing on a solid ENTITY top (pipe caps). The strict AABB test
// reports no overlap at exact contact (feet == block top), so without this the player
// would drop one frame and snap back the next — the "glitch dance" on pipes. A solid top
// within this epsilon of the feet counts as a standing contact, mirroring LandingEpsilon.
constexpr float TopRestEpsilon = 2.0f;

// True when one of the pair is the player resting on the other's solid top: feet within
// TopRestEpsilon of the top (or a sub-pixel overlap), a real horizontal footprint, and no
// upward flight. The entity-pass equivalent of the tile pass's LandingEpsilon landing.
bool restingOnSolidTop(const Entity& a, const Entity& b) {
    const Entity* player = nullptr;
    const Entity* solid = nullptr;
    if (a.hitbox.layer == CollisionLayer::Player) {
        player = &a;
        solid = &b;
    } else if (b.hitbox.layer == CollisionLayer::Player) {
        player = &b;
        solid = &a;
    }
    if (!player || !solid->isSolid() || player->getVelocity().y < 0.0f) return false;

    const float feetY = player->getPosition().y + player->hitbox.offset.y + player->hitbox.height;
    const float topY = solid->getPosition().y + solid->hitbox.offset.y;
    if (std::fabs(feetY - topY) > TopRestEpsilon) return false;

    const float myLeft = player->getPosition().x + player->hitbox.offset.x;
    const float myRight = myLeft + player->hitbox.width;
    const float otherLeft = solid->getPosition().x + solid->hitbox.offset.x;
    const float otherRight = otherLeft + solid->hitbox.width;
    return myLeft < otherRight && myRight > otherLeft;
}
}

CollisionManager::CollisionManager(TileMap* tileMap) : tileMap(tileMap) {}

void CollisionManager::update(std::vector<Entity*>& entities, float deltaTime) {
    // Pass 1: Entity vs TileMap. Dying bodies pop through the world, so they skip it.
    for (auto* entity : entities) {
        if (!entity || !entity->isActive || entity->isDying()) continue;
        entity->isGrounded = false;
        processTileCollisions(entity, deltaTime);
    }

    // Pass 2: Entity vs Entity
    processEntityCollisions(entities);
}

void CollisionManager::processTileCollisions(Entity* entity, float deltaTime) {
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

    // Check downwards. The foot spans several columns when running along block edges, so
    // probe the left, centre and right edges of the foot — a single centre column let the
    // player drop through corners and made isGrounded flap on platforms. Block cells act
    // as ground here (see isGroundTile); the entity pass still does the real push-out.
    //
    // A tile only counts as landing when the foot was above its top at the START of this
    // frame (prevFootY). If the foot is already inside the tile — a block hit from the side
    // mid-jump — snapping up to the top would teleport the player onto the block; those
    // contacts are resolved by the horizontal checks and the entity pass instead.
    if (vel.y >= 0.0f) {
        const std::size_t row = TileMap::Rows - 1 - static_cast<std::size_t>(footY / TileMap::TileHeight);

        if (row < TileMap::Rows) {
            const float tileTop = (TileMap::Rows - 1 - row) * TileMap::TileHeight;
            const float prevFootY = footY - vel.y * deltaTime;
            const float footProbes[3] = {leftX, centerX, rightX};
            for (const float probeX : footProbes) {
                const std::size_t col = static_cast<std::size_t>(probeX / TileMap::TileWidth);

                if (col < tileMap->getColumns() && isGroundTile(tileMap->getTile(row, col))) {
                    if (prevFootY > tileTop + LandingEpsilon) break;
                    pos.y = tileTop - hb.height - hb.offset.y;
                    vel.y = 0.0f;
                    entity->isGrounded = true;
                    entity->onTileCollision(tileMap->getTile(row, col), CollisionType::Bottom);
                    break;
                }
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
        if (!a || !a->isActive || a->isDying()) continue;

        // The player can overlap several solid blocks in one frame (a head pressed into a
        // row of bricks). Only the deepest contact is resolved — ties go to the block that
        // contains the player's centre — so a stale overlap with a neighbour can never
        // shove the player sideways, and exactly one block gets bumped (the one above and
        // in the middle of him, not the leftmost in spawn order).
        Entity* bestBlock = nullptr;
        float bestScore = 0.0f;
        CollisionType bestSide = CollisionType::None;

        for (std::size_t j = i + 1; j < entities.size(); ++j) {
            Entity* b = entities[j];
            if (!b || !b->isActive || b->isDying()) continue;

            // At exact contact (feet == block top) the strict AABB reports no overlap, so
            // the pair is only kept when the player is resting on a solid top within the
            // rest epsilon — see restingOnSolidTop.
            if (!a->hitbox.intersects(b->hitbox, a->getPosition(), b->getPosition()) &&
                !restingOnSolidTop(*a, *b)) {
                continue;
            }

            // Trigger pass: trigger hitboxes never block or push; they only fire the
            // onTriggerEnter hook when the other entity is the player (e.g. FlagPole).
            if (a->hitbox.isTrigger || b->hitbox.isTrigger) {
                Entity* trigger = a->hitbox.isTrigger ? a : b;
                Entity* other = (trigger == a) ? b : a;
                if (other->hitbox.layer == CollisionLayer::Player) {
                    trigger->onTriggerEnter(*other);
                }
                continue;
            }

            const CollisionType sideA = calculateSide(*a, *b);
            const bool playerInvolved =
                a->hitbox.layer == CollisionLayer::Player || b->hitbox.layer == CollisionLayer::Player;
            const bool blockInvolved = a->isSolid() || b->isSolid();

            if (playerInvolved && blockInvolved) {
                // Defer: remember the deepest candidate instead of resolving right away.
                const Vector2 overlap =
                    a->hitbox.getOverlap(b->hitbox, a->getPosition(), b->getPosition());
                const float score = std::fabs(overlap.y);
                bool candidate = false;
                if (!bestBlock) {
                    candidate = true;
                } else if (score > bestScore + 0.001f) {
                    candidate = true;
                } else if (std::fabs(score - bestScore) <= 0.001f) {
                    // Tie: prefer the block containing the player's centre.
                    const Entity* player = a->hitbox.layer == CollisionLayer::Player ? a : b;
                    const Entity* other = (player == a) ? b : a;
                    const float playerCenterX =
                        player->getPosition().x + player->hitbox.offset.x + player->hitbox.width / 2.0f;
                    const float bestLeft = bestBlock->getPosition().x + bestBlock->hitbox.offset.x;
                    const float newLeft = other->getPosition().x + other->hitbox.offset.x;
                    const bool bestContains =
                        playerCenterX >= bestLeft && playerCenterX < bestLeft + bestBlock->hitbox.width;
                    const bool newContains =
                        playerCenterX >= newLeft && playerCenterX < newLeft + other->hitbox.width;
                    candidate = newContains && !bestContains;
                }
                if (candidate) {
                    bestBlock = b;
                    bestScore = score;
                    bestSide = sideA;
                }
            } else {
                resolveEntityInteraction(*a, *b, sideA);
            }
        }

        // Resolve the single best player-vs-block contact. Neighbouring blocks that only
        // overlapped because of the pre-push positions are intentionally skipped.
        if (bestBlock) {
            resolveEntityInteraction(*a, *bestBlock, bestSide);
        }
    }
}

CollisionType CollisionManager::calculateSide(const Entity& a, const Entity& b) const {
    Vector2 overlap = a.hitbox.getOverlap(b.hitbox, a.getPosition(), b.getPosition());

    // Compare the penetration MAGNITUDES, not the signed values. getOverlap returns
    // negative penetrations when two same-sized boxes are exactly aligned (e.g. a player
    // perfectly under a block: overlap.x = -blockWidth), which previously made the signed
    // comparison pick the horizontal axis and shove the player a full block sideways.
    const float overlapX = std::fabs(overlap.x);
    const float overlapY = std::fabs(overlap.y);

    // The smaller overlap axis dictates the collision side; near-ties favour the vertical
    // (stomp) reading so the player is never punished for a sub-pixel clip.
    if (overlapX < overlapY * StompBias) {
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

    if (other->hitbox.layer == CollisionLayer::Enemy) {
        if (playerSide == CollisionType::Bottom) {
            // Player landed on top of the enemy: squash it and bounce.
            other->onStomped(*player);
            player->setVelocity({player->getVelocity().x, -350.0f}); // Bounce force
        } else {
            // Player hit the enemy from the side or from below: take damage.
            player->takeDamage(other->getDamageValue());
        }
    } else if (other->isSolid()) {
        // Solid blocks stop the player (push-out). A bump from below also dispatches the
        // block-hit event — but only when the head is moving into the block fast enough
        // (a graze at the top of the arc is too weak to trigger it).
        const float upwardSpeed = std::max(0.0f, -player->getVelocity().y);
        CollisionType resolvedSide = playerSide;
        if (playerSide == CollisionType::Bottom && !other->isLandable()) {
            // A top that must not be stood on (e.g. the goal castle): turn the landing
            // into a horizontal slide so the player falls back down the nearer side
            // instead of resting on the roof.
            const float playerCenterX = player->getPosition().x + player->hitbox.offset.x
                                        + player->hitbox.width / 2.0f;
            const float blockerCenterX = other->getPosition().x + other->hitbox.offset.x
                                         + other->hitbox.width / 2.0f;
            resolvedSide = (playerCenterX < blockerCenterX) ? CollisionType::Right
                                                            : CollisionType::Left;
        }
        pushOutOfBlock(*player, *other, resolvedSide);
        if (playerSide == CollisionType::Top && upwardSpeed >= MinBumpSpeed) {
            other->onBlockHit(BlockHitEvent{*player, playerSide, upwardSpeed});
        }
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
            // Only kill the velocity component that points INTO the block: a player
            // brushing a block while moving the other way keeps his momentum (fixes the
            // "jump at high speed loses all momentum" chain where a side graze and a
            // stale vertical overlap each zeroed one axis).
            if (mover.getVelocity().y > 0.0f) mover.setVelocity({mover.getVelocity().x, 0.0f});
            mover.isGrounded = true;
            break;
        case CollisionType::Top:
            newPos.y = blockerPos.y + blockerBox.offset.y + blockerBox.height - mover.hitbox.offset.y;
            if (mover.getVelocity().y < 0.0f) mover.setVelocity({mover.getVelocity().x, 0.0f});
            break;
        case CollisionType::Right:
            // The mover's right face is in contact: it sits to the LEFT of the blocker,
            // so its right edge is pinned to the blocker's left edge.
            newPos.x = blockerPos.x + blockerBox.offset.x - mover.hitbox.offset.x - mover.hitbox.width;
            if (mover.getVelocity().x > 0.0f) mover.setVelocity({0.0f, mover.getVelocity().y});
            break;
        case CollisionType::Left:
            // The mover's left face is in contact: it sits to the RIGHT of the blocker.
            newPos.x = blockerPos.x + blockerBox.offset.x + blockerBox.width - mover.hitbox.offset.x;
            if (mover.getVelocity().x < 0.0f) mover.setVelocity({0.0f, mover.getVelocity().y});
            break;
        default:
            return;
    }
    mover.setPosition(newPos);
}

}
