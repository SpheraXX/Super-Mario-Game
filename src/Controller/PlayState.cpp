#include "Controller/PlayState.h"

#include "Controller/GameOverState.h"
#include "Controller/IAudioManager.h"
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
#include "Model/Save/ProfileManager.h"
#include "Model/World/WorldType.h"
#include "Model/Core/WorldManager.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <iostream>
#include <memory>
#include <algorithm>
#include <string>

namespace controller {

namespace {
// Level-clear rewards: a flat bonus for reaching the goal, plus time remaining.
constexpr int GoalBonus = 5000;
constexpr int TimeBonusPerSecond = 10;
// How long to wait after death before transitioning (lets 'lost_a_life' theme finish).
constexpr float DeathMusicDelay = 2.0f;
// Audio track ID for the Starman/Invincibility theme, must match audio_meta.json.
constexpr const char* StarmanTrackId = "starman";
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

        if (!scene->loadLevel(&savedState->level, &savedState->player, true)) {
            model::LogManager::instance().error("PlayState: failed to load level assets from save");
        }
        savedState.reset();
        hasSavedState = false;
    } else {
        model::GameSaveData currentSave;
        if (model::SaveManager::instance().load(currentSave)) {
            if (!scene->loadLevel(nullptr, &currentSave.player, false)) {
                model::LogManager::instance().error("PlayState: failed to load level assets");
            }
        } else {
            if (!scene->loadLevel()) {
                model::LogManager::instance().error("PlayState: failed to load level assets");
            }
        }
    }

    // Build the screen-space HUD.
    hudRenderer = std::make_unique<view::HudRenderer>();

    levelComplete = false;

    // Snapshot score and coins at level entry — restored on death so dying doesn't
    // permanently cost progress accumulated during previous levels.
    checkpointScore = model::GameManager::instance().getScore();
    checkpointCoins = model::GameManager::instance().getCoins();

    // Start the world-appropriate background theme.
    m_lastKnownWorldType = scene->getWorldType();
    playWorldMusic();

    // Cache level progress and high scores from disk so captureSaveData() doesn't
    // need to re-read the file each time it's called (e.g. on pause-save).
    m_cachedSaveBase = model::GameSaveData{};
    model::SaveManager::instance().load(m_cachedSaveBase);
}

