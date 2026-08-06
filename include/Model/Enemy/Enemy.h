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

    virtual void onStomped(Entity& player);
    virtual void onHit(Entity& source) override;
    int getDamageValue() const override;

    // True while the enemy shows its squished sprite (stomped but not gone yet).
    bool isSquished() const;

protected:
    int damageValue;
    bool isStomped;
    float despawnTimer;
};

}

#endif
