#ifndef MODEL_CHARACTER_H
#define MODEL_CHARACTER_H

#include "Model/Entity.h"
#include "Model/CollisionResult.h"

namespace model {

enum class AnimState {
    Idle,
    Walk,
    Run,
    Jump,
    Fall,
    Die
};

class TileMap;

class Character : public Entity {
public:
    Character(Vector2 position, Vector2 size);
    ~Character() override = default;

    void update(float deltaTime) override;
    virtual void render();

    virtual void onCollision(Entity* other);
    virtual void takeDamage(int amount);

    void applyGravity(float deltaTime);
    bool isOnGround() const;
    void setMap(const TileMap* map);

    Vector2 getVelocity() const;
    void setVelocity(Vector2 v);

    int getDirection() const;
    void setDirection(int d);

    bool isAlive() const;

    AnimState getAnimState() const;
    void setAnimState(AnimState state);

    bool isFacingRight() const;
    void setFacingRight(bool right);

protected:
    void clampVelocity();
    void resolveTileCollisions();

    Vector2 velocity;
    int direction;
    int health;
    bool alive;
    AnimState animState;
    bool facingRight;
    const TileMap* mapPtr = nullptr;

    static constexpr float Gravity = 980.0f;
    static constexpr float MaxFallSpeed = 600.0f;
};

}

#endif
