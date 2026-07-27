#include "Controller/PlayState.h"

#include "Controller/GameOverState.h"
#include "Controller/MenuState.h"
#include "Controller/StateManager.h"
#include "Model/GameManager.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>

#include <exception>
#include <iostream>
#include <memory>
#include <string>

namespace controller {

namespace {
// Issue 4 will replace this with a real level-index -> file mapping. For Phase 1 every
// level resolves to the single sample map so PlayState always has something to show.
std::string mapPathForLevel(int level) {
    (void)level;
    return "assets/maps/plain.map";
}
}

void PlayState::onEnter() {
    const int level = model::GameManager::instance().getCurrentLevel();
    try {
        map.loadFromFile(mapPathForLevel(level));
        renderer = std::make_unique<view::TileMapRenderer>("assets/blocks.png");
        mapLoaded = true;
    } catch (const std::exception& error) {
        // Fail soft: keep running with a blank field rather than crashing the whole app.
        std::cerr << "PlayState: failed to load level assets: " << error.what() << '\n';
        mapLoaded = false;
    }

    fontLoaded = font.openFromFile("assets/fonts/Tuffy.ttf");
}

void PlayState::handleEvent(const sf::Event& event) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        switch (key->code) {
            case sf::Keyboard::Key::Escape:
                // Abandon the run and return to the menu.
                manager->replaceState(std::make_unique<MenuState>());
                break;
            case sf::Keyboard::Key::G:
                // TEMPORARY debug hook to exercise the game-over transition until real
                // death logic (Issue 3/5) exists. Awards points, then ends the run.
                model::GameManager::instance().addScore(500);
                model::GameManager::instance().loseLife();
                model::GameManager::instance().loseLife();
                model::GameManager::instance().loseLife();
                break;
            default:
                break;
        }
    }
}

void PlayState::update(float deltaTime) {
    (void)deltaTime;
    // SEAM: step the World/physics and update the Player here (Issues 3/5).

    if (model::GameManager::instance().isGameOver()) {
        manager->replaceState(std::make_unique<GameOverState>());
    }
}

void PlayState::render(sf::RenderWindow& window) {
    const sf::Color skyBlue(92, 148, 252);
    window.clear(skyBlue);

    if (mapLoaded && renderer) {
        renderer->render(window, map);
    }
    // SEAM: draw World entities / Player / HUD here (Issues 2/3/5).

    if (fontLoaded) {
        const int level = model::GameManager::instance().getCurrentLevel();

        sf::Text levelLabel(font, "LEVEL " + std::to_string(level), 22);
        levelLabel.setFillColor(sf::Color::White);
        levelLabel.setOutlineColor(sf::Color::Black);
        levelLabel.setOutlineThickness(2.f);
        levelLabel.setPosition({10.f, 8.f});
        window.draw(levelLabel);

        sf::Text hint(font, "ESC: Menu | G: Debug Game Over", 16);
        hint.setFillColor(sf::Color::White);
        hint.setOutlineColor(sf::Color::Black);
        hint.setOutlineThickness(2.f);
        hint.setPosition({10.f, window.getSize().y - 26.f});
        window.draw(hint);
    }
}

}
