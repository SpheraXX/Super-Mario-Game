#include "Controller/AppEngine.h"

#include "Controller/MainMenuState.h"
#include "Model/Map/TileMap.h"
#include "Model/SettingsManager.h"
#include "View/AssetManager.h"
#include "View/UI/UIElement.h"

#include <SFML/System/Clock.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/View.hpp>

#include <algorithm>
#include <cmath>
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
}

// Starts on the first windowed size; applyDisplayMode() overwrites this before the first
// frame, including for fullscreen, where the width is measured off the display.
unsigned int AppEngine::logicalWidth  = AppEngine::SizeOptions[0].logicalWidth;
float        AppEngine::displayOffsetX = 0.f;
float        AppEngine::displayOffsetY = 0.f;
unsigned int AppEngine::displayScale   = AppEngine::SizeOptions[0].scale;

unsigned int AppEngine::screenWidth() {
    return logicalWidth;
}

AppEngine::AppEngine() 
    : audioManager()
    , inputMapper()
    , gameContext{&audioManager, &inputMapper}
    , states(std::make_unique<MainMenuState>(), &gameContext) {
    model::SettingsManager::instance().subscribe([this](const model::Settings& s) {
        // AudioManager is subscribed directly, but we can do it here:
        audioManager.setMasterVolume(static_cast<float>(s.masterVolume));
        audioManager.setMusicVolume(static_cast<float>(s.musicVolume));
        audioManager.setSFXVolume(static_cast<float>(s.sfxVolume));
    });

    applyDisplayMode();  // creates the window, the offscreen target and both views

    // Offscreen target could not be sized: nothing can be drawn, so fail loudly here
    // rather than rendering black for the rest of the run.
    if (scene.getSize().x == 0) {
        throw std::runtime_error("Could not create the offscreen render target");
    }

    states.pushState(std::make_unique<MainMenuState>());
    states.applyPending(); // make the initial state live before the loop starts

    // Inject coordinate transform into UI layer once — keeps View independent of Controller.
    view::ui::UIElement::transformCoordinate = [](const sf::Vector2i& p) {
        return AppEngine::windowToLogical(p);
    };
}

void AppEngine::applyDisplayMode() {
    const sf::Vector2u desktop = sf::VideoMode::getDesktopMode().size;

    unsigned int scale = 1;
    sf::VideoMode mode;
    if (fullscreen) {
        // 16 rows must fill the display, so the magnification comes from the height and is
        // rounded DOWN to an integer: on a 1080p screen that is 4x (1024 of 1080 rows used)
        // and the 56 leftover pixels become the letterbox. Keeping it whole is the point —
        // a fractional scale is what makes tile edges shimmer.
        scale = std::max(1u, desktop.y / ScreenHeight);
        // Then as many logical columns as the width allows.
        logicalWidth = std::max(ScreenHeight, desktop.x / scale);
        mode = sf::VideoMode::getDesktopMode();
    } else {
        const DisplayOption& option = SizeOptions[sizeIndex];
        // Shrink the magnification — never the logical frame — until the window fits the
        // desktop. A 4x window on a small display would otherwise be parked partly
        // off-screen by the OS, which reads as the game failing to render.
        scale = option.scale;
        while (scale > 1
               && (option.logicalWidth * scale > desktop.x
                   || ScreenHeight * scale + WindowChrome > desktop.y)) {
            --scale;
        }
        logicalWidth = option.logicalWidth;
        mode = sf::VideoMode({option.logicalWidth * scale, ScreenHeight * scale});
    }

    window.create(mode, "CS202 Super Mario",
                  fullscreen ? static_cast<std::uint32_t>(sf::Style::None)
                             : static_cast<std::uint32_t>(sf::Style::Titlebar | sf::Style::Close),
                  fullscreen ? sf::State::Fullscreen : sf::State::Windowed);
    window.setFramerateLimit(60);

    // Offscreen target at the logical resolution; the whole frame is composited here and
    // upscaled in one blit (see render()).
    if (!scene.resize({logicalWidth, ScreenHeight})) {
        return;  // the constructor turns this into an exception
    }
    scene.setSmooth(false);

    // The view the states draw through: exactly the logical frame, spanning the whole
    // offscreen target.
    sceneView.setSize({static_cast<float>(logicalWidth), static_cast<float>(ScreenHeight)});
    sceneView.setCenter({logicalWidth / 2.0f, ScreenHeight / 2.0f});
    sceneView.setViewport({{0.0f, 0.0f}, {1.0f, 1.0f}});

    // The view the finished frame is blitted through: the same logical frame, but confined
    // to a centred, integer-scaled rectangle of the window. Whatever the rectangle does not
    // cover stays the black the window is cleared to — the letterbox.
    const sf::Vector2u client = window.getSize();
    const sf::Vector2f used{static_cast<float>(logicalWidth * scale),
                            static_cast<float>(ScreenHeight * scale)};
    const float offsetX = std::floor(std::max(0.0f, (client.x - used.x) / 2.0f));
    const float offsetY = std::floor(std::max(0.0f, (client.y - used.y) / 2.0f));
    presentView = sceneView;
    presentView.setViewport({{offsetX / client.x, offsetY / client.y},
                             {used.x / client.x, used.y / client.y}});

    // Store statics so windowToLogical() works without an engine reference.
    displayOffsetX = offsetX;
    displayOffsetY = offsetY;
    displayScale   = scale;

    std::cerr << "display: " << (fullscreen ? "fullscreen" : "windowed")
              << " window " << client.x << 'x' << client.y
              << "  logical " << logicalWidth << 'x' << ScreenHeight
              << " (" << logicalWidth / model::TileMap::TileWidth << " cols x "
              << model::TileMap::Rows << " rows)"
              << "  scale " << scale << 'x'
              << "  bars " << offsetX << ',' << offsetY << '\n';
}

