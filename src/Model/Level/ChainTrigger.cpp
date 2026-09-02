#include "Model/Level/ChainTrigger.h"

#include "Model/Core/Hitbox.h"
#include "Model/Core/World.h"
#include "Model/Map/TileMap.h"

namespace model {

ChainTrigger::ChainTrigger(Vector2 position, Vector2 size)
    : Entity(position, size) {
    hitbox = Hitbox({0.0f, 0.0f}, size.x, size.y, /* isTrigger */ true,
                    CollisionLayer::Trigger);
}

void ChainTrigger::onTriggerEnter(Entity& other) {
    if (triggered || !world) return;
    if (other.hitbox.layer != CollisionLayer::Player) return;

    triggered = true;
    // Cutting the chain is a change to the MAP, not to a set of entities, and that is the
    // whole point: the bridge is terrain, so enemies collide with it (the entity pass only
    // ever resolves the player against solid entities — a bridge built from entities would
    // have dropped Bowser through it on frame one). Erasing the tiles is therefore what
    // makes "anything standing on it falls off" fall out for free, for the player and for
    // every enemy alike, with no per-entity bookkeeping.
    world->removeTilesOfType(TileMap::ChainSymbol);
}

}
