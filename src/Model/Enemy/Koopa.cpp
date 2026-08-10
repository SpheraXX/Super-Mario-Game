#include "Model/Enemy/Koopa.h"

namespace model {

// The Koopa artwork is 16x23, scaled 2x into world units — about a tile and a half tall,
// which matches the original. The shell frame is 16x16, so entering a shell state shrinks
// this box (see onStomped).
Koopa::Koopa(Vector2 position, bool winged)
    : Enemy(position, {32.0f, 46.0f}),
      state(KoopaState::Walking),
      shellSpeed(SpinSpeed),
      winged(winged) {
    velocity.x = -WalkSpeed;
    setDirection(-1);
}

void Koopa::updateAI(float /* deltaTime */) {
    switch (state) {
        case KoopaState::Walking:
            velocity.x = WalkSpeed * getDirection();
            // Paratroopas bounce along rather than walk. Green ones in the original hop in
            // the player's general direction; only World 7-3 has genuinely flying ones.
            if (winged && isOnGround()) {
                velocity.y = HopSpeed;
            }
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
    // The first stomp on a Paratroopa only costs it the wings: it lands as an ordinary
    // Koopa, and a second stomp is what produces the shell.
    if (winged) {
        winged = false;
        return;
    }

    if (state == KoopaState::Walking) {
        state = KoopaState::ShellIdle;
        velocity.x = 0.0f;

        // The shell art is half the height of the standing Koopa, so the box shrinks with
        // it. Position is the top-left corner, so it has to drop by the difference too —
        // otherwise the shell would hang in the air where the Koopa's head used to be.
        Vector2 sz = getSize();
        const float shrunkBy = sz.y - ShellHeight;
        sz.y = ShellHeight;
        setSize(sz);
        hitbox.height = ShellHeight;

        Vector2 pos = getPosition();
        pos.y += shrunkBy;
        setPosition(pos);
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

bool Koopa::isShell() const {
    return state != KoopaState::Walking;
}

bool Koopa::isWinged() const {
    return winged;
}

void Koopa::onCollision(Entity& other, CollisionType /* side */) {
    if (state == KoopaState::ShellSpinning) {
        // A spinning shell knocks out any other enemy it touches.
        if (&other != this && other.hitbox.layer == CollisionLayer::Enemy) {
            other.onHit(*this);
        }
    }
}

void Koopa::onTileCollision(char /* tile */, CollisionType side) {
    if (side == CollisionType::Left || side == CollisionType::Right) {
        setDirection(-getDirection());
        if (state == KoopaState::Walking) {
            velocity.x = WalkSpeed * getDirection();
        } else if (state == KoopaState::ShellSpinning) {
            velocity.x = shellSpeed * getDirection();
        }
    }
}

}
