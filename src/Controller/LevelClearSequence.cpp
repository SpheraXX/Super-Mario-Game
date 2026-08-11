#include "Controller/LevelClearSequence.h"

#include "Controller/LevelScene.h"
#include "Model/Core/GameManager.h"
#include "Model/Level/FlagPole.h"
#include "Model/Player/Player.h"

#include <algorithm>
#include <string>

namespace controller {

namespace {
// Level-clear rewards.
constexpr int FlagBonus = 5000;
constexpr int TimeBonusPerSecond = 10;

// Duration of the pole-slide segment before Mario walks on.
constexpr float SlideDuration = 0.45f;
// Auto-walk speed on the flat completion zone after the slide.
constexpr float CinematicWalkSpeed = 220.0f;

}

void LevelClearSequence::begin(LevelScene& scene) {
    scenePtr = &scene;

    // Award the clear bonus immediately (flag + time remaining), like the timer path.
    scene.pauseTimer();
    const int timeBonus = scene.getRemainingTime() * TimeBonusPerSecond;
    model::GameManager::instance().addScore(FlagBonus + timeBonus);
    model::GameManager::instance().setLevelClearBonus(FlagBonus + timeBonus);

    // Remember the geometry the sequence animates against.
    const model::Vector2 poleTop = scene.flagPole()->getPosition();
    poleGroundY = poleTop.y + scene.flagPole()->getSize().y;
    poleSlideStartY = scene.player()->getPosition().y;

    poleElapsed = 0.0f;
    phase = Phase::SlideToPole;
    active = true;
    finished = false;
}

void LevelClearSequence::update(float deltaTime) {
    if (!active || finished || !scenePtr) {
        return;
    }

    model::Player* player = scenePtr->player();
    model::FlagPole* flagPole = scenePtr->flagPole();
    if (!player || !flagPole) {
        finished = true;
        return;
    }

    switch (phase) {
        case Phase::SlideToPole: {
            poleElapsed += deltaTime;
            const float progress = std::clamp(poleElapsed / SlideDuration, 0.0f, 1.0f);
            flagPole->setSlideProgress(progress);

            // Mario hugs the pole and slides with the pennant from where he touched it
            // down to the ground. The horizontal position is fixed to the pole's column
            // (centred), the vertical lerps to the flat completion-zone ground.
            const float poleX = static_cast<float>(flagPole->getPosition().x);
            const float targetY = poleGroundY - player->getSize().y;
            player->setPosition({
                poleX + (flagPole->getSize().x - player->getSize().x) / 2.0f,
                poleSlideStartY + (targetY - poleSlideStartY) * progress});
            player->setVelocity({0.0f, 0.0f});
            player->isGrounded = true;
            player->setFacingRight(true);

            if (progress >= 1.0f) {
                phase = Phase::WalkToCastle;
            }
            break;
        }
        case Phase::WalkToCastle: {
            // Auto-walk rightwards on the flat completion zone until the castle door.
            const float marioLeft = player->getPosition().x;
            player->setVelocity({CinematicWalkSpeed, 0.0f});
            player->setFacingRight(true);
            player->update(deltaTime);
            if (marioLeft + player->getSize().x / 2.0f >= scenePtr->castleDoorX()) {
                player->setVelocity({0.0f, 0.0f});
                phase = Phase::ReachedCastle;
            }
            break;
        }
        case Phase::ReachedCastle:
            finished = true;
            break;
    }
}

bool LevelClearSequence::isActive() const {
    return active;
}

bool LevelClearSequence::isFinished() const {
    return finished;
}

}