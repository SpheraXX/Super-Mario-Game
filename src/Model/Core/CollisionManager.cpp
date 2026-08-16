#include "Model/Core/CollisionManager.h"
#include "Model/Block/Block.h"
#include "Model/Core/BlockHitEvent.h"
#include "Model/Character.h"
#include "Model/Enemy/Enemy.h"
#include "Model/Entity.h"
#include "Model/Item/Item.h"
#include "Model/Map/TileMap.h"
#include "Model/Player/Player.h"

#include <algorithm>
#include <cmath>

namespace model {

namespace {
// Side-detection bias in the player's favour: a collision counts as horizontal only when
// the horizontal overlap is clearly dominant. Near-ties resolve as vertical, i.e. as a
// stomp — so an enemy that clips the player's feet for one frame cannot flip into a
// side hit (and damage) frame-to-frame.
constexpr float StompBias = 0.75f;

// A tile the player can stand on. The static ground is 'G', the stair block is unbreakable
// terrain, and block cells ('C'/'B'/'#') always hold a static solid entity, so landing on
// any of them is equivalent. Grounding on these cells keeps isGrounded stable on top of
// blocks (gravity + animation never flap).
//
// This list decides LANDING (the feet), and is deliberately a different set from
// TileMap::isSolidTile, which decides what blocks movement at all. Anything walkable has to
// appear in BOTH: a symbol that is solid but not ground stops the player sideways and
// overhead yet never supports him, so he drops straight through the top of it — which is
// exactly what the stair block did before it was added here.
bool isGroundTile(char symbol) {
    return symbol == 'G' || symbol == model::TileMap::StairSymbol
        || symbol == 'C' || symbol == 'B' || symbol == '#'
        || model::TileMap::isPipeSymbol(symbol)
        || model::TileMap::isCastleSymbol(symbol);
}

// Minimum upward speed (world units/s) for a top-face block contact to count as a bump.
// A head that only grazes a block at the top of a jump is physically stopped (the push-out
// below always applies) but is too weak to open it. Mario's full-hold jump touches a block
// 5 tiles above his feet with only ~162px/s, while a real 4-tile bump lands at ~360px/s —
// this gate is what keeps the "barely 4 blocks" jump from opening blocks 5 tiles up.
constexpr float MinBumpSpeed = 100.0f;

// Landing tolerance for the downward snap. The feet only snap onto a tile when they crossed
// its top edge this frame (foot above the top at the start, on/below it at the end). A
// player brushing a block's side mid-jump has his foot inside the tile without ever having
// been above it — snapping him up there is the "rounded up, standing on the block" bug. The
// epsilon absorbs float noise so the resting-on-ground case (prevFoot == tile top, vel.y = 0)
// keeps grounding every frame.
constexpr float LandingEpsilon = 0.5f;

// Rest tolerance for standing on a solid ENTITY top (pipe caps). The strict AABB test
// reports no overlap at exact contact (feet == block top), so without this the player
// would drop one frame and snap back the next — the "glitch dance" on pipes. A solid top
// within this epsilon of the feet counts as a standing contact, mirroring LandingEpsilon.
constexpr float TopRestEpsilon = 1.0f;

// True when one of the pair is the player resting on the other's solid top: feet within
// TopRestEpsilon of the top (or a sub-pixel overlap), a real horizontal footprint, and no
// upward flight. The entity-pass equivalent of the tile pass's LandingEpsilon landing.
//
// The collision layers are the type contract here: the Player layer is only ever carried
// by Player (a Character), so the static_cast is safe by construction.
bool restingOnSolidTop(const Entity& a, const Entity& b) {
    const Character* player = nullptr;
    const Entity* solid = nullptr;
    if (a.hitbox.layer == CollisionLayer::Player) {
        player = static_cast<const Character*>(&a);
        solid = &b;
    } else if (b.hitbox.layer == CollisionLayer::Player) {
        player = static_cast<const Character*>(&b);
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
    // Static world objects (pipes, blocks, the flagpole) are skipped entirely: they
    // never move, so tile resolution is a no-op for them anyway.
    for (auto* entity : entities) {
        if (!entity || !entity->isActive) continue;
        auto* character = dynamic_cast<Character*>(entity);
        if (!character || character->isDying()) continue;
        character->isGrounded = false;
        // Trigger bodies (a popped coin, decorative floats) take no part in collision at
        // all: the tile pass must not resolve them either, or a coin falling back onto
        // its block would be snapped onto the block's top and stuck there instead of
        // returning to its spawn height and disappearing.
        if (character->hitbox.isTrigger) continue;
        processTileCollisions(*character, deltaTime);
    }

    // Pass 2: Entity vs Entity
    processEntityCollisions(entities);
}

void CollisionManager::processTileCollisions(Character& entity, float deltaTime) {
    if (!tileMap) return;

    Vector2 pos = entity.getPosition();
    Hitbox& hb = entity.hitbox;
    Vector2 vel = entity.getVelocity();
    
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
    //
    // Crossing the top edge is not enough on its own: a body FALLING ALONG a tall solid
    // column (a pipe shaft, a tall 'G' pillar) crosses the top edge of every shaft cell as
    // its feet sink past them, and snap-landing on each one would glue the player to the
    // wall at whatever height his feet happen to be ("standing on a wall"). A cell whose
    // top is not exposed — static solid terrain directly above it, so the body could never
    // have dropped onto it from above — is a side clip, not a landing: the horizontal
    // checks below push the body off the face and the fall continues.
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
                    // The feet crossed this cell's top this frame; only snap when the top
                    // is exposed (nothing solid directly above it in the static map). Grid
                    // rows run bottom-first (row 0 is the deepest row — see tileOrigin), so
                    // the cell one tile ABOVE here is row+1. The cell above is checked in
                    // static terrain terms, so a block entity 'C' above ground does NOT
                    // hide the ground's top — that is the standing surface under a block row.
                    //
                    // A covered probe is SKIPPED, not fatal: when the foot straddles a
                    // wall's base column, the probe on the wall cell has a covered top
                    // (the shaft above it) while the rest of the foot rests on open ground.
                    // Bailing out of the loop on that first probe left the ground unevaluated
                    // and the body sank through it — standing still against the wall, no
                    // horizontal push ever fired. The uncovered probes still land the body.
                    const bool topExposed = row + 1 >= TileMap::Rows ||
                        !TileMap::isSolidTile(tileMap->getTile(row + 1, col));
                    if (!topExposed) continue;
                    pos.y = tileTop - hb.height - hb.offset.y;
                    vel.y = 0.0f;
                    entity.isGrounded = true;
                    entity.onTileCollision(tileMap->getTile(row, col), CollisionType::Bottom);
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
                entity.onTileCollision(tileMap->getTile(row, col), CollisionType::Top);
            }
        }
    }

