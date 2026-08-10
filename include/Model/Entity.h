#ifndef MODEL_ENTITY_H
#define MODEL_ENTITY_H

#include "Model/Core/Vector2.h"
#include "Model/Core/Hitbox.h"
#include "Model/Core/CollisionResult.h"

namespace model {

// A world object: anything placed in the level with a position, size and hitbox.
// Static objects (pipes, the flagpole, blocks) are plain world objects; everything
// that moves, takes damage or dies adds those capabilities in Character. Nothing in
// this interface implies life, motion or input, so e.g. a FlagPole can never be
// asked for a velocity or a death state.
class Entity {
public:
    Entity(Vector2 position, Vector2 size);
    virtual ~Entity() = default;

    virtual void update(float deltaTime);
    virtual void onCollision(Entity& other, CollisionType side);
    virtual void onTileCollision(char tile, CollisionType side);

    // Fired when an entity with a trigger hitbox overlaps the player (see the trigger
    // pass in CollisionManager). Default: nothing — trigger semantics live in the
    // concrete type (e.g. FlagPole marks itself touched).
    virtual void onTriggerEnter(Entity& other) { (void)other; }

    virtual bool isSolid() const { return false; }

    Vector2 getPosition() const;
    Vector2 getSize() const;
    void setPosition(Vector2 newPosition);
    void setSize(Vector2 newSize);

    Hitbox hitbox;
    bool isActive;

private:
    Vector2 position;
    Vector2 size;
};

}

#endif
