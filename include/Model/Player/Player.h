#ifndef MODEL_PLAYER_H
#define MODEL_PLAYER_H

#include "Model/Character.h"
#include "Model/Core/VerticalSlide.h"
#include "Model/Input/InputSnapshot.h"

#include <memory>

namespace model {

// The power axis: one exclusive slot. Fire and Star replace each other — they are
// incompatible, so a Star picked over Fire drops the fireball for good (no restore on
// expiry), and a Flower picked over Star drops the invincibility. Size is a separate,
// orthogonal axis (the `big` flag): Mushroom only touches that one.
enum class PlayerPower {
    None,
    Fire,
    Star
};

// What the collectible hands to Player::applyPowerUp. The compatibility rules and the
// redundant-power-up scoring live in ONE place (applyPowerUp), not in the item classes.
enum class PlayerPowerUp {
    Mushroom,
    FireFlower,
    Star
};

class Player : public Character {
public:
    Player(Vector2 position, Vector2 size);
    ~Player() override;

    void update(float deltaTime) override;

    void handleInput(float deltaTime, const InputSnapshot& input) override;
    void takeDamage(int amount) override;

    using Character::die;
    // Full death: lose a life and play the pop/fall death animation. bounce=false is
    // used for pit falls (the body just keeps dropping).
    void die(bool bounce);

    // Single entry point for every power-up item. Enforces the whole compatibility table
    // (Mushroom = size only, Fire <-> Star override each other) and hands out the 1000
    // points when the collected power-up is redundant.
    void applyPowerUp(PlayerPowerUp type);

    // Remaining star invincibility, or -1 when not starred. The view flashes the sprite
    // off this, so the flashing and the invincibility always end together.
    float getRemainingTime() const;

    // Const power queries for the view and for gameplay routing: renderers receive a
    // `const Player&` and must tell Fire from Star without a non-const state reference.
    bool isFire() const;
    bool isStar() const;
    // Whether the player is currently in a two-tile-tall form. Read off the size axis,
    // not off the power: Fire and Star never change size, so the only way to grow is a
    // Mushroom (and the only way to shrink is damage).
    bool isBig() const;
    // Big Mario headbutts breakable bricks apart; small Mario just bounces off them.
    bool canBreakBricks() const override;

    // Total horizontal ground covered, in world units. The view picks the walk frame off
    // this instead of a timer, so the cycle stays in step with how fast the player is
    // actually moving.
    float getWalkCycleDistance() const;
    // Remaining post-damage invulnerability; the view fades the sprite off this, so the
    // blink and the invulnerability window always end together.
    float getBlinkRemaining() const;

    // Which playable character this is. Mario and Luigi share every pose and power-up; they
    // only differ by the row they live in on the spritesheet, so the view picks the row off
    // this instead of dynamic_casting the concrete type.
    virtual bool isLuigi() const;

    // Stomp-bounce tuning, read by CollisionManager when the player lands on a stompable
    // enemy: bounceUp = max(0, getStompBounceRatio() * fallSpeed - getStompBounceConstant()),
    // where fallSpeed is his downward speed at the moment of impact. Movement tuning is
    // polymorphic (see getWalkSpeed), so each character reports its own rebound; the Player
    // defaults are the stock feel and Mario inherits them.
    virtual float getStompBounceRatio() const;
    virtual float getStompBounceConstant() const;

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

    // Pipe-entry state (the shared VerticalSlide component, same motion as items popping
    // out of blocks): while it runs the player has no physics, no input and no damage —
    // the scene freezes the world and drives the slide directly, and the body is drawn
    // behind the terrain so the pipe mouth covers it. beginPipeSlide() slides from the
    // current y to targetY; advancePipeSlide() returns true while still sliding and
    // false the frame the target is reached; endPipeSlide() drops the state.
    bool isPipeSliding() const;
    void beginPipeSlide(float targetY);
    bool advancePipeSlide(float deltaTime);
    void endPipeSlide();

