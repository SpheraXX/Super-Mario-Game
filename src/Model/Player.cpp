#include "Model/Player.h"
#include "Model/Mario.h"
#include "Model/Luigi.h"

#include <cmath>

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/ConvexShape.hpp>

namespace model {

Player::Player(Vector2 position, Vector2 size)
    : Character(position, size),
      state(std::make_unique<SmallState>()),
      score(0),
      coins(0),
      lives(3),
      damageCooldown(0.0f) {
}

Player::~Player() = default;

void Player::update(float deltaTime) {
    state->update(*this, deltaTime);
    Character::update(deltaTime);
    syncAnimation();

    if (damageCooldown > 0.0f) {
        damageCooldown -= deltaTime;
        if (damageCooldown < 0.0f) damageCooldown = 0.0f;
    }

    if (auto newState = state->checkExpiration()) {
        state->onExit(*this);
        state = std::move(newState);
        state->onEnter(*this);
    }
}

void Player::render(sf::RenderWindow& window) {
    if (!alive) return;

    Vector2 pos = getPosition();
    Vector2 sz = getSize();

    sf::ConvexShape shape(5);
    shape.setPoint(0, {0.0f, 0.0f});
    shape.setPoint(1, {sz.x*0.7f, 0.0f});
    shape.setPoint(2, {sz.x, sz.y*0.5f});
    shape.setPoint(3, {sz.x*0.7f, sz.y});
    shape.setPoint(4, {0.0f, sz.y});
    shape.setPosition({pos.x, pos.y});

    sf::Color baseColor = sf::Color::Red;
    if (dynamic_cast<Luigi*>(this)) {
        baseColor = sf::Color::Green;
    }

    if (dynamic_cast<StarState*>(state.get())) {
        shape.setFillColor(sf::Color::Yellow);
    } else if (dynamic_cast<FireState*>(state.get())) {
        shape.setFillColor(sf::Color(255, 165, 0));
    } else if (dynamic_cast<SuperState*>(state.get())) {
        shape.setFillColor(sf::Color(200, 200, 200));
    } else {
        shape.setFillColor(baseColor);
    }

    if (!facingRight) {
        shape.setScale({-1.0f, 1.0f});
        shape.setOrigin({sz.x, 0.0f});
    }

    window.draw(shape);
}

void Player::handleInput() {
    if (!alive) return;

    float walkSpeed = 180.0f;
    float runSpeed = 320.0f;
    float jumpForce = -450.0f;

    if (auto* mario = dynamic_cast<Mario*>(this)) {
        walkSpeed = Mario::WalkSpeed;
        runSpeed = Mario::RunSpeed;
        jumpForce = Mario::JumpForce;
    } else if (auto* luigi = dynamic_cast<Luigi*>(this)) {
        walkSpeed = Luigi::WalkSpeed;
        runSpeed = Luigi::RunSpeed;
        jumpForce = Luigi::JumpForce;
    }

    float currentSpeed = 0.0f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1))
    {
        becomeSuper();
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2)) {
        becomeFire();
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3)) {
        becomeStar();
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift)) {
        currentSpeed = runSpeed;
    } else {
        currentSpeed = walkSpeed;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
        velocity.x = -currentSpeed;
        setDirection(-1);
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
               sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
        velocity.x = currentSpeed;
        setDirection(1);
    } else {
        velocity.x = 0.0f;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
        if (isOnGround()) {
            velocity.y = jumpForce;
        }
    }
}

void Player::onCollision(Entity* /* other */) {
}

void Player::takeDamage(int amount) {
    if (!alive || damageCooldown > 0.0f) return;

    PlayerState* newState = state->takeDamage(*this);
    if (newState == nullptr) {
        lives--;
        if (lives <= 0) {
            alive = false;
            animState = AnimState::Die;
        } else {
            setPosition({200.0f, 600.0f});
            setVelocity({0.0f, 0.0f});
            state->onExit(*this);
            state = std::make_unique<SmallState>();
            state->onEnter(*this);
            damageCooldown = DamageCooldownTime;
        }
    } else if (newState != state.get()) {
        state->onExit(*this);
        state.reset(newState);
        state->onEnter(*this);
        damageCooldown = DamageCooldownTime;
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

const char* Player::getStateName() const {
    return state->getStateName();
}

float Player::getRemainingTime() const {
    return state->getRemainingTime();
}

void Player::becomeSuper() {
    if (dynamic_cast<SuperState*>(state.get()) || dynamic_cast<FireState*>(state.get()) || dynamic_cast<StarState*>(state.get())) return;
    setState(std::make_unique<SuperState>());
}

void Player::becomeFire() {
    if (dynamic_cast<FireState*>(state.get()) || dynamic_cast<StarState*>(state.get())) return;
    setState(std::make_unique<FireState>());
}

void Player::becomeStar() {
    if (dynamic_cast<StarState*>(state.get())) return;
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
