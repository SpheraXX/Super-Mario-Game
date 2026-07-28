#ifndef MODEL_ENTITY_H
#define MODEL_ENTITY_H

#include "Model/Vector2.h"
#include "Model/Hitbox.h"
#include "Model/CollisionResult.h"

namespace model {

class Entity {
public:
    Entity(Vector2 position, Vector2 size);
    virtual ~Entity() = default;

    virtual void update(float deltaTime);
    virtual void onCollision(Entity& other, CollisionType side);
    virtual void onTileCollision(char tile, CollisionType side);

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
