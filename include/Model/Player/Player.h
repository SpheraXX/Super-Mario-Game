#ifndef MODEL_PLAYER_H
#define MODEL_PLAYER_H

#include "Model/Character.h"
#include "Model/Player/PlayerState.h"

#include <memory>

namespace model {

class Player : public Character {
public:
    Player(Vector2 position, Vector2 size);
    ~Player() override;

    void update(float deltaTime) override;

    void handleInput(float deltaTime) override;
    void onCollision(Entity* other) override;
    void takeDamage(int amount) override;

    using Character::die;
    // Full death: lose a life and play the pop/fall death animation. bounce=false is
    // used for pit falls (the body just keeps dropping).
    void die(bool bounce);

    void setState(std::unique_ptr<PlayerState> newState);
    PlayerState& getState();
    const char* getStateName() const;
    float getRemainingTime() const;

    void becomeSuper();
    void becomeFire();
    void becomeStar();

    void addScore(int points);
    void addCoin();
    void addLife();

    // Score/coins/lives are owned by GameManager (single source of truth for the HUD);
    // the Player API keeps thin wrappers so gameplay code never touches the singleton.
    int getScore() const;
    int getCoins() const;
    int getLives() const;

    // Down-input intent from the last handleInput() (see the protected member).
    bool getInputDown() const { return inputDown; }

protected:
    std::unique_ptr<PlayerState> state;
    float damageCooldown;
    // Jump forgiveness: coyoteTime lets a jump fire shortly after leaving a platform,
    // jumpBufferTime remembers a press made slightly before landing. Both are tiny
    // windows (see constants) and make platforming feel far less brittle.
    float coyoteTime = 0.0f;
    float jumpBufferTime = 0.0f;
    // Continuous-jump state: jumpHoldTime tracks how long the current ascent has been
    // boosted; playerInitiatedJump marks the ascent as started by the player (so releasing
    // the key stops the boost, but a stomp bounce is never boosted).
    float jumpHoldTime = 0.0f;
    bool jumpHeld = false;
    bool playerInitiatedJump = false;
    // Horizontal-input intent from the last handleInput(): used by syncAnimation so that
    // pushing against a wall (velocity keeps getting zeroed by the push-out) still reads
    // as walking instead of flapping Walk/Idle every frame.
    bool inputMoving = false;
    // Down input intent from the last handleInput(): standing on a pipe and holding
    // Down enters it (play state reads this to teleport to the portal target).
    bool inputDown = false;

protected:
    static constexpr float DamageCooldownTime = 0.5f;
    static constexpr float CoyoteTime = 0.1f;
    static constexpr float JumpBufferTime = 0.1f;
    static constexpr float MaxJumpHoldTime = 0.16f;
    // First impulse of a jump (tap jump ≈ a small hop); the held key adds getJumpAccel()
    // on top, capped at getMaxJumpSpeed(). The hold is short enough that even a full hold
    // only rises barely over the 128px of a 4-tile wall (Mario: ≈136px) and stays clearly
    // under the 160px of a 5-tile wall — the deliberate "barely 4 blocks" jump. The weak
    // graze at the top of that arc is filtered by the bump-speed gate in CollisionManager.
    static constexpr float JumpInitialSpeed = -220.0f;
    // Horizontal inertia: velocity approaches the target speed, never snapping.
    static constexpr float GroundAccel = 800.0f;
    static constexpr float AirAccel = 600.0f;
    static constexpr float Friction = 1600.0f;

    // Animation hysteresis thresholds (self-explanatory in syncAnimation): entering walk
    // needs >10px/s, leaving it needs to drop below 2px/s (or lose the input), and the
    // walk/run split sits at 200px/s. The gaps stop the pose from flickering frame menus.
    static constexpr float IdleSpeedThreshold = 10.0f;
    static constexpr float StoppedSpeedThreshold = 2.0f;
    static constexpr float RunSpeedThreshold = 200.0f;

    // Underwater (simplified): holding the jump key continuously swims upward. The
    // world's scaled gravity + drag then keep the motion floaty instead of jumpy.
    static constexpr float SwimAccel = 900.0f;
    static constexpr float SwimMaxSpeed = 220.0f;

private:
    void syncAnimation();
};

}

#endif
