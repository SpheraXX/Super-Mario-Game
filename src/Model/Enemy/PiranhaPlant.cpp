#include "Model/Enemy/PiranhaPlant.h"

#include "Model/Map/TileMap.h"

namespace model {

PiranhaPlant::PiranhaPlant(Vector2 pipeMouthTopLeft)
    : Enemy({pipeMouthTopLeft.x + TileMap::TileWidth - Width / 2.0f, pipeMouthTopLeft.y},
            {Width, Height}),
      phase(Phase::Hidden),
      phaseTimer(HiddenTime),
      mouthY(pipeMouthTopLeft.y) {
    // Nothing about this enemy is physical: no falling, no drifting. updateAI writes the
    // position outright, and a zero velocity means Character's integration is a no-op.
    setGravityScale(0.0f);
    velocity = {0.0f, 0.0f};
}

float PiranhaPlant::extension() const {
    switch (phase) {
        case Phase::Hidden:     return 0.0f;
        case Phase::Extended:   return 1.0f;
        // phaseTimer counts down, so the fraction elapsed is 1 - timer/duration.
        case Phase::Rising:     return 1.0f - phaseTimer / RiseTime;
        case Phase::Retracting: return phaseTimer / RetractTime;
    }
    return 0.0f;
}

void PiranhaPlant::advancePhase() {
    switch (phase) {
        case Phase::Hidden:     phase = Phase::Rising;     phaseTimer = RiseTime;     break;
        case Phase::Rising:     phase = Phase::Extended;   phaseTimer = ExtendedTime; break;
        case Phase::Extended:   phase = Phase::Retracting; phaseTimer = RetractTime;  break;
        case Phase::Retracting: phase = Phase::Hidden;     phaseTimer = HiddenTime;   break;
    }
}

void PiranhaPlant::updateAI(float deltaTime) {
    phaseTimer -= deltaTime;
    if (phaseTimer <= 0.0f) {
        advancePhase();
    }

    // Fully down puts the plant's top exactly on the mouth line, so its whole body sits
    // inside the pipe and the pipe tiles (drawn after it) cover it. Fully up lifts it clear
    // by its own height. Any pipe at least two cells tall hides it completely.
    Vector2 position = getPosition();
    position.y = mouthY - extension() * Height;
    setPosition(position);
}

}