    // The vertical checks above can move pos.y — landing snaps the body up out of the tile it
    // had sunk into during the frame. Recompute the vertical extents from the *updated* pos.y
    // before probing sideways.
    //
    // Reading the pre-snap values here is what made the player jerk backwards in mid-air: a
    // body falling fast has its foot inside the ground row by the time this runs, so the
    // stale `footY - 1` probe sampled the floor, the side check read that floor as a wall,
    // and pos.x was snapped to the tile boundary the player had already passed. It only
    // showed up while airborne, because a body already resting on the ground is never
    // snapped and its stale extents are still correct.
    headY = pos.y + hb.offset.y;
    footY = pos.y + hb.offset.y + hb.height;
    centerY = pos.y + hb.offset.y + hb.height / 2.0f;

    // Horizontal checks probe the head, the centre AND the feet, for the same reason the
    // downward check probes three columns: a single centre sample is blind to anything that
    // only covers part of the body. Big Mario is two tiles tall, so a centre-only probe let
    // him walk into the bottom cell of a pipe (the shaft) whenever his middle happened to
    // line up with the empty row above it. The topmost/bottom samples are pulled a pixel
    // inside the box so a body resting exactly on a floor does not read the floor itself as
    // a wall.
    const float sideProbes[3] = {headY + 1.0f, centerY, footY - 1.0f};

