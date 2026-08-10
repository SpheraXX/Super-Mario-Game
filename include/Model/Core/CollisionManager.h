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
    void resolveEntityInteraction(Entity& a, Entity& b, CollisionType side);

private:
    TileMap* tileMap;

    // Tile resolution only applies to Characters: static world objects never move, so
    // the pass has nothing to resolve for them (and they no longer expose a velocity).
    void processTileCollisions(Character& entity, float deltaTime);
    void processEntityCollisions(std::vector<Entity*>& entities);

    // Push the mover out of a solid blocker along the collision axis. Works on any
    // character-vs-blocker pair: the responder (e.g. CoinBlock) reacts through its
    // onBlockHit hook, dispatched by type at the call site.
    void pushOutOfBlock(Character& mover, const Entity& blocker, CollisionType moverSide);
};

}

#endif
