#include "Controller/LevelCompleteState.h"

#include "Controller/AppEngine.h"
#include "Controller/MenuState.h"
#include "Controller/PlayState.h"
#include "Controller/StateManager.h"
#include "Model/Core/GameManager.h"
#include "View/AssetManager.h"
#include "View/TextUtils.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>

#include <memory>
#include <string>

namespace controller {

void LevelCompleteState::handleEvent(const sf::Event& event) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::Space) {
            auto& game = model::GameManager::instance();
            // A finished map with no '; next=' is the last one: back to the menu.
            if (game.getNextMapPath().empty()) {
                manager->clear();
                manager->pushState(std::make_unique<MenuState>());
            } else {
                game.setCurrentMapPath(game.getNextMapPath());
                game.nextLevel();
                manager->clear();
                manager->pushState(std::make_unique<PlayState>());
            }
        }
    }
}

void LevelCompleteState::update(float deltaTime) {
    (void)deltaTime;
}

void LevelCompleteState::render(sf::RenderTarget& window) {
    // Transparent overlay: never clear, the frozen PlayState below stays on screen.
    if (!view::AssetManager::instance().isFontLoaded()) {
        return;
    }
    const sf::Font& font = view::AssetManager::instance().getUiFont();
    const float centerX = static_cast<float>(AppEngine::screenWidth()) / 2.0f;
    const float centerY = static_cast<float>(AppEngine::ScreenHeight);

    sf::Text title(font, "COURSE CLEAR!", titleSize);
    title.setFillColor(sf::Color(255, 230, 90));
    title.setOutlineColor(sf::Color::Black);
    title.setOutlineThickness(2.f);
    view::text::drawCentered(window, title, centerX, centerY * 0.30f);

    const int bonus = model::GameManager::instance().getLevelClearBonus();
    const std::string bonusString = "BONUS: " + std::to_string(bonus);
    const unsigned int fitted = view::text::fitCharacterSize(font, bonusString, static_cast<float>(AppEngine::screenWidth()) * 0.94f, bonusSize);
    sf::Text bonusText(font, bonusString, fitted);
    bonusText.setFillColor(sf::Color::White);
    bonusText.setOutlineColor(sf::Color::Black);
    bonusText.setOutlineThickness(1.f);
    view::text::drawCentered(window, bonusText, centerX, centerY * 0.45f);

    sf::Text hint(font, "PRESS ENTER", hintSize);
    hint.setFillColor(sf::Color(200, 200, 200));
    hint.setOutlineColor(sf::Color::Black);
    hint.setOutlineThickness(1.f);
    view::text::drawCentered(window, hint, centerX, centerY * 0.58f);
}

}