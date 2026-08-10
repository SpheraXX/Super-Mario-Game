#ifndef MODEL_CHARACTER_H
#define MODEL_CHARACTER_H

#include "Model/Entity.h"

namespace model {

class TileMap;
class World;

enum class AnimState {
    Idle,
    Walk,
    Run,
    Jump,
    Fall,
    Die
};

// A living, moving world object: owns its velocity, gravity-driven motion, health
// and death state. Player, enemies and NPCs derive from this; static objects (pipes,
// the flagpole, blocks) stay plain world objects without motion or life semantics.
class Character : public Entity {
public:
    Character(Vector2 position, Vector2 size);
    ~Character() override = default;

    void update(float deltaTime) override;

    // Input gathering is delegated polymorphically: only the player reacts. It runs
    // BEFORE update() so gravity & integration see the correct player-intended
    // velocity, not stale values.
    virtual void handleInput(float deltaTime) { (void)deltaTime; }

    virtual void takeDamage(int amount);

    // Enter the death animation: the body pops up (if bounce) then falls away.
    // Dying bodies ignore tile collisions and stop all other interaction until the
    // level removes them after the fall.
    virtual void beginDying(bool bounce);
    bool isDying() const;
    bool isAlive() const;

    // Movement tuning: subclasses (Mario/Luigi) override with their own numbers so the
    // player controller never needs to know which character it is driving. Jumps are
    // continuous: the initial impulse is followed by a hold-time acceleration (getJumpAccel)
    // capped at getMaxJumpSpeed, so a full-length hold rises exactly MaxJumpSpeed^2/2G.
    virtual float getWalkSpeed() const;
    virtual float getRunSpeed() const;
    virtual float getMaxJumpSpeed() const;
    virtual float getJumpAccel() const;

    void applyGravity(float deltaTime);
    virtual void die();
    
    bool isOnGround() const;
    void setMap(const TileMap* map);

    // The world this character lives in (set by PlayState at spawn, like setMap). It
    // carries the gravity/fall/drag tuning the physics below reads; with no world set
    // the classic land constants apply, so existing tests keep their exact numbers.
    void setWorld(const World& world);
    float getGravity() const;
    float getMaxFallSpeed() const;
    float getHorizontalDrag() const;
    bool isUnderwater() const;

    Vector2 getVelocity() const;
    void setVelocity(Vector2 v);

    int getDirection() const;
    void setDirection(int d);

    AnimState getAnimState() const;
    void setAnimState(AnimState state);

    bool isFacingRight() const;
    void setFacingRight(bool right);

    // Whether the character is resting on solid ground or a block top this frame.
    bool isGrounded;

protected:
    Vector2 velocity;

    int direction;
    int health;
    bool alive;
    bool isDyingFlag = false;
    AnimState animState;
    bool facingRight;
    const TileMap* mapPtr = nullptr;
    const World* worldPtr = nullptr;

    // Land (Overworld) physics constants. Worlds scale these through their World
    // descriptor; keeping them here means a character without a world behaves exactly
    // like the tuned default.
    static constexpr float DefaultGravity = 1600.0f;
    static constexpr float DefaultMaxFallSpeed = 900.0f;
    // Small upward pop applied when a death animation starts (e.g. from enemy contact).
    static constexpr float DeathBounceSpeed = -400.0f;
};

}

#endif
