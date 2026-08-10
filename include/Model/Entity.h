#ifndef MODEL_ENTITY_H
#define MODEL_ENTITY_H

#include "Model/Core/Vector2.h"
#include "Model/Core/Hitbox.h"
#include "Model/Core/CollisionResult.h"

namespace model {

class World;

class Entity {
public:
    Entity(Vector2 position, Vector2 size);
    virtual ~Entity() = default;

    virtual void update(float deltaTime);
    virtual void onCollision(Entity& other, CollisionType side);
    virtual void onTileCollision(char tile, CollisionType side);

    // Behaviour hooks with safe defaults: game code (Controller, CollisionManager) can
    // drive entities through these virtuals instead of checking concrete types.
    virtual void handleInput() {}
    virtual void takeDamage(int amount) { (void)amount; }
    virtual void onStomped(Entity& stomper) { (void)stomper; }
    virtual void onHit(Entity& source) { (void)source; }
    virtual int getDamageValue() const { return 0; }

    // Semantic state queries (Characters override; plain entities are alive and passable).
    virtual bool isAlive() const { return true; }
    virtual bool isDying() const { return false; }
    virtual bool isSolid() const { return false; }

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
    void setWorld(World* w) { world = w; }

    Vector2 getPosition() const;
    Vector2 getSize() const;
    void setPosition(Vector2 newPosition);
    void setSize(Vector2 newSize);

    Vector2 getVelocity() const;
    void setVelocity(Vector2 v);

    Hitbox hitbox;
    Vector2 velocity;
    // In play: false once the level has reclaimed the entity (died, or left the world).
    bool isActive;
    // Placed but not yet woken: the camera has never come close enough. Dormant entities
    // do not update, collide, or draw. Distinct from isActive — a dormant entity is a
    // future participant, an inactive one is finished. Waking is one-way.
    bool isDormant;
    bool isGrounded;

protected:
    World* world = nullptr;  // non-owning: the level outlives every entity in it

private:
    Vector2 position;
    Vector2 size;
};

}

#endif
