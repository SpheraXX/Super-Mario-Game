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
    : window(sf::VideoMode({ScreenWidth * WindowScale, ScreenHeight * WindowScale}),
             "CS202 Super Mario",
             sf::Style::Titlebar | sf::Style::Close) {
    window.setFramerateLimit(60);

    // Fixed, non-resizable window: the view always covers exactly the logical resolution,
    // scaled up to the window. No letterboxing is needed since the aspect ratio is fixed.
    fixedView.setSize({static_cast<float>(ScreenWidth), static_cast<float>(ScreenHeight)});
    fixedView.setCenter({ScreenWidth / 2.0f, ScreenHeight / 2.0f});
    fixedView.setViewport({{0.0f, 0.0f}, {1.0f, 1.0f}});

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
        states.handleEvent(*event);
    }
}

void AppEngine::update(float deltaTime) {
    states.update(deltaTime);
}

void AppEngine::render() {
    // Re-apply the fixed-resolution view every frame (states may temporarily install
    // their own view, e.g. the scrolling camera in PlayState).
    window.setView(fixedView);

    // Each state owns its own clear colour, so the engine just delegates and displays.
    states.render(window);
    window.display();
}

}
