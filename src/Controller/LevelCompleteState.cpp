#include "Controller/LevelCompleteState.h"

#include "Controller/AppEngine.h"
#include "Controller/MenuState.h"
#include "Controller/PlayState.h"
#include "Controller/StateManager.h"
#include "Model/Core/GameManager.h"
#include "View/AssetManager.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>

#include <memory>
#include <string>

namespace controller {

namespace {
void centerOrigin(sf::Text& text) {
    const sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f});
}
}

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
    const float centerX = static_cast<float>(AppEngine::ScreenWidth) / 2.0f;
    const float centerY = static_cast<float>(AppEngine::ScreenHeight);

    sf::Text title(font, "COURSE CLEAR!", 36);
    title.setFillColor(sf::Color(255, 230, 90));
    title.setOutlineColor(sf::Color::Black);
    title.setOutlineThickness(3.f);
    centerOrigin(title);
    title.setPosition({centerX, centerY * 0.30f});
    window.draw(title);

    const int bonus = model::GameManager::instance().getLevelClearBonus();
    sf::Text bonusText(font, "BONUS: " + std::to_string(bonus), 20);
    bonusText.setFillColor(sf::Color::White);
    bonusText.setOutlineColor(sf::Color::Black);
    bonusText.setOutlineThickness(2.f);
    centerOrigin(bonusText);
    bonusText.setPosition({centerX, centerY * 0.45f});
    window.draw(bonusText);

    sf::Text hint(font, "PRESS ENTER", 16);
    hint.setFillColor(sf::Color(200, 200, 200));
    hint.setOutlineColor(sf::Color::Black);
    hint.setOutlineThickness(2.f);
    centerOrigin(hint);
    hint.setPosition({centerX, centerY * 0.58f});
    window.draw(hint);
}

}
