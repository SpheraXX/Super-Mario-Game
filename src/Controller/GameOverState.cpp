#include "Controller/GameOverState.h"

#include "Controller/AppEngine.h"
#include "Controller/MainMenuState.h"
#include "Controller/StateManager.h"
#include "Model/Core/GameManager.h"
#include "View/AssetManager.h"
#include "View/TextUtils.h"
#include "Controller/IAudioManager.h"
#include "Model/SettingsManager.h"
#include "Controller/InputMapper.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>

#include <memory>
#include <string>

namespace controller {

void GameOverState::onEnter() {
    if (context && context->audio) {
        context->audio->playMusic("game_over");
    }

    font = view::AssetManager::instance().getUiFont();
    fontLoaded = view::AssetManager::instance().isFontLoaded();
    if (fontLoaded) {
        const float fitWidth = static_cast<float>(AppEngine::screenWidth()) * 0.94f;
        titleSize = view::text::fitCharacterSize(font, "GAME OVER", fitWidth, 28);
        
        std::string backKeyName = InputMapper::getKeyName(model::SettingsManager::instance().get().keyBack);
        std::string hintStr = "Press ENTER or " + backKeyName + " to return to Menu";
        hintSize = view::text::fitCharacterSize(font, hintStr, fitWidth, 10);
    }
}

void GameOverState::handleEvent(const sf::Event& event) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Enter ||
            static_cast<int>(key->code) == model::SettingsManager::instance().get().keyBack) {
            // Transition to the new MainMenuState (not the old MenuState).
            manager->replaceState(std::make_unique<MainMenuState>());
        }
    }
}

void GameOverState::update(float deltaTime) {
    (void)deltaTime;
}

void GameOverState::render(sf::RenderTarget& window) {
    window.clear(sf::Color(60, 10, 10));

    if (!fontLoaded) {
        return;
    }

    const float centerX = static_cast<float>(AppEngine::screenWidth()) / 2.0f;
    const float centerY = static_cast<float>(AppEngine::ScreenHeight);
    const int score = model::GameManager::instance().getScore();

    sf::Text title(font, "GAME OVER", titleSize);
    title.setFillColor(sf::Color(230, 60, 60));
    view::text::drawCentered(window, title, centerX, centerY * 0.35f);

    const std::string scoreString = "Final Score: " + std::to_string(score);
    const unsigned int fitted = view::text::fitCharacterSize(font, scoreString, static_cast<float>(AppEngine::screenWidth()) * 0.94f, scoreSize);
    sf::Text scoreText(font, scoreString, fitted);
    scoreText.setFillColor(sf::Color::White);
    view::text::drawCentered(window, scoreText, centerX, centerY * 0.55f);

    std::string backKeyName = InputMapper::getKeyName(model::SettingsManager::instance().get().keyBack);
    std::string hintStr = "Press ENTER or " + backKeyName + " to return to Menu";
    sf::Text hint(font, hintStr, hintSize);
    hint.setFillColor(sf::Color(200, 200, 200));
    view::text::drawCentered(window, hint, centerX, centerY * 0.68f);
}

}