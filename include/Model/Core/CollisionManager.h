#ifndef MODEL_COLLISIONMANAGER_H
#define MODEL_COLLISIONMANAGER_H

#include <vector>
#include "Model/Core/CollisionResult.h"

namespace model {

class Entity;
class Character;
class TileMap;

class CollisionManager {
public:
    CollisionManager(TileMap* tileMap);

    void update(std::vector<Entity*>& entities, float deltaTime);

    CollisionType calculateSide(const Entity& a, const Entity& b) const;
    // Resolves one entity pair. Returns true when the pair was a player bump that
    // actually dispatched a BlockHitEvent to a Block.
    bool resolveEntityInteraction(Entity& a, Entity& b, CollisionType side);

private:
    TileMap* tileMap;

    // Tile resolution only applies to Characters: static world objects never move, so
    // the pass has nothing to resolve for them (and they no longer expose a velocity).
    void processTileCollisions(Character& entity, float deltaTime);
    void processEntityCollisions(std::vector<Entity*>& entities);

    // Knock out every enemy standing on the bumped block's top face (the classic
    // "headbutt kills what stands on the block"). Returns true when any enemy fell.
    // Callers pass the entities list so the scan sees exactly the active set.
    bool defeatEnemiesAbove(const Entity& block, Entity& player,
                            const std::vector<Entity*>& entities);

    // Push the mover out of a solid blocker along the collision axis. Works on any
    // character-vs-blocker pair: the responder (e.g. CoinBlock) reacts through its
    // onBlockHit hook, dispatched by type at the call site.
    void pushOutOfBlock(Character& mover, const Entity& blocker, CollisionType moverSide);
};

}

#endif
