#include "Model/Player/Player.h"
#include "Model/Core/GameManager.h"
#include "Model/Core/World.h"
#include "Model/Projectile/MarioFireball.h"

#include <cmath>

#include <SFML/Window/Keyboard.hpp>

namespace model {

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
    // never be cut by a released jump key.
    if (velocity.y >= 0.0f) {
        playerInitiatedJump = false;
    }

    state->update(*this, deltaTime);
    Character::update(deltaTime);
    syncAnimation();

    if (getAnimState() == AnimState::Walk || getAnimState() == AnimState::Run) {
        animationClock += deltaTime;
    } else {
        animationClock = 0.0f;
    }

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

    // Crouch: hold Down/S while grounded and big (Super or Fire). Crouching locks the
    // player in place — no walking, no jumping — and shrinks the box to one tile. The
    // feet stay anchored (syncPowerSize), so the head drops to make room; letting go
    // stands back up. Small Mario has no crouch.
    const bool crouchPressed =
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);
    const bool wantCrouch =
        crouchPressed && isOnGround() && (state->isSuper() || state->isFire());
    if (wantCrouch != crouching) {
        crouching = wantCrouch;
        syncPowerSize();
    }

    if (crouching) {
        horizontalInput = 0;
        velocity.x = 0.0f;
        sprinting = false;
    } else {
        // Sprint: hold Shift to move at run speed.
        const bool sprinting =
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);
        const float currentSpeed = sprinting ? runSpeed : walkSpeed;
        horizontalInput = 0;
        this->sprinting = sprinting;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
            horizontalInput = -1;
            velocity.x = -currentSpeed;
            setDirection(-1);
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
                   sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
            horizontalInput = 1;
            velocity.x = currentSpeed;
            setDirection(1);
        } else {
            velocity.x = 0.0f;
        }
    }

    const bool jumpPressed =
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

    // No jump buffering: a jump fires only on the press edge while grounded, so holding
    // the key does nothing until it is released and pressed again. A crouching player
    // cannot jump (and releasing crouch never auto-jumps: the edge was already consumed).
    if (jumpPressed && !jumpHeld && isOnGround() && !crouching) {
        velocity.y = jumpForce;
        playerInitiatedJump = true;
    }

    // Jump cut: releasing the key mid-ascent kills the upward velocity for a snappy,
    // Mario-like feel (hold to jump higher).
    if (!jumpPressed && playerInitiatedJump && velocity.y < 0.0f) {
        velocity.y = 0.0f;
        playerInitiatedJump = false;
    }

    // Fireball: holding the key re-fires whenever the cooldown clears (1.0s). Works in the
    // air too — only the underlying Fire state may shoot, and a Star wrapped around Fire
    // keeps the ability (StarState forwards canShoot/shoot to its previous state).
    const bool firePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X);
    if (firePressed && state->canShoot() && world) {
        state->shoot();
        // Spawn just in front of the facing side, around mouth height, so the ball never
        // overlaps the player (and never spawns inside the ground).
        const Vector2 pos = getPosition();
        const Vector2 origin{pos.x + (getDirection() > 0 ? getSize().x : -model::MarioFireball::Width),
                             pos.y + getSize().y * 0.3f};
        world->spawn(std::make_unique<model::MarioFireball>(origin, this, getDirection()));
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
        damageCooldown = DamageBlinkTime;
        syncPowerSize();
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
    syncPowerSize();
}

void Player::syncPowerSize() {
    const float targetHeight = crouching ? CrouchHeight
        : ((state->isSuper() || state->isFire()) ? BigHeight : SmallHeight);
    if (getSize().y == targetHeight) return;

    // Anchor the feet: keep the bottom edge fixed while the box grows upward (or shrinks
    // back down). Screen y grows downward, so the top edge moves by the height delta.
    Vector2 pos = getPosition();
    pos.y += getSize().y - targetHeight;
    setPosition(pos);
    setSize({getSize().x, targetHeight});

    // Entity::setSize only touches `size`; the collision hitbox carries its own
    // width/height and would keep colliding with the old box unless we bring it along.
    hitbox.height = targetHeight;
    hitbox.width = getSize().x;
}

bool Player::isBig() const {
    // Crouching drops the box to CrouchHeight, which is still taller than Small — a
    // crouching big Mario is still big.
    return getSize().y > SmallHeight;
}

bool Player::canBreakBricks() const {
    return isBig();
}

PlayerState& Player::getState() {
    return *state;
}

bool Player::isFire() const {
    return state->isFire();
}

bool Player::isStar() const {
    return state->isStar();
}

float Player::getBlinkRemaining() const {
    return damageCooldown;
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

bool Player::isLuigi() const {
    return false;
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

int Player::getHorizontalInput() const {
    return horizontalInput;
}

bool Player::isSprinting() const {
    return sprinting;
}

bool Player::isCrouching() const {
    return crouching;
}

float Player::getAnimationClock() const {
    return animationClock;
}

void Player::syncAnimation() {
    if (!alive) {
        setAnimState(AnimState::Die);
        return;
    }

    const Vector2 vel = getVelocity();
    bool grounded = isOnGround();
    AnimState nextState = AnimState::Idle;

    if (horizontalInput > 0) {
        setFacingRight(true);
    } else if (horizontalInput < 0) {
        setFacingRight(false);
    }

    if (!grounded) {
        nextState = vel.y < 0 ? AnimState::Jump : AnimState::Fall;
    } else if (crouching) {
        nextState = AnimState::Crouch;
    } else if (horizontalInput != 0) {
        nextState = sprinting ? AnimState::Run : AnimState::Walk;
    } else {
        nextState = AnimState::Idle;
    }

    if (nextState != getAnimState()) {
        animationClock = 0.0f;
    }

    setAnimState(nextState);
}

}