    // Check left
    if (vel.x < 0.0f) {
        const std::size_t col = static_cast<std::size_t>(leftX / TileMap::TileWidth);
        if (col < tileMap->getColumns()) {
            for (const float probeY : sideProbes) {
                const std::size_t row =
                    TileMap::Rows - 1 - static_cast<std::size_t>(probeY / TileMap::TileHeight);
                if (row < TileMap::Rows && TileMap::isSolidTile(tileMap->getTile(row, col))) {
                    pos.x = (col + 1) * TileMap::TileWidth - hb.offset.x;
                    vel.x = 0.0f;
                    entity.onTileCollision(tileMap->getTile(row, col), CollisionType::Left);
                    break;
                }
            }
        }
    }

    // Check right
    if (vel.x > 0.0f) {
        const std::size_t col = static_cast<std::size_t>(rightX / TileMap::TileWidth);
        if (col < tileMap->getColumns()) {
            for (const float probeY : sideProbes) {
                const std::size_t row =
                    TileMap::Rows - 1 - static_cast<std::size_t>(probeY / TileMap::TileHeight);
                if (row < TileMap::Rows && TileMap::isSolidTile(tileMap->getTile(row, col))) {
                    pos.x = col * TileMap::TileWidth - hb.width - hb.offset.x;
                    vel.x = 0.0f;
                    entity.onTileCollision(tileMap->getTile(row, col), CollisionType::Right);
                    break;
                }
            }
        }
    }

    entity.setPosition(pos);
    entity.setVelocity(vel);
}

