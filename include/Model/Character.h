#ifndef MODEL_CHARACTER_H
#define MODEL_CHARACTER_H

#include "Model/Entity.h"
#include "Model/CollisionResult.h"

namespace sf { class RenderWindow; }

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
    virtual void render(sf::RenderWindow& window);

    virtual void onCollision(Entity* other);
    virtual void takeDamage(int amount);

    // Enter the death animation: the body pops up (if bounce) then falls away.
    // Dying bodies ignore tile collisions and stop all other interaction until the
    // level removes them after the fall.
    virtual void beginDying(bool bounce);
    bool isDying() const;

    void applyGravity(float deltaTime);
    virtual void move();
    virtual void die();
    
    bool isOnGround() const;
    void setMap(const TileMap* map);

    int getDirection() const;
    void setDirection(int d);

    bool isAlive() const;

    AnimState getAnimState() const;
    void setAnimState(AnimState state);

    bool isFacingRight() const;
    void setFacingRight(bool right);

    void resolveTileCollisions();

protected:
    void clampVelocity();

    int direction;
    int health;
    bool alive;
    bool isDyingFlag = false;
    float deathElapsed = 0.0f;
    AnimState animState;
    bool facingRight;
    const TileMap* mapPtr = nullptr;

    static constexpr float Gravity = 1600.0f;
    static constexpr float MaxFallSpeed = 900.0f;
    // Small upward pop applied when a death animation starts (e.g. from enemy contact).
    static constexpr float DeathBounceSpeed = -400.0f;
};

}

#endif
