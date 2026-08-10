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

    void handleInput() override;
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
    // Const power query for the view layer: renderers receive a `const Player&` and must
    // be able to tell Fire from Super without a non-const state reference.
    bool isFire() const;
    bool isStar() const;
    // Whether the player is currently in a two-tile-tall form. Asked instead of the power
    // state because a Star wraps whatever state it replaced and does not report isSuper(),
    // so a Super Mario under a star would otherwise read as small.
    bool isBig() const;

    // Big Mario headbutts breakable bricks apart; small Mario just bounces off them.
    bool canBreakBricks() const override;
    // Remaining post-damage invulnerability; the view fades the sprite off this, so the
    // blink and the invulnerability window always end together.
    float getBlinkRemaining() const;

    void becomeSuper();
    void becomeFire();
    void becomeStar();

    // Which playable character this is. Mario and Luigi share every pose and power-up; they
    // only differ by the row they live in on the spritesheet, so the view picks the row off
    // this instead of dynamic_casting the concrete type.
    virtual bool isLuigi() const;

    void addScore(int points);
    void addCoin();
    void addLife();

    int getScore() const;
    int getCoins() const;
    int getLives() const;
    int getHorizontalInput() const;
    bool isSprinting() const;
    bool isCrouching() const;
    float getAnimationClock() const;

protected:
    std::unique_ptr<PlayerState> state;
    int score;
    int coins;
    // Invulnerability window after losing a power-up, and at the same time the blink
    // timer: the renderer fades the sprite while this is above zero.
    float damageCooldown;
    // Press-edge jump tracking: jumpHeld remembers the raw button state from the
    // previous frame so a held key cannot re-trigger a jump on landing; playerInitiatedJump
    // marks the current ascent as started by the player (so releasing it cuts the jump,
    // but a stomp bounce is never cut).
    bool jumpHeld = false;
    bool playerInitiatedJump = false;
    int horizontalInput = 0;
    bool sprinting = false;
    bool crouching = false;
    float animationClock = 0.0f;

private:
    void syncAnimation();
    // Keep the entity box (and its collision hitbox) in step with the current power state:
    // Super and Fire are two tiles tall, Small is one; crouching (big only) drops to the
    // sitting pose's art height (CrouchHeight). The feet stay anchored, so growing and
    // shrinking never shoves the player through the floor. Called on every state swap and
    // on every crouch toggle; Star deliberately bypasses it (becomeStar never calls
    // setState), which is exactly why a star preserves whatever size the player had before
    // it.
    void syncPowerSize();
    // Post-damage invulnerability: the player blinks for exactly this long (the view
    // drives the blink alpha off damageCooldown), so invulnerability and the flicker
    // always end together — a blinking Mario can never be hit after the window the
    // animation implies has run out.
    static constexpr float DamageBlinkTime = 2.0f;
    static constexpr float SmallHeight = 32.0f;
    static constexpr float BigHeight = 64.0f;
    // Matches the sitting pose's art at the sheet's 2x render scale: the pose occupies
    // y=250..271 (22px) in the 32px-tall big cell, so the crouched box is 22*2 = 44 tall.
    static constexpr float CrouchHeight = 44.0f;
};

}

#endif
