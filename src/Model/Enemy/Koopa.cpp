#include "Model/Enemy/Koopa.h"

namespace model {

// World units: one tile, matching the 16x16 source artwork scaled 2x. (The placeholder
// enemy sheet draws every enemy at the same size; make this taller only once Koopa gets
// its own taller artwork, or the sprite will be stretched to fit.)
Koopa::Koopa(Vector2 position, bool winged)
    : Enemy(position, {32.0f, 32.0f}),
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
        awardScore();
        return;
    }

    if (state == KoopaState::Walking) {
        state = KoopaState::ShellIdle;
        velocity.x = 0.0f;
        // Shell is the same one-tile box as the walking Koopa while both share the
        // placeholder 16x16 artwork; give it a shorter box once the real shell art lands.
        hitbox.height = 32.0f;
        Vector2 sz = getSize();
        sz.y = 32.0f;
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
            velocity.x = WalkSpeed * getDirection();
        } else if (state == KoopaState::ShellSpinning) {
            velocity.x = shellSpeed * getDirection();
        }
    }
}

}
