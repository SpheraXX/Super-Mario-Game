#include "Controller/PlayState.h"

#include "Controller/GameOverState.h"
#include "Controller/LevelCompleteState.h"
#include "Controller/PauseState.h"
#include "Controller/MainMenuState.h"
#include "Controller/StateManager.h"
#include "Model/SettingsManager.h"
#include "Model/Core/GameManager.h"
#include "Model/Core/LogManager.h"
#include "Model/Player/Player.h"
#include "Model/Save/SaveData.h"
#include "Model/Save/SaveManager.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <iostream>
#include <memory>
#include <string>

namespace controller {

namespace {
// Level-clear rewards: a flat bonus for reaching the goal, plus time remaining.
constexpr int GoalBonus = 5000;
constexpr int TimeBonusPerSecond = 10;
}

PlayState::PlayState() : hasSavedState(false) {}

PlayState::PlayState(const model::GameSaveData& save)
    : hasSavedState(true), savedState(std::make_unique<model::GameSaveData>(save)) {}

PlayState::~PlayState() = default;

void PlayState::onEnter() {
    scene = std::make_unique<LevelScene>();
    if (context && context->input) {
        scene->setInputMapper(context->input);
    }

    if (hasSavedState && savedState) {
        auto& game = model::GameManager::instance();
        game.setScore(savedState->score);
        game.setLives(savedState->lives);
        game.setCoins(savedState->coins);
        game.setCurrentLevel(savedState->level.currentLevel);
        game.setCurrentMapPath(savedState->level.mapPath);
        if (!savedState->level.nextMapPath.empty()) {
            game.setNextMapPath(savedState->level.nextMapPath);
        }
        if (!savedState->level.levelName.empty()) {
            game.setLevelName(savedState->level.levelName);
        }

        if (!scene->loadLevel(&savedState->level, &savedState->player)) {
            model::LogManager::instance().error("PlayState: failed to load level assets from save");
        }
        savedState.reset();
        hasSavedState = false;
    } else {
        if (!scene->loadLevel()) {
            model::LogManager::instance().error("PlayState: failed to load level assets");
        }
    }

    // Build the screen-space HUD.
    hudRenderer = std::make_unique<view::HudRenderer>();

    levelComplete = false;
}

void PlayState::handleEvent(const sf::Event& event) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        const auto& settings = model::SettingsManager::instance().get();
        if (static_cast<int>(key->code) == settings.keyPause ||
            static_cast<int>(key->code) == settings.keyBack ||
            key->code == sf::Keyboard::Key::Escape) {
            model::LogManager::instance().info("Pause");
            manager->pushState(std::make_unique<PauseState>(
                [this]() {
                    this->saveGame();
                },
                [this]() {
                    if (scene) {
                        model::LogManager::instance().info("Player respawn");
                        scene->restartLevel();
                    }
                }
            ));
            return;
        }

        switch (key->code) {
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

    // After the flagpole is touched the scripted clear play keeps updating the frozen
    // tableau (pole slide, walk to the castle) until the overlay is pushed.
    if (sequence.isActive()) {
        sequence.update(deltaTime);
        if (sequence.isFinished()) {
            finishClear();
        }
        return;
    }

    const LevelScene::Event event = scene->update(deltaTime);
    if (event == LevelScene::Event::ClearTriggered) {
        // Freeze the world and start the scripted clear play; without a live player or
        // pole there is nothing to animate, so jump straight to the overlay.
        if (scene->player() && scene->flagPole()) {
            scene->setCinematicActive(true);
            sequence.begin(*scene);
        } else {
            finishClear();
        }
    } else if (event == LevelScene::Event::RunEnded) {
        // The player's death fall is over: either the run is over or the whole level
        // restarts from its first area (whatever area the body fell in).
        if (model::GameManager::instance().isGameOver()) {
            manager->replaceState(std::make_unique<GameOverState>());
        } else {
            model::LogManager::instance().info("Player respawn");
            scene->restartLevel();
        }
    }

    // HUD snapshot for the next frame.
    auto& game = model::GameManager::instance();
    hudData.score = game.getScore();
    hudData.coins = game.getCoins();
    hudData.levelName = game.getLevelName();
    hudData.time = scene->getRemainingTime();
}

void PlayState::finishClear() {
    // Award the clear bonus (a flat reward for reaching the goal, plus time remaining)
    // before the timer stops.
    scene->pauseTimer();
    const int timeBonus = scene->getRemainingTime() * TimeBonusPerSecond;
    model::GameManager::instance().addScore(GoalBonus + timeBonus);
    model::GameManager::instance().setLevelClearBonus(GoalBonus + timeBonus);

    levelComplete = true;
    scene->setCinematicActive(false);
    model::LogManager::instance().info("Level end: " + model::GameManager::instance().getLevelName());
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

model::GameSaveData PlayState::captureSaveData() const {
    model::GameSaveData data;
    auto& game = model::GameManager::instance();
    data.score = game.getScore();
    data.lives = game.getLives();
    data.coins = game.getCoins();
    data.version = 1;

    data.level.currentLevel = game.getCurrentLevel();
    data.level.mapPath = game.getCurrentMapPath();
    data.level.nextMapPath = game.getNextMapPath();
    data.level.levelName = game.getLevelName();

    if (scene) {
        scene->captureLevelSaveData(data.level);

        if (const auto* player = scene->player()) {
            data.player.posX = player->getPosition().x;
            data.player.posY = player->getPosition().y;
            data.player.velX = player->getVelocity().x;
            data.player.velY = player->getVelocity().y;
            data.player.isBig = player->isBig();
            data.player.starDuration = player->getStarDuration();
            data.player.facingDirection = player->getDirection();
            data.player.isLuigi = player->isLuigi();

            if (player->isStar()) {
                data.player.power = "Star";
            } else if (player->isFire()) {
                data.player.power = "Fire";
            } else {
                data.player.power = "None";
            }
        }
    }

    return data;
}

void PlayState::saveGame() const {
    model::GameSaveData data = captureSaveData();
    model::SaveManager::instance().save(data);
}

}