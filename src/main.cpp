#include "Model/TileMap.h"
#include "View/TileMapRenderer.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <exception>
#include <iostream>

int main() {
    constexpr unsigned int visibleColumns = 20;
    constexpr unsigned int screenWidth = visibleColumns * model::TileMap::TileWidth;
    constexpr unsigned int screenHeight = model::TileMap::Rows * model::TileMap::TileHeight;
    const sf::Color skyBlue(92, 148, 252);

    try {
        model::TileMap map;
        map.loadFromFile("assets/maps/plain.map");

        view::TileMapRenderer renderer("assets/blocks.png");

        sf::RenderWindow window(
            sf::VideoMode({screenWidth, screenHeight}),
            "CS202 Super Mario"
        );
        window.setFramerateLimit(60);

        while (window.isOpen()) {
            while (const std::optional event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    window.close();
                }
            }

            window.clear(skyBlue);
            renderer.render(window, map);
            window.display();
        }
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << '\n';
        return 1;
    }

    return 0;
}