    // While the slide runs the player must not overdraw the pipe mouth (mirror of the
    // items' emergence draw rule).
    bool drawsBehindTerrain() const override;

protected:
    // Orthogonal power-up axes. `big` is the size axis (Mushroom sets it, damage clears
    // it); `power` is the exclusive ability slot (Fire/Star replace each other).
    bool big = false;
    PlayerPower power = PlayerPower::None;
    // Star invincibility remaining, counting down to None on expiry.
    float starDuration = 0.0f;
    // Fireball refire gate; handleInput consumes it on a shot.
    float fireCooldown = 0.0f;
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
    // Accumulated horizontal distance driving the walk cycle. See getWalkCycleDistance.
    float walkCycleDistance = 0.0f;
    bool jumpHeld = false;
    bool playerInitiatedJump = false;
    // Horizontal-input intent from the last handleInput(): used by syncAnimation so that
    // pushing against a wall (velocity keeps getting zeroed by the push-out) still reads
    // as walking instead of flapping Walk/Idle every frame.
    bool inputMoving = false;
    // Down input intent from the last handleInput(): standing on a pipe and holding
    // Down enters it (play state reads this to teleport to the portal target).
    bool inputDown = false;

    static constexpr float DamageCooldownTime = 1.0f;
    // Height of the one-tile (Small) form, used by isBig(). Super/Fire are two tiles.
    static constexpr float SmallHeight = 16.0f;
    static constexpr float BigHeight = 32.0f;
    static constexpr float CoyoteTime = 0.1f;
    static constexpr float JumpBufferTime = 0.1f;
    static constexpr float MaxJumpHoldTime = 0.16f;
    // First impulse of a jump (tap jump ≈ a small hop); the held key adds getJumpAccel()
    // on top, capped at getMaxJumpSpeed(). The hold is short enough that even a full hold
    // only rises barely over the 128px of a 4-tile wall (Mario: ≈136px) and stays clearly
    // under the 160px of a 5-tile wall — the deliberate "barely 4 blocks" jump. The weak
    // graze at the top of that arc is filtered by the bump-speed gate in CollisionManager.
    static constexpr float JumpInitialSpeed = -110.0f;
    // Stock stomp-bounce defaults (see getStompBounceRatio/getStompBounceConstant): a rebound
    // off the impact speed, not a fixed kick. At a normal ~350px/s drop, 0.85 * 350 - 30 =
    // 267px/s upward — a classic-feel relaunch; at a ~35px/s trickle the max(0, ...) floor
    // cuts the bounce to nothing, so a slow drop is merely absorbed.
    static constexpr float StompBounceRatio = 0.85f;
    static constexpr float StompBounceConstant = 30.0f;
    // Horizontal inertia: velocity approaches the target speed, never snapping.
    //
    // Retuned for responsiveness. The original 800/600/1600 (with the Overworld's 0.4/s
    // drag on top) took 0.50s to reach run speed and 0.22s to coast to a stop, which read
    // as skating on ice: the player moved a beat after the key went down and kept going a
    // beat after it came up. These values keep the weighty feel — the velocity is still
    // never snapped — while cutting that to 0.23s and 0.15s.
    static constexpr float GroundAccel = 800.0f;
    // Air control stays below ground accel on purpose: committing to a jump direction is
    // part of the platforming. At 1200 the run speed is reachable in the first third of a
    // full jump (0.34s of 0.85s) instead of the last fifth.
    static constexpr float AirAccel = 600.0f;
    static constexpr float Friction = 1200.0f;

    // Animation hysteresis thresholds (self-explanatory in syncAnimation): entering walk
    // needs >10px/s, leaving it needs to drop below 2px/s (or lose the input), and the
    // walk/run split sits at 200px/s. The gaps stop the pose from flickering frame menus.
    static constexpr float IdleSpeedThreshold = 5.0f;
    static constexpr float StoppedSpeedThreshold = 1.0f;
    static constexpr float RunSpeedThreshold = 100.0f;

    // Underwater (simplified): holding the jump key continuously swims upward. The
    // world's scaled gravity + drag then keep the motion floaty instead of jumpy.
    static constexpr float SwimAccel = 450.0f;
    static constexpr float SwimMaxSpeed = 110.0f;

private:
    // The two axes above are driven from here only; nothing outside Player may mutate
    // big/power directly.
    // Fireball refire cooldown after a shot.
    static constexpr float FireCooldownDuration = 0.5f;
    // Star invincibility length; the countdown lives in starDuration.
    static constexpr float StarDuration = 10.0f;

    void syncAnimation();
    // Keep the entity box (and its collision hitbox) in step with the size axis: big is
    // two tiles tall, small is one. The feet stay anchored, so growing and shrinking never
    // shove the player through the floor. Called after every change to `big`.
    void syncPowerSize();

    // Pipe-entry state; owned here so it survives area changes (the scene's keepPlayer
    // rebuild keeps the whole entity, state included).
    std::unique_ptr<VerticalSlide> pipeSlide;
};

}

#endif
