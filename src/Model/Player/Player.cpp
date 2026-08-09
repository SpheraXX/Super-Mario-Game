#include "Model/Player/Player.h"
#include "Model/Core/GameManager.h"

#include <cmath>

#include <SFML/Window/Keyboard.hpp>

namespace model {

Player::Player(Vector2 position, Vector2 size)
    : Character(position, size),
      state(std::make_unique<SmallState>()),
      damageCooldown(0.0f) {
    // The collision layer drives the (type-check-free) routing in CollisionManager:
    // exactly one entity per pair must be the player for the pair to resolve.
    hitbox.layer = CollisionLayer::Player;
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

void Player::handleInput() {
    if (!alive) return;

    // Movement tuning is polymorphic: each concrete character reports its own numbers.
    const float walkSpeed = getWalkSpeed();
    const float runSpeed = getRunSpeed();
    const float jumpForce = getJumpForce();

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
    if (state->isSuper() || state->isFire() || state->isStar()) return;
    setState(std::make_unique<SuperState>());
}

void Player::becomeFire() {
    if (state->isFire() || state->isStar()) return;
    setState(std::make_unique<FireState>());
}

void Player::becomeStar() {
    if (state->isStar()) return;
    state->onExit(*this);
    auto prevState = std::move(state);
    state = std::make_unique<StarState>(std::move(prevState));
    state->onEnter(*this);
}

void Player::addScore(int points) {
    model::GameManager::instance().addScore(points);
}

void Player::addLife() {
    model::GameManager::instance().addLife();
}

int Player::getScore() const {
    return model::GameManager::instance().getScore();
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
