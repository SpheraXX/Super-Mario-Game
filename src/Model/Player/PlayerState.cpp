#include "Model/Player/PlayerState.h"
#include "Model/Player/Player.h"

namespace model {

std::unique_ptr<PlayerState> PlayerState::checkExpiration() {
    return nullptr;
}

void SmallState::update(Player& /* player */, float /* deltaTime */) {
}

void SmallState::onEnter(Player& /* player */) {
}

void SmallState::onExit(Player& /* player */) {
}

PlayerState* SmallState::takeDamage(Player& /* player */) {
    return nullptr;
}

PlayerAnimState SmallState::getAnimState(const Player& player) const {
    switch (player.getAnimState()) {
        case AnimState::Jump: return PlayerAnimState::SmallJump;
        case AnimState::Fall: return PlayerAnimState::SmallFall;
        case AnimState::Walk: return PlayerAnimState::SmallWalk;
        case AnimState::Run:  return PlayerAnimState::SmallRun;
        case AnimState::Die:  return PlayerAnimState::SmallDie;
        default:              return PlayerAnimState::SmallIdle;
    }
}

void SuperState::update(Player& /* player */, float /* deltaTime */) {
}

void SuperState::onEnter(Player& /* player */) {
}

void SuperState::onExit(Player& /* player */) {
}

PlayerState* SuperState::takeDamage(Player& /* player */) {
    return new SmallState();
}

PlayerAnimState SuperState::getAnimState(const Player& player) const {
    switch (player.getAnimState()) {
        case AnimState::Jump: return PlayerAnimState::BigJump;
        case AnimState::Fall: return PlayerAnimState::BigFall;
        case AnimState::Walk: return PlayerAnimState::BigWalk;
        case AnimState::Run:  return PlayerAnimState::BigRun;
        default:              return PlayerAnimState::BigIdle;
    }
}

void FireState::update(Player& /* player */, float deltaTime) {
    if (fireCooldown > 0.0f) {
        fireCooldown -= deltaTime;
        if (fireCooldown < 0.0f) {
            fireCooldown = 0.0f;
        }
    }
}

void FireState::onEnter(Player& /* player */) {
    fireCooldown = 0.0f;
}

void FireState::onExit(Player& /* player */) {
    fireCooldown = 0.0f;
}

PlayerState* FireState::takeDamage(Player& /* player */) {
    return new SmallState();
}

PlayerAnimState FireState::getAnimState(const Player& player) const {
    switch (player.getAnimState()) {
        case AnimState::Jump: return PlayerAnimState::FireJump;
        case AnimState::Fall: return PlayerAnimState::FireFall;
        case AnimState::Walk: return PlayerAnimState::FireWalk;
        case AnimState::Run:  return PlayerAnimState::FireRun;
        default:              return PlayerAnimState::FireIdle;
    }
}

bool FireState::canShoot() const {
    return fireCooldown <= 0.0f;
}

void FireState::shoot() {
    fireCooldown = FireCooldownDuration;
}

StarState::StarState(std::unique_ptr<PlayerState> previous)
    : duration(StarDuration), previousState(std::move(previous)) {
}

void StarState::update(Player& /* player */, float deltaTime) {
    duration -= deltaTime;
    if (duration < 0.0f) {
        duration = 0.0f;
    }
}

void StarState::onEnter(Player& /* player */) {
    duration = StarDuration;
}

void StarState::onExit(Player& /* player */) {
    duration = 0.0f;
}

PlayerState* StarState::takeDamage(Player& /* player */) {
    return this;
}

PlayerAnimState StarState::getAnimState(const Player& player) const {
    switch (player.getAnimState()) {
        case AnimState::Jump: return PlayerAnimState::StarJump;
        case AnimState::Fall: return PlayerAnimState::StarFall;
        case AnimState::Walk: return PlayerAnimState::StarWalk;
        case AnimState::Run:  return PlayerAnimState::StarRun;
        default:              return PlayerAnimState::StarIdle;
    }
}

std::unique_ptr<PlayerState> StarState::checkExpiration() {
    if (duration <= 0.0f && previousState) {
        return std::move(previousState);
    }
    return nullptr;
}

}
