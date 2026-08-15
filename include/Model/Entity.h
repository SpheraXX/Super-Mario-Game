#ifndef MODEL_ENTITY_H
#define MODEL_ENTITY_H

#include "Model/Core/Vector2.h"
#include "Model/Core/Hitbox.h"
#include "Model/Core/CollisionResult.h"

namespace model {

// The level's service interface (Model/Core/World.h) — the channel an entity uses to
// spawn another entity or ask where the player is. Distinct from WorldTheme, which is
// the physics/graphics descriptor Character carries.
class World;

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

    // False for anything that moves on its own rules rather than through the world's
    // geometry: Lakitu drifting over terrain, a fireball passing through walls, a Piranha
    // Plant that lives inside a solid pipe. Skips the tile pass entirely — blanking
    // onTileCollision is not enough, because the push-out happens before that hook runs.
    virtual bool usesTileCollision() const { return true; }

    // False for enemies that hurt the player from above too (Spiny's spikes, Bowser), so
    // the collision routing damages instead of squashing and bouncing.
    virtual bool isStompable() const { return true; }

    // True for entities that smash a breakable brick when they hit one from below. Only a
    // big player can; everything else bounces off.
    virtual bool canBreakBricks() const { return false; }

    // True for entities drawn *before* the tile map, so the terrain covers them. A Piranha
    // Plant has to slide out from behind its pipe; drawn in the normal pass it would hang
    // visibly in front of the pipe at every retracted position.
    virtual bool drawsBehindTerrain() const { return false; }

    // The world an entity lives in: its channel for spawning (projectiles, transformations)
    // and for asking about the player. Set when the level takes ownership of the entity.
    // Note this is model::World (the level's service interface, Model/Core/World.h), not
    // model::WorldTheme (the physics/graphics descriptor that Character carries).
    void setWorld(World* w) { world = w; }

    Vector2 getPosition() const;
    Vector2 getSize() const;
    void setPosition(Vector2 newPosition);
    void setSize(Vector2 newSize);

    Hitbox hitbox;
    // In play: false once the level has reclaimed the entity (died, or left the world).
    bool isActive;
    // Placed but not yet woken: the camera has never come close enough. Dormant entities
    // do not update, collide, or draw. Distinct from isActive — a dormant entity is a
    // future participant, an inactive one is finished. Waking is one-way.
    //
    // Dormancy is not a motion property (it asks "does this participate yet?"), so unlike
    // velocity it belongs on Entity: a block or pipe far ahead of the camera is dormant
    // just as an enemy is.
    bool isDormant;

protected:
    World* world = nullptr;  // non-owning: the level outlives every entity in it

private:
    Vector2 position;
    Vector2 size;
};

}

#endif
