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
    const char* getStateName() const override { return "Super"; }
};

class FireState : public PlayerState {
public:
    void update(Player& player, float deltaTime) override;
    void onEnter(Player& player) override;
    void onExit(Player& player) override;

    PlayerState* takeDamage(Player& player) override;
    PlayerAnimState getAnimState(const Player& player) const override;

    bool canShoot() const;
    void shoot();

    const char* getStateName() const override { return "Fire"; }

private:
    float fireCooldown = 0.0f;
    static constexpr float FireCooldownDuration = 0.5f;
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

    const char* getStateName() const override { return "Star"; }
    float getRemainingTime() const override { return duration; }

private:
    float duration;
    std::unique_ptr<PlayerState> previousState;
    static constexpr float StarDuration = 10.0f;
};

}

#endif
