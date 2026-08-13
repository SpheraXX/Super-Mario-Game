#include "Model/Enemy/Koopa.h"

namespace model {

// World units: one tile wide, matching the source artwork drawn 1:1. (The placeholder
// enemy sheet draws every enemy at the same size; make this taller only once Koopa gets
// its own taller artwork, or the sprite will be stretched to fit.)
Koopa::Koopa(Vector2 position, bool winged)
    : Enemy(position, {16.0f, StandHeight}),
      state(KoopaState::Walking),
      shellSpeed(SpinSpeed),
      winged(winged),
      flyBaseY(position.y),
      flyingDown(false) {
    velocity.x = -WalkSpeed;
    setDirection(-1);
    // A Paratroopa is airborne from the moment it spawns, so it must not be pulled down
    // by gravity at all — the wings, not the ground, hold it up. loseWings() puts this back.
    if (winged) {
        setGravityScale(0.0f);
    }
}

void Koopa::updateAI(float /* deltaTime */) {
    switch (state) {
        case KoopaState::Walking:
            // A winged Paratroopa flies: it cruises horizontally while patrolling up and
            // down over its spawn altitude. Turning at walls is still handled by
            // onTileCollision, and the ledge-turn in Enemy::update never fires because a
            // flying Koopa is never grounded — which is what lets it cross pits.
            if (winged) {
                velocity.x = FlySpeed * getDirection();
                // Reverse at the top and bottom of the patrol. Bounded rather than
                // integrated, so a frame lost to a ceiling collision cannot make the
                // patrol drift off its anchor.
                const float altitude = getPosition().y;
                if (altitude <= flyBaseY - FlyAmplitude) {
                    flyingDown = true;
                } else if (altitude >= flyBaseY) {
                    flyingDown = false;
                }
                velocity.y = flyingDown ? FlyRiseSpeed : -FlyRiseSpeed;
                break;
            }
            velocity.x = WalkSpeed * getDirection();
            break;
        case KoopaState::ShellIdle:
            velocity.x = 0.0f;
            break;
        case KoopaState::ShellSpinning:
            velocity.x = shellSpeed * getDirection();
            break;
    }
}

void Koopa::onStomped(Entity& player) {
    // The first stomp on a Paratroopa only costs it the wings: it falls out of the sky and
    // lands as an ordinary Koopa, and a second stomp is what produces the shell.
    if (winged) {
        loseWings();
        awardScore();
        return;
    }

    if (state == KoopaState::Walking) {
        state = KoopaState::ShellIdle;
        velocity.x = 0.0f;
        // Retreating into the shell shrinks the box from the standing height down to the
        // shell frame's, and drops it so the shell rests where the feet were rather than
        // hanging at head height.
        Vector2 pos = getPosition();
        pos.y += getSize().y - ShellHeight;
        setPosition(pos);
        hitbox.height = ShellHeight;
        Vector2 sz = getSize();
        sz.y = ShellHeight;
        setSize(sz);
    } else if (state == KoopaState::ShellIdle) {
        state = KoopaState::ShellSpinning;
        // Kick direction based on player relative position
        float playerCenter = player.getPosition().x + player.getSize().x / 2.0f;
        float koopaCenter = getPosition().x + getSize().x / 2.0f;
        setDirection(playerCenter < koopaCenter ? 1 : -1);
        velocity.x = shellSpeed * getDirection();
    } else if (state == KoopaState::ShellSpinning) {
        state = KoopaState::ShellIdle;
        velocity.x = 0.0f;
    }
}

void Koopa::loseWings() {
    winged = false;
    // Hand the body back to gravity. Without restoring the scale the de-winged Koopa would
    // keep the Paratroopa's weightlessness and hang in mid-air, walking on nothing.
    setGravityScale(1.0f);
    // Drop the patrol's climb so the fall starts from rest rather than continuing upward.
    velocity.y = 0.0f;
    velocity.x = WalkSpeed * getDirection();
}

bool Koopa::isShell() const {
    return state != KoopaState::Walking;
}

bool Koopa::isWinged() const {
    return winged;
}

void Koopa::onCollision(Entity& other, CollisionType /* side */) {
    if (state == KoopaState::ShellSpinning) {
        // A spinning shell knocks out any other enemy it touches. The layer guard is the
        // contract for the cast: only Enemy subclasses carry the Enemy layer.
        if (&other != this && other.hitbox.layer == CollisionLayer::Enemy) {
            static_cast<Enemy&>(other).onHit(*this);
        }
    }
}

void Koopa::onTileCollision(char /* tile */, CollisionType side) {
    if (side == CollisionType::Left || side == CollisionType::Right) {
        setDirection(-getDirection());
        if (state == KoopaState::Walking) {
            velocity.x = (winged ? FlySpeed : WalkSpeed) * getDirection();
        } else if (state == KoopaState::ShellSpinning) {
            velocity.x = shellSpeed * getDirection();
        }
        return;
    }

    // Terrain, not just the patrol bounds, can turn a Paratroopa around vertically. Without
    // this a ceiling low enough to stop the climb short of the patrol's top would leave it
    // pressed against that ceiling forever, since the altitude test below never trips.
    if (winged) {
        if (side == CollisionType::Top) {
            flyingDown = true;
        } else if (side == CollisionType::Bottom) {
            flyingDown = false;
        }
    }
}

}
