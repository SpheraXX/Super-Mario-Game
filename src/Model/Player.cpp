#include "Model/Player.h"

#include <cmath>

namespace model {

Player::Player(Vector2 position, Vector2 size)
    : Character(position, size),
      state(std::make_unique<SmallState>()),
      score(0),
      coins(0),
      lives(3) {
}

Player::~Player() = default;

void Player::update(float deltaTime) {
    state->update(*this, deltaTime);
    Character::update(deltaTime);
    syncAnimation();

    if (auto newState = state->checkExpiration()) {
        state->onExit(*this);
        state = std::move(newState);
        state->onEnter(*this);
    }
}

void Player::render() {
}

void Player::handleInput() {
}

void Player::onCollision(Entity* /* other */) {
}

void Player::takeDamage(int amount) {
    if (!alive) return;

    PlayerState* newState = state->takeDamage(*this);
    if (newState == nullptr) {
        alive = false;
        animState = AnimState::Die;
    } else if (newState != state.get()) {
        state->onExit(*this);
        state.reset(newState);
        state->onEnter(*this);
    }
}

void Player::setState(std::unique_ptr<PlayerState> newState) {
    if (!newState) return;
    state->onExit(*this);
    state = std::move(newState);
    state->onEnter(*this);
}

PlayerState& Player::getState() {
    return *state;
}

void Player::becomeSuper() {
    setState(std::make_unique<SuperState>());
}

void Player::becomeFire() {
    setState(std::make_unique<FireState>());
}

void Player::becomeStar() {
    state->onExit(*this);
    auto prevState = std::move(state);
    state = std::make_unique<StarState>(std::move(prevState));
    state->onEnter(*this);
}

void Player::addScore(int points) {
    score += points;
}

void Player::addCoin() {
    coins++;
    if (coins >= 100) {
        coins -= 100;
        lives++;
    }
}

void Player::addLife() {
    lives++;
}

int Player::getScore() const {
    return score;
}

int Player::getCoins() const {
    return coins;
}

int Player::getLives() const {
    return lives;
}

void Player::syncAnimation() {
    if (!alive) {
        setAnimState(AnimState::Die);
        return;
    }

    Vector2 vel = getVelocity();
    bool grounded = isOnGround();

    if (vel.x > 0.1f) {
        setFacingRight(true);
    } else if (vel.x < -0.1f) {
        setFacingRight(false);
    }

    if (!grounded) {
        setAnimState(vel.y < 0 ? AnimState::Jump : AnimState::Fall);
    } else if (std::abs(vel.x) > 0.1f) {
        setAnimState(std::abs(vel.x) > 200.0f ? AnimState::Run : AnimState::Walk);
    } else {
        setAnimState(AnimState::Idle);
    }
}

}
