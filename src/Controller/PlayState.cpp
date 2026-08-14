#include "Controller/PlayState.h"

#include "Controller/GameOverState.h"
#include "Controller/LevelCompleteState.h"
#include "Controller/MainMenuState.h"   // ESC → menu (PauseState placeholder)
#include "Controller/StateManager.h"
#include "Model/Core/GameManager.h"
#include "Model/Player/Player.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <iostream>
#include <memory>
#include <string>

namespace controller {

void PlayState::onEnter() {
    scene = std::make_unique<LevelScene>();
    if (!scene->loadLevel()) {
        std::cerr << "PlayState: failed to load level assets\n";
    }

    // Build the screen-space HUD.
    hudRenderer = std::make_unique<view::HudRenderer>();

    levelComplete = false;
}

void PlayState::handleEvent(const sf::Event& event) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        switch (key->code) {
            case sf::Keyboard::Key::Escape:
                // TODO: replace with PauseState in Phase 2.
                // For now, ESC returns directly to the main menu.
                manager->replaceState(std::make_unique<MainMenuState>());
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
        scene->setCinematicActive(true);
        if (scene->player() && scene->flagPole()) {
            sequence.begin(*scene);
        } else {
            finishClear();
        }
    } else if (event == LevelScene::Event::RunEnded) {
        // The player's death fall is over: either the run is over or the level restarts.
        if (model::GameManager::instance().isGameOver()) {
            manager->replaceState(std::make_unique<GameOverState>());
        } else {
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

void PlayState::finishClear() {
    levelComplete = true;
    scene->setCinematicActive(false);
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