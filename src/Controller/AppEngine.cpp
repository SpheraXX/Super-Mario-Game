#include "Controller/AppEngine.h"

#include "Controller/PlayState.h"
#include "Model/Map/TileMap.h"

#include <SFML/System/Clock.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/View.hpp>

#include <algorithm>
#include <fstream>
#include <memory>
#include <optional>
#include <iostream>
#include <stdexcept>
#include <string>

namespace controller {

namespace {
// Upper bound on a single frame's elapsed time, so a stall (e.g. window drag) cannot make
// the fixed-step loop try to catch up with a huge burst of updates ("spiral of death").
constexpr float MaxFrameTime = 0.25f;

// Height of the window chrome (title bar + borders) in pixels, so a window that exactly
// fills the desktop height still leaves the whole client area visible.
constexpr float WindowChrome = 32.0f;

// Largest scale at which the logical frame still fits the desktop with room for the
// chrome. A bigger window would be parked off-screen by the OS (seen as "does not render").
float maxScaleForDesktop() {
    const sf::Vector2u desktop = sf::VideoMode::getDesktopMode().size;
    const float horizontal = static_cast<float>(desktop.x) / AppEngine::ScreenWidth;
    const float vertical =
        (static_cast<float>(desktop.y) - WindowChrome) / AppEngine::ScreenHeight;
    return std::min(horizontal, vertical);
}

// TEMP trace instrumentation (removed after playtest).
void trace(const std::string& msg) {
    std::ofstream out("trace_log.txt", std::ios::app);
    out << msg << '\n';
}
}

AppEngine::AppEngine()
    : window(sf::VideoMode({static_cast<unsigned int>(
                                ScreenWidth * std::min(DefaultWindowScale, maxScaleForDesktop())),
                            static_cast<unsigned int>(
                                ScreenHeight * std::min(DefaultWindowScale, maxScaleForDesktop()))}),
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
        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            // TEMP diagnostics (removed after playtest).
            trace("key=" + std::to_string(static_cast<int>(key->code))
                  + " focus=" + std::to_string(window.hasFocus()));
            if (key->code == sf::Keyboard::Key::F2) {
                // Toggle the window scale: 1x -> 1.5x -> 2x -> 1x.
                scaleIndex = (scaleIndex + 1) % 3;
                applyWindowScale();
                trace("scaleIndex=" + std::to_string(scaleIndex));
            }
        }
        states.handleEvent(*event);
    }
}

void AppEngine::applyWindowScale() {
    // Clamp to what fits the desktop: the requested scale may be 2x on a display that
    // cannot host a 1280x1024 window. (Unsigned underflow in the centering below once
    // parked oversized windows at y=32767, off the visible screen.)
    const float scale = std::min(ScaleOptions[scaleIndex], maxScaleForDesktop());
    const sf::Vector2u newSize{
        static_cast<unsigned int>(ScreenWidth * scale),
        static_cast<unsigned int>(ScreenHeight * scale)};
    window.setSize(newSize);
    trace("window=" + std::to_string(newSize.x) + "x" + std::to_string(newSize.y));

    // Re-center the window on the desktop so a shrink stays fully visible. All arithmetic
    // is signed and the position is clamped: a window larger than the desktop must never
    // wrap into a huge (off-screen) coordinate.
    const sf::Vector2u desktop = sf::VideoMode::getDesktopMode().size;
    window.setPosition({
        std::max(0, (static_cast<int>(desktop.x) - static_cast<int>(newSize.x)) / 2),
        std::max(0, (static_cast<int>(desktop.y) - static_cast<int>(newSize.y)) / 2)});
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

    // Pass 2 — blit the finished frame to the window through the FIXED view, not a scaled
    // sprite. The view always spans the whole window, so the logical frame fills the entire
    // client area at every scale: at 1x this is a direct 1:1 translation, and 1.5x/2x are
    // handled by the window's view transform. Because the scene is already composited, this
    // single draw cannot introduce seams between tiles the way scaling each sprite
    // individually did.
    window.setView(fixedView);
    sf::Sprite frame(scene.getTexture());
    window.draw(frame);
    window.display();
}

}