void PlayState::handleEvent(const sf::Event& event) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        const auto& settings = model::SettingsManager::instance().get();
        if (static_cast<int>(key->code) == settings.keyPause ||
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

    if (deathDelayTimer > 0.0f) {
        deathDelayTimer -= deltaTime;
        if (deathDelayTimer <= 0.0f) {
            if (model::GameManager::instance().isGameOver()) {
                auto restartCb = [m = this->manager]() {
                    auto& gm = model::GameManager::instance();
                    // reset() always sends currentMapPath back to the campaign's default
                    // map (arcade-style "game over sends you back to World 1") -- correct
                    // for campaign play, but a custom map should restart itself instead.
                    const bool wasCustom = gm.isCustomMapSession();
                    const std::string customPath = gm.getCurrentMapPath();
                    const std::string customName = gm.getLevelName();
                    gm.reset();
                    if (wasCustom) {
                        gm.setCurrentMapPath(customPath);
                        gm.setLevelName(customName);
                        gm.setCustomMapSession(true);
                    }
                    m->replaceState(std::make_unique<PlayState>());
                };
                manager->replaceState(std::make_unique<GameOverState>(std::move(restartCb)));
            } else {
                model::LogManager::instance().info("Player respawn");
                // Restore score and coins to the start of this level.
                model::GameManager::instance().setScore(checkpointScore);
                model::GameManager::instance().setCoins(checkpointCoins);
                scene->restartLevel();
                // Restart the world theme music from the beginning on respawn.
                playWorldMusic();
            }
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
        // The player's death fall is over. Instead of transitioning immediately,
        // we wait 2 seconds to let the 'lost_a_life' music finish.
        deathDelayTimer = DeathMusicDelay;
        return;
    }

    // Handle Starman music
    if (auto* player = scene->player()) {
        if (player->isStar() && !playingStarmanMusic) {
            playingStarmanMusic = true;
            if (context && context->audio) {
                context->audio->stopMusic();
                context->audio->playMusic(StarmanTrackId);
            }
        } else if (!player->isStar() && playingStarmanMusic) {
            playingStarmanMusic = false;
            playWorldMusic();
        }
    }

    // Handle area (world type) transitions (e.g. entering a pipe)
    if (scene->getWorldType() != m_lastKnownWorldType) {
        m_lastKnownWorldType = scene->getWorldType();
        // Only change music if we aren't currently overriding it with Starman
        if (!playingStarmanMusic) {
            playWorldMusic();
        }
    }

    // HUD snapshot for the next frame.
    auto& game = model::GameManager::instance();
    hudData.score = game.getScore();
    hudData.coins = game.getCoins();
    hudData.lives = game.getLives();
    hudData.levelName = game.getLevelName();
    hudData.time = scene->getRemainingTime();
}

void PlayState::playWorldMusic() {
    if (!context || !context->audio) return;
    // Map WorldType to the audio track IDs registered in audio_meta.json.
    std::string trackId;
    switch (scene->getWorldType()) {
        case model::WorldType::Underground: trackId = "underground"; break;
        case model::WorldType::Underwater:  trackId = "underwater";  break;
        case model::WorldType::Castle:      trackId = "castle";      break;
        case model::WorldType::Overworld:
        default:                            trackId = "overworld";   break;
    }
    // Force restart so respawns replay the theme from the top.
    context->audio->stopMusic();
    context->audio->playMusic(trackId);
}

void PlayState::updateProgressAndUnlocks(model::GameSaveData& data) {
    std::string currentLevelId = model::GameManager::instance().getLevelName();

    // High Score logic (Task 3): only update if this run scored higher
    auto it = data.high_scores.find(currentLevelId);
    int scoreGainedThisLevel = model::GameManager::instance().getScore() - checkpointScore;
    if (it == data.high_scores.end() || scoreGainedThisLevel > it->second) {
        data.high_scores[currentLevelId] = scoreGainedThisLevel;
    }

    // Level progress (Task 5): mark as passed, track first-time clear
    m_isFirstTimeClear = (data.level_progress[currentLevelId] != "pass");
    if (m_isFirstTimeClear) {
        data.level_progress[currentLevelId] = "pass";
    }

    // Data-driven unlocking (Task 6): scan WorldManager for levels requiring this level
    for (const auto& w : model::WorldManager::instance().getWorlds()) {
        for (const auto& l : w.levels) {
            if (l.unlockRequires == currentLevelId) {
                if (data.level_progress[l.id] != "pass") {
                    data.level_progress[l.id] = "available";
                }
                // Unlock the parent world if not already
                if (std::find(data.unlocked_worlds.begin(),
                              data.unlocked_worlds.end(), w.id)
                    == data.unlocked_worlds.end()) {
                    data.unlocked_worlds.push_back(w.id);
                }
            }
        }
    }
}

void PlayState::syncProfileStats(const model::GameSaveData& data) {
    auto& pm = model::ProfileManager::instance();
    int activeIdx = pm.getActiveProfileIndex();
    if (activeIdx < 0 || activeIdx >= 4) return;

    auto p = pm.getProfiles()[activeIdx];

    int totalHighScore = 0;
    for (const auto& kv : data.high_scores) {
        totalHighScore += kv.second;
    }
    p.total_score = totalHighScore;

    if (m_isFirstTimeClear) {
        p.passed_levels += 1;
    }
    pm.updateProfile(activeIdx, p);
}

void PlayState::finishClear() {
    // Award the clear bonus (a flat reward for reaching the goal, plus time remaining)
    // before the timer stops.
    scene->pauseTimer();
    const int timeBonus = scene->getRemainingTime() * TimeBonusPerSecond;
    model::GameManager::instance().addScore(GoalBonus + timeBonus);
    model::GameManager::instance().setLevelClearBonus(GoalBonus + timeBonus);

    // Update checkpoint so death on later levels doesn't lose post-clear progress.
    checkpointScore = model::GameManager::instance().getScore();
    checkpointCoins = model::GameManager::instance().getCoins();

    levelComplete = true;
    scene->setCinematicActive(false);

    // Build save data and apply progress/unlock logic
    model::GameSaveData data = captureSaveData();
    updateProgressAndUnlocks(data);
    model::SaveManager::instance().save(data);
    syncProfileStats(data);

    model::LogManager::instance().info(
        "Level end: " + model::GameManager::instance().getLevelName()
    );
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
    // Start from the cached base (loaded in onEnter) to preserve level_progress
    // and high_scores without re-reading from disk each time.
    model::GameSaveData data = m_cachedSaveBase;

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