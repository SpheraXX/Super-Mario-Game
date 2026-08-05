#include "Controller/GameOverState.h"

#include "Controller/AppEngine.h"
#include "Controller/MenuState.h"
#include "Controller/StateManager.h"
#include "Model/Core/GameManager.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>

#include <memory>
#include <iostream>
#include <string>

namespace controller {

namespace {
void centerOrigin(sf::Text& text) {
    const sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f});
}
}

void GameOverState::onEnter() {
    fontLoaded = font.openFromFile("assets/fonts/Tuffy.ttf");
}

void GameOverState::handleEvent(const sf::Event& event) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Enter) {
            // MenuState::onEnter() resets progress, so return to the menu.
            manager->replaceState(std::make_unique<MenuState>());
        }
    }
}

void GameOverState::update(float deltaTime) {
    (void)deltaTime;
}

void GameOverState::render(sf::RenderTarget& window) {
    std :: cerr << "Rendering GameOverState" << std :: endl;
    window.clear(sf::Color(60, 10, 10));

    if (!fontLoaded) {
        return;
    }

    const float centerX = static_cast<float>(AppEngine::ScreenWidth) / 2.0f;
    const float centerY = static_cast<float>(AppEngine::ScreenHeight);
    const int score = model::GameManager::instance().getScore();

    sf::Text title(font, "GAME OVER", 56);
    title.setFillColor(sf::Color(230, 60, 60));
    centerOrigin(title);
    title.setPosition({centerX, centerY * 0.35f});
    window.draw(title);

    sf::Text scoreText(font, "Final Score: " + std::to_string(score), 26);
    scoreText.setFillColor(sf::Color::White);
    centerOrigin(scoreText);
    scoreText.setPosition({centerX, centerY * 0.55f});
    window.draw(scoreText);

    sf::Text hint(font, "Press ENTER to return to Menu", 20);
    hint.setFillColor(sf::Color(200, 200, 200));
    centerOrigin(hint);
    hint.setPosition({centerX, centerY * 0.68f});
    window.draw(hint);
}

}
