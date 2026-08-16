#include "Model/Player/Player.h"

#include "Model/Core/World.h"
#include "Model/Projectile/MarioFireball.h"
#include "Model/Core/GameManager.h"

#include <cmath>
#include <string>

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
    // never be boosted by a held jump key.
    if (velocity.y >= 0.0f) {
        if (playerInitiatedJump) {
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

    // Advance the walk cycle by ground covered rather than by elapsed time. Driving it off
    // distance means the legs speed up on their own when running and stop dead when the
    // player is pushed against a wall — no second frame rate, and no cycle running on the
    // spot. Only horizontal motion counts; a falling player is showing the jump pose anyway.
    walkCycleDistance += std::fabs(getVelocity().x) * deltaTime;

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

void Player::setInputMapper(IInputMapper* mapper) {
    inputMapper = mapper;
}

void Player::handleInput(float deltaTime) {
    if (!alive) return;

    // Movement tuning is polymorphic: each concrete character reports its own numbers.
    const float walkSpeed = getWalkSpeed();
    const float runSpeed = getRunSpeed();

    // Sprint: hold Shift or the mapped Run key to move at run speed.
    const bool sprinting =
        (inputMapper && inputMapper->isActionPressed(model::InputAction::Run)) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);
    const float targetSpeed = sprinting ? runSpeed : walkSpeed;

    // Horizontal inertia: accelerate toward the input target (friction when idle). The
    // velocity is never snapped, so movement feels weighty and the physics stays continuous.
    bool movingLeft = (inputMapper && inputMapper->isActionPressed(model::InputAction::MoveLeft)) ||
                      sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
                      sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);
    bool movingRight = (inputMapper && inputMapper->isActionPressed(model::InputAction::MoveRight)) ||
                       sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
                       sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);
    inputMoving = movingLeft || movingRight;
    // Down enters pipes: kept edge-free (held state) so the play state can warp while
    // the player stands on the cap. Pipe entry is a hold action in SMB, not a press.
    inputDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);
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
        (inputMapper && inputMapper->isActionPressed(model::InputAction::Jump)) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

    // Press edge: remember the intent briefly (jump buffering).
    if (jumpPressed && !jumpHeld) {
        jumpBufferTime = JumpBufferTime;
    }

    // Fire while the press is fresh and we are grounded or still inside the coyote window.
    if (jumpBufferTime > 0.0f && coyoteTime > 0.0f) {
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
                velocity.y = -getMaxJumpSpeed();
            }
            jumpHoldTime += deltaTime;
        }
    }

    // Underwater swim (simplified): holding jump pushes the player upward continuously,
    // no ground/coyote requirement. Weaker than a land jump and capped low, so the
    // player drifts up slowly instead of leaping out of the water.
    if (isUnderwater() && jumpPressed) {
        velocity.y -= SwimAccel * deltaTime;
        if (velocity.y < -SwimMaxSpeed) {
            velocity.y = -SwimMaxSpeed;
        }
    }

    // Fireball: holding the key re-fires whenever the cooldown clears. Works in the air
    // too — only the underlying Fire state may shoot, and a Star wrapped around Fire keeps
    // the ability (StarState forwards canShoot/shoot to its previous state).
    const bool firePressed = (inputMapper && inputMapper->isActionPressed(model::InputAction::Run)) ||
                             sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X);
    if (firePressed && state->canShoot() && world) {
        state->shoot();
        // Spawn just in front of the facing side, around mouth height, so the ball never
        // overlaps the player (and never spawns inside the ground).
        const Vector2 pos = getPosition();
        const Vector2 origin{pos.x + (getDirection() > 0 ? getSize().x : -MarioFireball::Width),
                             pos.y + getSize().y * 0.3f};
        world->spawn(std::make_unique<MarioFireball>(origin, this, getDirection()));
    }

    jumpHeld = jumpPressed;
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
        // Bring the BODY down with the state. Powering up goes through setState(), which
        // syncs the size itself, but this path swaps the state directly and so has to do
        // it too — without this a damaged Super Mario keeps the two-tile box: he still
        // draws as big (the renderer picks the frame off the size), still collides as big,
        // and still breaks bricks, while the state underneath says Small.
        syncPowerSize();
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
    syncPowerSize();
}

void Player::syncPowerSize() {
    const float targetHeight = (state->isSuper() || state->isFire()) ? BigHeight : SmallHeight;
    if (getSize().y == targetHeight) return;

    // Anchor the feet: keep the bottom edge fixed while the box grows upward (or shrinks
    // back down). Screen y grows downward, so the top edge moves by the height delta.
    Vector2 pos = getPosition();
    pos.y += getSize().y - targetHeight;
    setPosition(pos);
    setSize({getSize().x, targetHeight});

    // Entity::setSize only touches `size`; the collision hitbox carries its own height and
    // would keep colliding with the old box unless we bring it along.
    //
    // Only the HEIGHT changes. The width and x-offset are deliberately narrower than the
    // sprite (see Mario's constructor: a 16-wide box inset by 8 inside a 32-wide sprite) so
    // side grazes are as forgiving as the artwork suggests. Overwriting them with the full
    // sprite width — as an earlier version of this did — silently widened the player's box
    // by 100% on the first power-up and made every near-miss a hit.
    hitbox.height = targetHeight;
}

bool Player::isFire() const {
    return state->isFire();
}

bool Player::isStar() const {
    return state->isStar();
}

bool Player::isBig() const {
    // Crouching drops the box to CrouchHeight, which is still taller than Small — a
    // crouching big Mario is still big.
    return getSize().y > SmallHeight;
}

bool Player::canBreakBricks() const {
    return isBig();
}

float Player::getWalkCycleDistance() const {
    return walkCycleDistance;
}

float Player::getBlinkRemaining() const {
    return damageCooldown;
}

bool Player::isLuigi() const {
    return false;
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

void Player::addCoin() {
    model::GameManager::instance().addCoin();
}

void Player::addLife() {
    model::GameManager::instance().addLife();
}

int Player::getScore() const {
    return model::GameManager::instance().getScore();
}

int Player::getCoins() const {
    return model::GameManager::instance().getCoins();
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

    const float speedX = std::abs(vel.x);
    if (!grounded) {
        setAnimState(vel.y < 0 ? AnimState::Jump : AnimState::Fall);
    } else {
        // Hysteresis so the sprite never flaps between Idle and Walk. Entering motion
        // needs a real horizontal push (>IdleSpeed); leaving it only below a lower bar —
        // and holding the run direction (inputMoving, e.g. pushing against a wall) keeps
        // the walk pose even when the push-out has just zeroed the velocity.
        const AnimState current = getAnimState();
        const bool walking = (speedX > IdleSpeedThreshold)
                             || ((current == AnimState::Walk || current == AnimState::Run)
                                 && speedX > StoppedSpeedThreshold)
                             || ((current == AnimState::Walk || current == AnimState::Run)
                                 && inputMoving && speedX > 0.0f);
        if (walking) {
            setAnimState(speedX > RunSpeedThreshold ? AnimState::Run : AnimState::Walk);
        } else {
            setAnimState(AnimState::Idle);
        }
    }
}

}
