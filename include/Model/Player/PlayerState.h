#ifndef MODEL_PLAYERSTATE_H
#define MODEL_PLAYERSTATE_H

#include "Model/Character.h"

#include <memory>

namespace model {

class Player;

enum class PlayerAnimState {
    SmallIdle, SmallWalk, SmallRun, SmallJump, SmallFall, SmallDie,
    BigIdle,   BigWalk,   BigRun,   BigJump,   BigFall,
    FireIdle,  FireWalk,  FireRun,  FireJump,  FireFall, FireShoot,
    StarIdle,  StarWalk,  StarRun,  StarJump,  StarFall
};

class PlayerState {
public:
    virtual ~PlayerState() = default;

    virtual void update(Player& player, float deltaTime) = 0;
    virtual void onEnter(Player& player) = 0;
    virtual void onExit(Player& player) = 0;

    virtual PlayerState* takeDamage(Player& player) = 0;
    virtual PlayerAnimState getAnimState(const Player& player) const = 0;

    // Power-up queries so the player never checks concrete state types.
    virtual bool isSuper() const { return false; }
    virtual bool isFire() const { return false; }
    virtual bool isStar() const { return false; }

    // Shooting: only the underlying Fire state can fire, and a Star wrapped around it keeps
    // the ability (Star forwards to its previous state, so the cooldown lives in one place).
    // Player asks canShoot() before spending a shot and calls shoot() to start the cooldown.
    virtual bool canShoot() const { return false; }
    virtual void shoot() {}

    virtual const char* getStateName() const = 0;
    virtual float getRemainingTime() const { return -1.0f; }

    virtual std::unique_ptr<PlayerState> checkExpiration();
};

class SmallState : public PlayerState {
public:
    void update(Player& player, float deltaTime) override;
    void onEnter(Player& player) override;
    void onExit(Player& player) override;

    PlayerState* takeDamage(Player& player) override;
    PlayerAnimState getAnimState(const Player& player) const override;
    const char* getStateName() const override { return "Small"; }
};

class SuperState : public PlayerState {
public:
    void update(Player& player, float deltaTime) override;
    void onEnter(Player& player) override;
    void onExit(Player& player) override;

    PlayerState* takeDamage(Player& player) override;
    PlayerAnimState getAnimState(const Player& player) const override;
    bool isSuper() const override { return true; }
    const char* getStateName() const override { return "Super"; }
};

class FireState : public PlayerState {
public:
    void update(Player& player, float deltaTime) override;
    void onEnter(Player& player) override;
    void onExit(Player& player) override;

    PlayerState* takeDamage(Player& player) override;
    PlayerAnimState getAnimState(const Player& player) const override;

    bool isFire() const override { return true; }
    bool canShoot() const override;
    void shoot() override;

    const char* getStateName() const override { return "Fire"; }

private:
    float fireCooldown = 0.0f;
    // One ball per 1.5s; holding the fire key re-fires as soon as the cooldown clears.
    static constexpr float FireCooldownDuration = 1.0f;
};

class StarState : public PlayerState {
public:
    explicit StarState(std::unique_ptr<PlayerState> previous);

    void update(Player& player, float deltaTime) override;
    void onEnter(Player& player) override;
    void onExit(Player& player) override;

    PlayerState* takeDamage(Player& player) override;
    PlayerAnimState getAnimState(const Player& player) const override;

    std::unique_ptr<PlayerState> checkExpiration() override;

    bool isStar() const override { return true; }
    const char* getStateName() const override { return "Star"; }
    float getRemainingTime() const override { return duration; }

    // A star preserves the state it wrapped, so shooting follows that state: fire-under-star
    // can shoot (the cooldown lives in the wrapped FireState), anything else cannot.
    bool canShoot() const override;
    void shoot() override;

private:
    float duration;
    std::unique_ptr<PlayerState> previousState;
    static constexpr float StarDuration = 10.0f;
};

}

#endif
