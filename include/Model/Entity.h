#ifndef MODEL_ENTITY_H
#define MODEL_ENTITY_H

#include "Model/Core/Vector2.h"
#include "Model/Core/Hitbox.h"
#include "Model/Core/CollisionResult.h"

namespace model {

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

    Vector2 getPosition() const;
    Vector2 getSize() const;
    void setPosition(Vector2 newPosition);
    void setSize(Vector2 newSize);

    Vector2 getVelocity() const;
    void setVelocity(Vector2 v);

    Hitbox hitbox;
    Vector2 velocity;
    bool isActive;
    bool isGrounded;

private:
    Vector2 position;
    Vector2 size;
};

}

#endif
