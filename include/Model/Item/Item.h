#ifndef MODEL_ITEM_H
#define MODEL_ITEM_H

#include "Model/Character.h"
#include "Model/Core/VerticalSlide.h"

#include <memory>

namespace model {

// Base class for collectibles (Mushroom, FireFlower, Star...). Derives from Character so
// a walking item reuses gravity + velocity integration for free.
//
// An item collects itself through Entity::onCollision the moment it touches the player
// (and since it is not solid, the player is never pushed by it). The collision pair
// resolver skips items outright on top of that — see CollisionManager — so no damage
// path can ever reach one; collection stays safely in the hook that fires before the
// routing.
//
// Items can pop out of a ? block: beginEmergence() parks the item inside the block's
// cell and wraps all of its behaviour in the rise (see update()). While the pop runs the
// item has no physics, its updateBehavior() is not called, and it is drawn behind the
// terrain so it never overdraws the block face. The first frame after full clearance,
// onEmergenceComplete() lets the subclass resume (e.g. a Mushroom starts walking).
class Item : public Character {
public:
    Item(Vector2 position, Vector2 size);

    void update(float deltaTime) override;

    void onCollision(Entity& other, CollisionType side) override;

    // Apply the item's effect to the collector (the player). Subclasses override.
    virtual void onCollect(Entity& collector);

    // The player bumped the block this item is resting on from below (see the collision
    // pass's block-top scan). Subclasses that react to the shake — a Mushroom turns
    // around — override; static items (flower, star) take the no-op.
    virtual void onBlockHitFromBelow() {}

    // Items are passable: walking into one collects it instead of being blocked.
    bool isSolid() const override { return false; }

    bool drawsBehindTerrain() const override;

    // Spawns the item inside the block's cell and makes it rise out through the block's
    // top face. Call right after construction, before the item is exposed to the world.
    void beginEmergence(Vector2 blockPosition, Vector2 blockSize);

protected:
    // What the item does in normal play; subclasses override this instead of update() so
    // the emergence gate can wrap their behaviour. Defaults to Character::update.
    virtual void updateBehavior(float deltaTime);
    // Called the moment the item has fully cleared the block, so the subclass can resume
    // its motion (e.g. start walking).
    virtual void onEmergenceComplete();

private:
    std::unique_ptr<VerticalSlide> emergence;
};

}

#endif