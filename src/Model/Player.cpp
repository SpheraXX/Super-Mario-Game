#include "Model/Player.h"
#include "Model/Mario.h"
#include "Model/Luigi.h"
#include "Model/GameManager.h"

#include <cmath>

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

namespace model {

Player::Player(Vector2 position, Vector2 size)
    : Character(position, size),
      state(std::make_unique<SmallState>()),
      score(0),
      coins(0),
      damageCooldown(0.0f) {
}

Player::~Player() = default;

void Player::update(float deltaTime) {
    // A player-initiated ascent only owns the rising half of the jump: once the apex is
    // passed the flag is cleared, so a stomp bounce (which happens while falling) can
    // never be cut by a released jump key.
    if (velocity.y >= 0.0f) {
        playerInitiatedJump = false;
    }

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
    return;
    if (!alive) return;

    Vector2 pos = getPosition();
    Vector2 sz = getSize();

    sf::RectangleShape rect({sz.x, sz.y});
    rect.setPosition({pos.x, pos.y});

    sf::Color baseColor = sf::Color::Red;
    if (dynamic_cast<Luigi*>(this)) {
        baseColor = sf::Color::Green;
    }

    if (dynamic_cast<StarState*>(state.get())) {
        rect.setFillColor(sf::Color::Yellow);
    } else if (dynamic_cast<FireState*>(state.get())) {
        rect.setFillColor(sf::Color(255, 165, 0));
    } else if (dynamic_cast<SuperState*>(state.get())) {
        rect.setFillColor(sf::Color(200, 200, 200));
    } else {
        rect.setFillColor(baseColor);
    }

    if (!facingRight) {
        rect.setScale({-1.0f, 1.0f});
        rect.setOrigin({sz.x, 0.0f});
    }

    window.draw(rect);
}

void Player::handleInput() {
    if (!alive) return;

    float walkSpeed = 180.0f;
    float runSpeed = 320.0f;
    float jumpForce = -450.0f;

    if (dynamic_cast<Mario*>(this)) {
        walkSpeed = Mario::WalkSpeed;
        runSpeed = Mario::RunSpeed;
        jumpForce = Mario::JumpForce;
    } else if (dynamic_cast<Luigi*>(this)) {
        walkSpeed = Luigi::WalkSpeed;
        runSpeed = Luigi::RunSpeed;
        jumpForce = Luigi::JumpForce;
    }

    // Sprint: hold Shift to move at run speed.
    const bool sprinting =
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);
    const float currentSpeed = sprinting ? runSpeed : walkSpeed;

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

    const bool jumpPressed =
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

    // No jump buffering: a jump fires only on the press edge while grounded, so holding
    // the key does nothing until it is released and pressed again.
    if (jumpPressed && !jumpHeld && isOnGround()) {
        velocity.y = jumpForce;
        playerInitiatedJump = true;
    }

    // Jump cut: releasing the key mid-ascent kills the upward velocity for a snappy,
    // Mario-like feel (hold to jump higher).
    if (!jumpPressed && playerInitiatedJump && velocity.y < 0.0f) {
        velocity.y = 0.0f;
        playerInitiatedJump = false;
    }

    jumpHeld = jumpPressed;
}

void Player::onCollision(Entity* /* other */) {
}

void Player::takeDamage(int amount) {
    if (!alive || isDying() || damageCooldown > 0.0f) return;

    PlayerState* newState = state->takeDamage(*this);
    if (newState == nullptr) {
        // Small Mario with no power-up left to lose: full death.
        die(true);
    } else if (newState != state.get()) {
        // Downgrade (e.g. Super -> Small): keep playing with brief invulnerability.
        state->onExit(*this);
        state.reset(newState);
        state->onEnter(*this);
        damageCooldown = DamageCooldownTime;
    }
}

void Player::die(bool bounce) {
    if (!alive || isDying()) return;
    model::GameManager::instance().loseLife();
    beginDying(bounce);
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
        model::GameManager::instance().addLife();
    }
}

void Player::addLife() {
    model::GameManager::instance().addLife();
}

int Player::getScore() const {
    return score;
}

int Player::getCoins() const {
    return coins;
}

int Player::getLives() const {
    return model::GameManager::instance().getLives();
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
