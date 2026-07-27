#include "Model/TileMap.h"
#include "Model/Mario.h"
#include "Model/Luigi.h"
#include "View/TileMapRenderer.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <exception>
#include <iostream>
#include <memory>
#include <cstdio>

int main() {
    constexpr unsigned int visibleColumns = 20;
    constexpr unsigned int screenWidth = visibleColumns * model::TileMap::TileWidth;
    constexpr unsigned int screenHeight = model::TileMap::Rows * model::TileMap::TileHeight;
    const sf::Color skyBlue(92, 148, 252);

    try {
        model::TileMap map;
        map.loadFromFile("assets/maps/test.map");

        view::TileMapRenderer renderer("assets/blocks.png");

        sf::RenderWindow window(
            sf::VideoMode({screenWidth, screenHeight}),
            "CS202 Super Mario - Test Scene"
        );
        window.setFramerateLimit(60);

        std::unique_ptr<model::Player> player = std::make_unique<model::Mario>(model::Vector2{200.0f, 600.0f});
        player->setMap(&map);

        sf::Font font;
        bool fontLoaded = font.openFromFile("assets/fonts/arial.ttf")
                       || font.openFromFile("C:/Windows/Fonts/arial.ttf")
                       || font.openFromFile("C:/Windows/Fonts/segoeui.ttf");

        sf::Clock clock;
        bool switching = false;

        while (window.isOpen()) {
            sf::Time elapsed = clock.restart();
            float deltaTime = elapsed.asSeconds();
            if (deltaTime > 0.05f) deltaTime = 0.05f;

            while (const std::optional event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    window.close();
                }
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
                window.close();
            }

            if (!switching) {
                player->handleInput();

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1)) {
                    player->becomeSuper();
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2)) {
                    player->becomeFire();
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3)) {
                    player->becomeStar();
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num4)) {
                    player->takeDamage(1);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
                    player = std::make_unique<model::Mario>(model::Vector2{200.0f, 600.0f});
                    player->setMap(&map);
                }
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num5) && !switching) {
                switching = true;
                if (dynamic_cast<model::Mario*>(player.get())) {
                    auto luigi = std::make_unique<model::Luigi>(model::Vector2{200.0f, 600.0f});
                    luigi->setMap(&map);
                    player = std::move(luigi);
                } else {
                    auto mario = std::make_unique<model::Mario>(model::Vector2{200.0f, 600.0f});
                    mario->setMap(&map);
                    player = std::move(mario);
                }
            } else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num5)) {
                switching = false;
            }

            player->update(deltaTime);
            player->resolveTileCollisions();

            window.clear(skyBlue);
            renderer.render(window, map);
            player->render(window);

            if (fontLoaded) {
                sf::Text text(font, "", 16);
                text.setFillColor(sf::Color::White);
                text.setPosition({10.0f, 10.0f});

                std::string charName = dynamic_cast<model::Luigi*>(player.get()) ? "Luigi" : "Mario";
                std::string stateName = player->getStateName();
                float remaining = player->getRemainingTime();

                std::string info;
                info += "Character: " + charName + "\n";
                info += "State: " + stateName + "\n";
                if (remaining >= 0.0f) {
                    char buf[32];
                    snprintf(buf, sizeof(buf), "Timer: %.1fs\n", remaining);
                    info += buf;
                }
                info += "Lives: " + std::to_string(player->getLives()) + "\n";
                info += "Score: " + std::to_string(player->getScore()) + "\n";
                info += "Coins: " + std::to_string(player->getCoins()) + "\n";
                info += "Facing: " + std::string(player->isFacingRight() ? "Right" : "Left") + "\n";

                text.setString(info);
                window.draw(text);

                sf::Text controls(font, "", 12);
                controls.setFillColor(sf::Color::White);
                controls.setPosition({10.0f, static_cast<float>(screenHeight) - 100.0f});

                std::string ctrl;
                ctrl += "A/D/Left/Right: Move | W/Space: Jump | Shift: Run\n";
                ctrl += "1: Super | 2: Fire | 3: Star | 4: Damage | 5: Switch | R: Reset";

                controls.setString(ctrl);
                window.draw(controls);
            }

            window.display();
        }
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << '\n';
        return 1;
    }

    return 0;
}
