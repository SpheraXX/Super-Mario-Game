#include "Model/Level/LavaBubble.h"

#include <cmath>

namespace model {

namespace {
constexpr float BubbleSize = 16.0f;
}

LavaBubble::LavaBubble(Vector2 origin, float riseHeight, float leapSeconds,
                       float restSeconds, float phase)
    : Projectile(origin, {BubbleSize, BubbleSize}, nullptr),
      origin(origin),
      riseHeight(riseHeight),
      leapSeconds(leapSeconds),
      restSeconds(restSeconds),
      elapsed(phase) {
    setGravityScale(0.0f);
    update(0.0f);
}

void LavaBubble::update(float deltaTime) {
    elapsed += deltaTime;

    const float period = leapSeconds + restSeconds;
    if (period <= 0.0f) return;

    float t = std::fmod(elapsed, period);
    if (t < 0.0f) t += period;

    if (t >= leapSeconds) {
        // Resting below the surface between leaps: park it a full body under the origin so
        // it is out of sight inside the lava rather than sitting visibly on top of it.
        falling = false;
        setPosition({origin.x, origin.y + BubbleSize});
        return;
    }

    // One symmetric hop: 4h*u*(1-u) peaks at exactly riseHeight when u = 0.5 and is zero at
    // both ends, so the bubble leaves and re-enters the lava at the origin every cycle.
    const float u = t / leapSeconds;
    const float height = 4.0f * riseHeight * u * (1.0f - u);
    falling = (u > 0.5f);
    setPosition({origin.x, origin.y - height});
}

void LavaBubble::onCollision(Entity& other, CollisionType /* side */) {
    if (!isActive || !isTarget(other)) return;
    if (auto* character = dynamic_cast<Character*>(&other)) {
        character->takeDamage(damageValue);
    }
    // Deliberately no expire() — the lava keeps producing it.
}

}
