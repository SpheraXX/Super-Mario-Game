#include "Controller/AppEngine.h"

#include "Controller/PlayState.h"
#include "Model/TileMap.h"

#include <SFML/System/Clock.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Graphics/View.hpp>

#include <algorithm>
#include <memory>
#include <optional>
#include <iostream>

namespace controller {

namespace {
// Upper bound on a single frame's elapsed time, so a stall (e.g. window drag) cannot make
// the fixed-step loop try to catch up with a huge burst of updates ("spiral of death").
constexpr float MaxFrameTime = 0.25f;
}

AppEngine::AppEngine()
    : window(sf::VideoMode({ScreenWidth, ScreenHeight}), "CS202 Super Mario") {
    window.setFramerateLimit(60);

    // The window may be resized, but never below the original resolution: the game
    // always renders at ScreenWidth x ScreenHeight, centred inside whatever remains.
    window.setMinimumSize(sf::Vector2u{ScreenWidth, ScreenHeight});

    letterboxView.setSize({static_cast<float>(ScreenWidth), static_cast<float>(ScreenHeight)});
    letterboxView.setCenter({ScreenWidth / 2.0f, ScreenHeight / 2.0f});
    letterboxView.setViewport({{0.0f, 0.0f}, {1.0f, 1.0f}});

    states.pushState(std::make_unique<PlayState>());
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
        if (event->is<sf::Event::Resized>()) {
            applyLetterbox();
        }
        states.handleEvent(*event);
    }
}

void AppEngine::applyLetterbox() {
    const sf::Vector2u size = window.getSize();
    const float windowRatio = static_cast<float>(size.x) / static_cast<float>(size.y);
    const float viewRatio = static_cast<float>(ScreenWidth) / static_cast<float>(ScreenHeight);
    float sizeX = 1.0f;
    float sizeY = 1.0f;
    float posX = 0.0f;
    float posY = 0.0f;

    if (windowRatio > viewRatio) {
        sizeX = viewRatio / windowRatio;
        posX = (1.0f - sizeX) / 2.0f;
    } else {
        sizeY = windowRatio / viewRatio;
        posY = (1.0f - sizeY) / 2.0f;
    }

    letterboxView.setViewport(sf::FloatRect({posX, posY}, {sizeX, sizeY}));
}

void AppEngine::update(float deltaTime) {
    states.update(deltaTime);
}

void AppEngine::render() {
    // Re-apply the locked-resolution view every frame (states may temporarily install
    // their own view, e.g. the scrolling camera in PlayState).
    window.setView(letterboxView);

    // Each state owns its own clear colour, so the engine just delegates and displays.
    states.render(window);
    window.display();
}

}
