#ifndef MODEL_COLLISIONMANAGER_H
#define MODEL_COLLISIONMANAGER_H

#include <vector>
#include "Model/CollisionResult.h"

namespace model {

class Entity;
class TileMap;
class Block;
class Player;

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

    // Push the player out of a solid block and handle the bump interaction
    // (e.g. collecting a coin when the block is hit from below).
    void pushOutOfBlock(Player& player, Block& block, CollisionType playerSide);
    void bumpBlock(Block& block);
};

}

#endif