void CollisionManager::processEntityCollisions(std::vector<Entity*>& entities) {
    for (std::size_t i = 0; i < entities.size(); ++i) {
        Entity* a = entities[i];
        if (!a || !a->isActive) continue;
        // Dying bodies ignore interaction until the level removes them. isDying() is a
        // virtual on Entity that static world objects answer false to, so this pass needs
        // no downcast — which matters here: it runs once per entity pair, every frame.
        if (a->isDying()) continue;

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
            if (!b || !b->isActive) continue;
            if (b->isDying()) continue;

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
            // The pair is unordered — either end of it can be the block — so pick the
            // solid one for the bump scan; the other is the player who bumped.
            Entity* const player = a->hitbox.layer == CollisionLayer::Player ? a : bestBlock;
            Entity* const block = (player == a) ? bestBlock : a;
            // A real bump also reacts with whatever stands on the block's top face:
            // enemies are flip-killed, resting mushrooms turn around. The block reacts
            // first, then the scan runs over the same active entity list.
            if (resolveEntityInteraction(*a, *bestBlock, bestSide)) {
                affectEntitiesAbove(*block, *player, entities);
            }
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

bool CollisionManager::resolveEntityInteraction(Entity& a, Entity& b, CollisionType sideA) {
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
    // two entities is the player; the other decides how the pair interacts. The layers
    // are the type contract for the casts below: the Player layer is only ever carried
    // by Player (a Character) and the Enemy layer only by Enemy subclasses.
    const bool aIsPlayer = a.hitbox.layer == CollisionLayer::Player;
    const bool bIsPlayer = b.hitbox.layer == CollisionLayer::Player;
    if (aIsPlayer == bIsPlayer) return false;

    Entity* player = aIsPlayer ? &a : &b;
    Entity* other = aIsPlayer ? &b : &a;
    Character& playerCharacter = static_cast<Character&>(*player);
    const CollisionType playerSide = (player == &a) ? sideA : sideB;

    // Items are structurally immune to every damage path: a starred player's side hit, a
    // bump, a stomp-side graze — none of them may dispatch an Item. Collection does not
    // flow through this routing anyway: the onCollision hooks above already delivered the
    // contact (Item::onCollision decides when a collect actually happens), so returning
    // here keeps the pair resolution a no-op while items stay collectible.
    if (other->hitbox.layer == CollisionLayer::Item) return false;

    if (other->hitbox.layer == CollisionLayer::Enemy) {
        Enemy& enemy = static_cast<Enemy&>(*other);
        // The Player layer is only ever carried by Player, so this cast is safe by the
        // same layer contract as the Character cast above.
        Player& hero = static_cast<Player&>(playerCharacter);
        // Star power overrides every stomp rule: contact defeats any enemy — no stomp
        // requirement, so Spiny's spikes and Bowser fall to it too. The stomp lockout is
        // skipped on purpose: a star hit is a defeat, not a pass-through-while-dying case.
        if (hero.isStar()) {
            enemy.onHit(*player);
            return false;
        }
        // Just stomped, or already squished: the player is falling on past it (or standing
        // in the shell it left behind), so the pair does not interact at all. Holding the
        // lockout open for as long as they remain overlapped is what stops it expiring
        // while he is still inside the enemy — see Enemy::acceptsPlayerContact.
        if (!enemy.acceptsPlayerContact()) {
            enemy.holdStompLockout();
            return false;
        }
        if (playerSide == CollisionType::Bottom && enemy.isStompable()) {
            // Player landed on top of a stompable enemy: squash it and bounce. The bounce
            // is a rebound off the impact speed — a per-character fraction of the speed he
            // fell at minus a friction-like constant (see Player::getStompBounceRatio) —
            // not a fixed kick: a hard fall throws him right back up, a slow drop is merely
            // absorbed. Horizontal momentum is kept.
            enemy.stompedBy(*player);
            const float fallSpeed = std::max(0.0f, playerCharacter.getVelocity().y);
            const float bounceUp = std::max(0.0f,
                hero.getStompBounceRatio() * fallSpeed - hero.getStompBounceConstant());
            playerCharacter.setVelocity({playerCharacter.getVelocity().x, -bounceUp});
        } else {
            // Player hit the enemy from the side or from below — or landed on something
            // that cannot be stomped at all (Spiny's spikes, Bowser): take damage.
            playerCharacter.takeDamage(enemy.getDamageValue());
        }
        return false;
    } else if (other->isSolid()) {
        // Solid blocks stop the player (push-out). A bump from below also dispatches the
        // block-hit event — but only when the head is moving into the block fast enough
        // (a graze at the top of the arc is too weak to trigger it), and only when the
        // blocker is actually a Block: a solid pipe never reacts to a bump.
        const float upwardSpeed = std::max(0.0f, -playerCharacter.getVelocity().y);
        pushOutOfBlock(playerCharacter, *other, playerSide);
        if (playerSide == CollisionType::Top && upwardSpeed >= MinBumpSpeed) {
            if (auto* block = dynamic_cast<Block*>(other)) {
                // The block reports whether it actually reacted: a spent ? block returns
                // false, so its bump counts for nothing (no reaction on its top face).
                return block->onBlockHit(BlockHitEvent{*player, playerSide, upwardSpeed});
            }
        }
        return false;
    }
    return false;
}

bool CollisionManager::affectEntitiesAbove(const Entity& block, Entity& player,
                                           const std::vector<Entity*>& entities) {
    // A real bump reacts with everything standing on the block's top face: enemies take
    // the classic headbutt flip-kill, and a Mushroom resting there turns around. Only
    // feet actually resting on the top within the standing epsilon qualify — an entity
    // brushing the block's side or falling past it is untouched. The collision layers
    // are the type contract for the casts, as in resolveEntityInteraction.
    const float topY = block.getPosition().y + block.hitbox.offset.y;
    const float blockLeft = block.getPosition().x + block.hitbox.offset.x;
    const float blockRight = blockLeft + block.hitbox.width;
    bool reactedAny = false;

    for (Entity* entity : entities) {
        if (!entity || !entity->isActive || entity->isDying()) continue;
        if (entity->hitbox.layer != CollisionLayer::Enemy
            && entity->hitbox.layer != CollisionLayer::Item) continue;

        const float feetY = entity->getPosition().y + entity->hitbox.offset.y
                            + entity->hitbox.height;
        if (std::fabs(feetY - topY) > TopRestEpsilon) continue;

        const float myLeft = entity->getPosition().x + entity->hitbox.offset.x;
        const float myRight = myLeft + entity->hitbox.width;
        if (myRight <= blockLeft || myLeft >= blockRight) continue;

        if (entity->hitbox.layer == CollisionLayer::Enemy) {
            // The same one-shot flip-and-fall defeat a spinning shell deals out; no stomp
            // lockout and no bounce — the bump already spent the player's upward motion.
            static_cast<Enemy&>(*entity).onHit(player);
            reactedAny = true;
        } else {
            static_cast<Item&>(*entity).onBlockHitFromBelow();
            reactedAny = true;
        }
    }
    return reactedAny;
}

void CollisionManager::pushOutOfBlock(Character& mover, const Entity& blocker, CollisionType moverSide) {
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
