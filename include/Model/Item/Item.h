#ifndef MODEL_ITEM_H
#define MODEL_ITEM_H

#include "Model/Character.h"

namespace model {

// Base class for collectibles (Mushroom, FireFlower, later Star...). Derives from
// Character so a walking item reuses gravity + velocity integration for free.
//
// An item does NOT need special-casing in CollisionManager: the manager already fires
// Entity::onCollision for every pair it finds, so an item collects itself through that
// hook the moment it touches the player (and since it is not solid, the player is never
// pushed by it).
class Item : public Character {
public:
    Item(Vector2 position, Vector2 size);

    void onCollision(Entity& other, CollisionType side) override;

    // Apply the item's effect to the collector (the player). Subclasses override.
    virtual void onCollect(Entity& collector);

    // Items are passable: walking into one collects it instead of being blocked.
    bool isSolid() const override { return false; }
};

}

#endif
