#ifndef MODEL_ENTITY_H
#define MODEL_ENTITY_H

#include "Model/Core/Vector2.h"
#include "Model/Core/Hitbox.h"
#include "Model/Core/CollisionResult.h"

namespace model {

// A world object: anything placed in the level with a position, size and hitbox.
// Static objects (pipes, the flagpole, blocks) are plain world objects; everything
// that moves, takes damage or dies adds those capabilities in Character. Nothing in
// this interface implies motion or input, so e.g. a FlagPole can never be asked for
// a velocity.
//
// Life state (isAlive/isDying) is the one exception, and it is deliberate: the
// collision passes walk a heterogeneous vector<Entity*> and must skip dying bodies,
// so the question has to be answerable for every element. Static objects take the
// defaults (alive, not dying) and never think about it again.
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

    // Life state, asked of every entity in the collision passes. Static world objects
    // (pipes, blocks, the flagpole) are permanently alive and never dying, so the
    // defaults are right for them and only Character overrides.
    //
    // These are deliberately on Entity rather than Character: the collision manager
    // walks a heterogeneous vector<Entity*> and needs the answer for every element.
    // Declaring them here is what lets it ask directly instead of dynamic_cast-ing to
    // Character on every entity pair, every frame. Motion (velocity, isGrounded) stays
    // on Character — being alive is not the same capability as being able to move.
    virtual bool isAlive() const { return true; }
    virtual bool isDying() const { return false; }

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