void AppEngine::cycleDisplayMode() {
    // The windowed sizes in order, then fullscreen, then back to the smallest window.
    if (fullscreen) {
        fullscreen = false;
        sizeIndex = 0;
    } else if (sizeIndex + 1 < SizeOptionCount) {
        ++sizeIndex;
    } else {
        fullscreen = true;
    }
    applyDisplayMode();
}

void AppEngine::run() {
    sf::Clock clock;
    float accumulator = 0.0f;

    // A frame this close to the fixed step is treated as exactly one step.
    //
    // The limiter paces the loop with sleeps, and sleep is not precise: real frames land at
    // 16.4ms, 16.9ms, 17.1ms... around the 16.67ms step. Fed straight into the accumulator
    // that beats against the fixed step — a frame measuring a hair under runs ZERO updates
    // (the identical picture is presented twice) and the next runs TWO (the world lurches a
    // double step). One duplicate plus one double-step, several times a second, is the
    // stutter.
    //
    // It is worst in the air because of the camera: it tracks the player horizontally, so
    // sideways a repeated frame barely registers, but it is pinned vertically, so every
    // hitch in y is drawn straight to the screen with nothing moving alongside to mask it.
    constexpr float FrameSnapTolerance = TimeStep * 0.25f;  // ~4ms at 60Hz

    while (window.isOpen() && !states.empty()) {
        float frameTime = std::min(clock.restart().asSeconds(), MaxFrameTime);
        if (std::fabs(frameTime - TimeStep) < FrameSnapTolerance) {
            frameTime = TimeStep;
        }
        accumulator += frameTime;

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
            // Cycle the window size / fullscreen. Deferred to after the queue is drained:
            // applyDisplayMode() recreates the window, and doing that mid-poll would
            // destroy the queue this loop is reading.
            if (key->code == sf::Keyboard::Key::F2) {
                displayChangePending = true;
            }
        }
        states.handleEvent(*event);
    }

    if (displayChangePending) {
        displayChangePending = false;
        cycleDisplayMode();
    }
}

void AppEngine::update(float deltaTime) {
    states.update(deltaTime);
}

void AppEngine::render() {
    // Pass 1 — compose the frame offscreen at the logical resolution, where one world unit
    // is exactly one pixel. Re-apply the view every frame, since states install their own
    // (e.g. the scrolling camera in PlayState).
    scene.setView(sceneView);

    // Each state owns its own clear colour, so the engine just delegates.
    states.render(scene);
    scene.display();

    // Pass 2 — blit the finished frame through the letterboxed view. The clear paints the
    // bars: any part of the window the integer-scaled frame does not cover stays black.
    window.clear(sf::Color::Black);
    window.setView(presentView);
    sf::Sprite frame(scene.getTexture());
    window.draw(frame);
    window.display();
}

sf::Vector2f AppEngine::windowToLogical(sf::Vector2i windowPos) {
    // Physical pixels → logical pixels.
    // Subtract the letterbox bars, then divide by the integer scale.
    const float lx = (static_cast<float>(windowPos.x) - displayOffsetX)
                     / static_cast<float>(displayScale);
    const float ly = (static_cast<float>(windowPos.y) - displayOffsetY)
                     / static_cast<float>(displayScale);
    return {lx, ly};
}

}  // namespace controller
