#ifndef MODEL_ENEMY_H
#define MODEL_ENEMY_H

#include "Model/Character.h"

namespace model {

class Enemy : public Character {
public:
    Enemy(Vector2 position, Vector2 size);
    virtual ~Enemy() = default;

    void update(float deltaTime) override;
    virtual void updateAI(float deltaTime) = 0;

    // Stomp and damage hooks are Enemy semantics: CollisionManager dispatches them via
    // the collision layers, and only Enemy subclasses react.
    virtual void onStomped(Entity& player);
    // Knocked out by another enemy (e.g. a spinning shell): pop up and fall away.
    virtual void onHit(Entity& source);
    int getDamageValue() const;

    // True while the enemy shows its squished sprite (stomped but not gone yet).
    bool isSquished() const;

protected:
    int damageValue;
    bool isStomped;
    float despawnTimer;
};

}

#endif
