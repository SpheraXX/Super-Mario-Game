#include "Controller/AppEngine.h"

#include "Controller/PlayState.h"
#include "Model/Map/TileMap.h"

#include <SFML/System/Clock.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/View.hpp>

#include <algorithm>
#include <memory>
#include <optional>
#include <iostream>
#include <stdexcept>

namespace controller {

namespace {
// Upper bound on a single frame's elapsed time, so a stall (e.g. window drag) cannot make
// the fixed-step loop try to catch up with a huge burst of updates ("spiral of death").
constexpr float MaxFrameTime = 0.25f;
}

AppEngine::AppEngine()
    : window(sf::VideoMode({RealScreenWidth, RealScreenHeight}),
             "CS202 Super Mario",
             sf::Style::Titlebar | sf::Style::Close) {
    window.setFramerateLimit(60);

    // Offscreen target at the logical resolution; the whole frame is composited here and
    // upscaled in one blit (see render()).
    if (!scene.resize({ScreenWidth, ScreenHeight})) {
        throw std::runtime_error("Could not create the offscreen render target");
    }
    scene.setSmooth(false);

    // Fixed, non-resizable window: the view always covers exactly the logical resolution.
    // No letterboxing is needed since the aspect ratio is fixed.
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
    // Pass 1 — compose the frame offscreen at the logical resolution, where one world unit
    // is exactly one pixel. Re-apply the fixed view every frame, since states may install
    // their own (e.g. the scrolling camera in PlayState).
    scene.setView(fixedView);

    // Each state owns its own clear colour, so the engine just delegates.
    states.render(scene);
    scene.display();

    // Pass 2 — blit the finished frame to the window, scaled up. Because the scene is
    // already composited, this single draw cannot introduce seams between tiles the way
    // scaling each sprite individually did.
    sf::Sprite scaled(scene.getTexture());
    scaled.setScale({WindowScale, WindowScale});
    window.draw(scaled);
    window.display();
}

}
