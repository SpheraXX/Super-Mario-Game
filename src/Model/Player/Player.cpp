#include "Model/Player/Player.h"
#include "Model/Core/GameManager.h"

#include <cmath>
#include <fstream>
#include <string>

#include <SFML/Window/Keyboard.hpp>

namespace model {

namespace {
// TEMP trace instrumentation (removed after playtest).
void trace(const std::string& msg) {
    std::ofstream out("trace_log.txt", std::ios::app);
    out << msg << '\n';
}
}

Player::Player(Vector2 position, Vector2 size)
    : Character(position, size),
      state(std::make_unique<SmallState>()),
      score(0),
      coins(0),
      damageCooldown(0.0f) {
    // The collision layer drives the (type-check-free) routing in CollisionManager:
    // exactly one entity per pair must be the player for the pair to resolve.
    hitbox.layer = CollisionLayer::Player;
}

Player::~Player() = default;

void Player::update(float deltaTime) {
    // A player-initiated ascent only owns the rising half of the jump: once the apex is
    // passed the flag is cleared, so a stomp bounce (which happens while falling) can
    // never be boosted by a held jump key.
    if (velocity.y >= 0.0f) {
        if (playerInitiatedJump) {
            trace("apex y=" + std::to_string(getPosition().y)
                  + " vy=" + std::to_string(velocity.y));
        }
        playerInitiatedJump = false;
    }

    // Forgiveness timers: the coyote window stays full while grounded and drains in the
    // air; the jump buffer drains until it is spent (or consumed by a jump).
    if (isOnGround()) {
        coyoteTime = CoyoteTime;
    } else {
        coyoteTime = std::max(0.0f, coyoteTime - deltaTime);
    }
    jumpBufferTime = std::max(0.0f, jumpBufferTime - deltaTime);

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

void Player::handleInput(float deltaTime) {
    if (!alive) return;

    // Movement tuning is polymorphic: each concrete character reports its own numbers.
    const float walkSpeed = getWalkSpeed();
    const float runSpeed = getRunSpeed();

    // Sprint: hold Shift to move at run speed.
    const bool sprinting =
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);
    const float targetSpeed = sprinting ? runSpeed : walkSpeed;

    // Horizontal inertia: accelerate toward the input target (friction when idle). The
    // velocity is never snapped, so movement feels weighty and the physics stays continuous.
    bool movingLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
                      sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);
    bool movingRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
                       sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);
    if (movingLeft) {
        setDirection(-1);
    } else if (movingRight) {
        setDirection(1);
    }

    const float targetX = movingLeft ? -targetSpeed : (movingRight ? targetSpeed : 0.0f);
    if (targetX == 0.0f) {
        // No input: coast to a stop.
        if (velocity.x > 0.0f) {
            velocity.x = std::max(0.0f, velocity.x - Friction * deltaTime);
        } else if (velocity.x < 0.0f) {
            velocity.x = std::min(0.0f, velocity.x + Friction * deltaTime);
        }
    } else {
        const float rate = (isOnGround() ? GroundAccel : AirAccel) * deltaTime;
        if (velocity.x < targetX) {
            velocity.x = std::min(targetX, velocity.x + rate);
        } else if (velocity.x > targetX) {
            velocity.x = std::max(targetX, velocity.x - rate);
        }
    }

    const bool jumpPressed =
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

    // Press edge: remember the intent briefly (jump buffering).
    if (jumpPressed && !jumpHeld) {
        jumpBufferTime = JumpBufferTime;
    }

    // Fire while the press is fresh and we are grounded or still inside the coyote window.
    if (jumpBufferTime > 0.0f && coyoteTime > 0.0f) {
        trace("jumpFire coyote=" + std::to_string(coyoteTime)
              + " buffer=" + std::to_string(jumpBufferTime));
        velocity.y = JumpInitialSpeed;
        playerInitiatedJump = true;
        jumpHoldTime = 0.0f;
        jumpBufferTime = 0.0f;
        coyoteTime = 0.0f;
    }

    // Continuous jump: holding the key adds upward acceleration on top of gravity for as
    // long as the ascent is player-initiated and rising, capped at the character's max
    // jump speed (Mario: exactly four tiles for a full-length hold). Releasing simply
    // stops the boost — velocity is never zeroed.
    if (jumpPressed && playerInitiatedJump && velocity.y < 0.0f) {
        if (jumpHoldTime < MaxJumpHoldTime) {
            velocity.y -= getJumpAccel() * deltaTime;
            if (velocity.y < -getMaxJumpSpeed()) {
                trace("cap vy=" + std::to_string(velocity.y));
                velocity.y = -getMaxJumpSpeed();
            }
            jumpHoldTime += deltaTime;
        }
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
    trace("playerDied lives=" + std::to_string(getLives()));
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
