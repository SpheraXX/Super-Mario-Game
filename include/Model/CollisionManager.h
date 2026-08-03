#ifndef MODEL_COLLISIONMANAGER_H
#define MODEL_COLLISIONMANAGER_H

#include <vector>
#include "Model/CollisionResult.h"

namespace model {

class Entity;
class TileMap;

class CollisionManager {
public:
    CollisionManager(TileMap* tileMap);

    void update(std::vector<Entity*>& entities, float deltaTime);

    CollisionType calculateSide(const Entity& a, const Entity& b) const;
    void resolveEntityInteraction(Entity& a, Entity& b, CollisionType side);

private:
    TileMap* tileMap;

    void processTileCollisions(Entity* entity, float deltaTime);
    void processEntityCollisions(std::vector<Entity*>& entities);

    // Push the mover out of a solid blocker along the collision axis. Works on any
    // entity pair: the responder (e.g. CoinBlock) reacts through its onCollision hook.
    void pushOutOfBlock(Entity& mover, const Entity& blocker, CollisionType moverSide);
};

}

#endif
