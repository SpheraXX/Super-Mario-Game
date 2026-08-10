#ifndef MODEL_CHARACTER_H
#define MODEL_CHARACTER_H

#include "Model/Entity.h"
#include "Model/Core/CollisionResult.h"

namespace model {

enum class AnimState {
    Idle,
    Walk,
    Run,
    Jump,
    Fall,
    Crouch,
    Die
};

class TileMap;

class Character : public Entity {
public:
    Character(Vector2 position, Vector2 size);
    ~Character() override = default;

    void update(float deltaTime) override;

    virtual void onCollision(Entity* other);
    virtual void takeDamage(int amount) override;

    // Enter the death animation: the body pops up (if bounce) then falls away.
    // Dying bodies ignore tile collisions and stop all other interaction until the
    // level removes them after the fall.
    virtual void beginDying(bool bounce);
    bool isDying() const override;
    bool isAlive() const override;

    // Movement tuning: subclasses (Mario/Luigi) override with their own numbers so the
    // player controller never needs to know which character it is driving.
    virtual float getWalkSpeed() const;
    virtual float getRunSpeed() const;
    virtual float getJumpForce() const;

    void applyGravity(float deltaTime);

    // Per-character gravity multiplier. 1.0 is a normal walker; 0.0 pins a character to
    // its own vertical logic (Piranha Plant riding its pipe, Lakitu hovering); small
    // values read as buoyancy (underwater Cheep Cheep) or a lazy projectile arc.
    // Does not affect the death fall, which always uses full gravity so every body
    // reliably drops out of the world and gets cleaned up.
    float getGravityScale() const;
    void setGravityScale(float scale);

    virtual void move();
    virtual void die();
    
    bool isOnGround() const;

    int getDirection() const;
    void setDirection(int d);

    AnimState getAnimState() const;
    void setAnimState(AnimState state);

    bool isFacingRight() const;
    void setFacingRight(bool right);

protected:
    void clampVelocity();

    int direction;
    int health;
    bool alive;
    bool isDyingFlag = false;
    float deathElapsed = 0.0f;
    AnimState animState;
    bool facingRight;
    float gravityScale = 1.0f;

    static constexpr float Gravity = 1600.0f;
    static constexpr float MaxFallSpeed = 900.0f;
    // Small upward pop applied when a death animation starts (e.g. from enemy contact).
    static constexpr float DeathBounceSpeed = -400.0f;
};

}

#endif
