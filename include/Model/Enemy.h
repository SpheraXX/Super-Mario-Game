#ifndef MODEL_ENEMY_H
#define MODEL_ENEMY_H

#include "Model/Character.h"

namespace model {

class Player;

class Enemy : public Character {
public:
    Enemy(Vector2 position, Vector2 size);
    virtual ~Enemy() = default;

    void update(float deltaTime) override;
    virtual void updateAI(float deltaTime) = 0;

    virtual void onStomped(Player& player);
    virtual void onHit(Entity& source);

    int getDamageValue() const;

protected:
    int damageValue;
    bool isStomped;
    float despawnTimer;
};

}

#endif
