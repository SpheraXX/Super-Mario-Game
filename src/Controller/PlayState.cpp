#include "Controller/PlayState.h"

#include "Controller/GameOverState.h"
#include "Controller/LevelCompleteState.h"
#include "Controller/MenuState.h"
#include "Controller/StateManager.h"
#include "Model/Core/GameManager.h"
#include "Model/Level/FlagPole.h"
#include "Model/Player/Player.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

namespace controller {

namespace {
// Level-clear rewards and the scripted clear play (3A: still owned here; the Scene
// owns the completion-zone geometry itself — see LevelScene::castleDoorX).
constexpr int FlagBonus = 5000;
constexpr int TimeBonusPerSecond = 10;

// Duration of the pole-slide segment before Mario walks on.
constexpr float SlideDuration = 0.45f;
// Auto-walk speed on the flat completion zone after the slide.
constexpr float CinematicWalkSpeed = 220.0f;

// TEMP trace instrumentation (removed after playtest).
void trace(const std::string& msg) {
    std::ofstream out("trace_log.txt", std::ios::app);
    out << msg << '\n';
}
}

void PlayState::onEnter() {
    scene = std::make_unique<LevelScene>();
    if (!scene->loadLevel()) {
        std::cerr << "PlayState: failed to load level assets\n";
    }

    // Build the screen-space HUD.
    hudRenderer = std::make_unique<view::HudRenderer>();

    levelComplete = false;
    completionOverlayPushed = false;
    clearPhase = ClearPhase::None;
}

void PlayState::handleEvent(const sf::Event& event) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        switch (key->code) {
            case sf::Keyboard::Key::Escape:
                manager->replaceState(std::make_unique<MenuState>());
                break;
            case sf::Keyboard::Key::G:
                // Debug: kill the player through the normal death flow.
                if (scene->player() && !scene->player()->isDying()) {
                    scene->player()->die(true);
                }
                break;
            case sf::Keyboard::Key::H:
                // Debug: toggle the collision-box overlay.
                scene->toggleHitboxes();
                break;
            default:
                break;
        }
    }
}

void PlayState::update(float deltaTime) {
    // Once the level is complete the game is frozen behind the completion overlay:
    // no timer, no input, no physics.
    if (levelComplete) {
        return;
    }

    // After the flagpole is touched a short scripted clear play keeps updating the
    // tableau (pole slide, walk to the castle) until the overlay is pushed. While it
    // runs the scene is frozen (LevelScene::setCinematicActive).
    if (clearPhase != ClearPhase::None) {
        updateClearSequence(deltaTime);
        return;
    }

    const LevelScene::Event event = scene->update(deltaTime);
    if (event == LevelScene::Event::ClearTriggered) {
        scene->setCinematicActive(true);
        beginLevelClear();
    } else if (event == LevelScene::Event::RunEnded) {
        // The player's death fall is over: either the run is over or the level restarts.
        if (model::GameManager::instance().isGameOver()) {
            manager->replaceState(std::make_unique<GameOverState>());
        } else {
            clearPhase = ClearPhase::None;
            completionOverlayPushed = false;
            scene->resetLevel();
        }
    }

    // HUD snapshot for the next frame.
    auto& game = model::GameManager::instance();
    hudData.score = game.getScore();
    hudData.coins = game.getCoins();
    hudData.levelName = game.getLevelName();
    hudData.time = scene->getRemainingTime();
}

void PlayState::beginLevelClear() {
    if (!scene->flagPole() || !scene->player()) {
        finishLevelClear();
        return;
    }

    // Award the clear bonus immediately (flag + time remaining), like the timer path.
    scene->pauseTimer();
    const int timeBonus = scene->getRemainingTime() * TimeBonusPerSecond;
    model::GameManager::instance().addScore(FlagBonus + timeBonus);
    model::GameManager::instance().setLevelClearBonus(FlagBonus + timeBonus);
    trace("clearBonus bonus=" + std::to_string(FlagBonus + timeBonus));

    // Remember the geometry the sequence animates against.
    const model::Vector2 poleTop = scene->flagPole()->getPosition();
    poleGroundY = poleTop.y + scene->flagPole()->getSize().y;
    poleSlideStartY = scene->player()->getPosition().y;

    poleElapsed = 0.0f;
    clearPhase = ClearPhase::SlideToPole;
}

void PlayState::updateClearSequence(float deltaTime) {
    model::Player* player = scene->player();
    model::FlagPole* flagPole = scene->flagPole();
    if (!player || !flagPole) {
        finishLevelClear();
        return;
    }

    switch (clearPhase) {
        case ClearPhase::SlideToPole: {
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
                clearPhase = ClearPhase::WalkToCastle;
            }
            break;
        }
        case ClearPhase::WalkToCastle: {
            // Auto-walk rightwards on the flat completion zone until the castle door.
            const float marioLeft = player->getPosition().x;
            player->setVelocity({CinematicWalkSpeed, 0.0f});
            player->setFacingRight(true);
            player->update(deltaTime);
            if (marioLeft + player->getSize().x / 2.0f >= scene->castleDoorX()) {
                player->setVelocity({0.0f, 0.0f});
                clearPhase = ClearPhase::ReachedCastle;
            }
            break;
        }
        case ClearPhase::ReachedCastle:
        case ClearPhase::None:
            finishLevelClear();
            break;
    }
}

void PlayState::finishLevelClear() {
    if (completionOverlayPushed) {
        return;
    }
    completionOverlayPushed = true;
    levelComplete = true;
    manager->pushState(std::make_unique<LevelCompleteState>());
}

void PlayState::render(sf::RenderTarget& window) {
    // The scene draws the world (camera, tiles, entities, debug overlay) and restores
    // the fixed view; the HUD is screen-space, on top of everything.
    scene->render(window);

    if (hudRenderer) {
        hudRenderer->render(window, hudData);
    }
}

}