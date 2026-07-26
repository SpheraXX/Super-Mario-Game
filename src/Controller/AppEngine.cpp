#include "Controller/AppEngine.h"

#include "Controller/MenuState.h"
#include "Model/TileMap.h"

#include <SFML/System/Clock.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <algorithm>
#include <memory>
#include <optional>

namespace controller {

namespace {
constexpr unsigned int VisibleColumns = 20;
constexpr unsigned int ScreenWidth = VisibleColumns * model::TileMap::TileWidth;
constexpr unsigned int ScreenHeight = model::TileMap::Rows * model::TileMap::TileHeight;

// Upper bound on a single frame's elapsed time, so a stall (e.g. window drag) cannot make
// the fixed-step loop try to catch up with a huge burst of updates ("spiral of death").
constexpr float MaxFrameTime = 0.25f;
}

AppEngine::AppEngine()
    : window(sf::VideoMode({ScreenWidth, ScreenHeight}), "CS202 Super Mario") {
    window.setFramerateLimit(60);

    states.pushState(std::make_unique<MenuState>());
    states.applyPending(); // make the initial state live before the loop starts
}

void AppEngine::run() {
    sf::Clock clock;
    float accumulator = 0.0f;

    while (window.isOpen() && !states.empty()) {
        accumulator += std::min(clock.restart().asSeconds(), MaxFrameTime);

        processInput();

        while (accumulator >= TimeStep) {
            update(TimeStep);
            accumulator -= TimeStep;
        }

        render();

        // Enact any transitions requested during input/update this frame.
        states.applyPending();
    }
}

void AppEngine::processInput() {
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
            return;
        }
        states.handleEvent(*event);
    }
}

void AppEngine::update(float deltaTime) {
    states.update(deltaTime);
}

void AppEngine::render() {
    // Each state owns its own clear colour, so the engine just delegates and displays.
    states.render(window);
    window.display();
}

}
