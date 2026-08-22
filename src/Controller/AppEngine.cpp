#include "Controller/AppEngine.h"

#include "Controller/MainMenuState.h"
#include "Model/Core/LogManager.h"
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

unsigned int AppEngine::logicalWidth   = AppEngine::LogicalWidth4x3;
float        AppEngine::displayOffsetX = 0.f;
float        AppEngine::displayOffsetY = 0.f;
float        AppEngine::displayScale   = 2.0f;

unsigned int AppEngine::screenWidth() {
    return logicalWidth;
}

AppEngine::AppEngine()
    : audioManager()
    , inputMapper()
    , gameContext{&audioManager, &inputMapper}
    , states(std::make_unique<MainMenuState>(), &gameContext) {

    model::SettingsManager::instance().addObserver(this);

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

void AppEngine::onSettingsChanged(const model::Settings& s) {
    audioManager.setMasterVolume(static_cast<float>(s.masterVolume));
    audioManager.setMusicVolume(static_cast<float>(s.musicVolume));
    audioManager.setSFXVolume(static_cast<float>(s.sfxVolume));

    // Only recreate the window (and reload all VRAM assets) when display-relevant
    // settings actually changed. Volume/key changes must NOT trigger this path.
    const bool displayChanged = (s.fullscreen      != lastGraphicsSettings.fullscreen      ||
                                 s.ratio           != lastGraphicsSettings.ratio           ||
                                 s.resolutionIndex != lastGraphicsSettings.resolutionIndex ||
                                 s.vsync           != lastGraphicsSettings.vsync);
    lastGraphicsSettings = s;

    if (displayChanged) {
        applyDisplayMode();
    }
}

void AppEngine::applyDisplayMode() {
    const auto& s = model::SettingsManager::instance().get();

    // logicalWidth is always derived from the user-chosen ratio, both windowed and fullscreen.
    logicalWidth = (s.ratio == model::AspectRatio::Ratio4x3) ? LogicalWidth4x3 : LogicalWidth16x9;

    const sf::Vector2u desktop = sf::VideoMode::getDesktopMode().size;
    float   scale = 1.0f;
    sf::VideoMode mode;

    if (s.fullscreen) {
        // Float scale so ScreenHeight fills desktop height exactly — no horizontal black bars.
        // Pillarbox (vertical black bars) appears on the sides when logicalWidth*scale < desktop.x.
        scale = static_cast<float>(desktop.y) / static_cast<float>(ScreenHeight);
        mode  = sf::VideoMode::getDesktopMode();
    } else {
        // Use ResolutionOption to find the largest integer scale that fits both dimensions,
        // then shrink the window to exactly logicalW×scale — no leftover pixels, no black bars.
        const ResolutionOption* opts =
            (s.ratio == model::AspectRatio::Ratio4x3) ? Ratio4x3Options : Ratio16x9Options;
        const int idx = std::clamp(s.resolutionIndex, 0, 1);

        const unsigned int resW = opts[idx].width;
        const unsigned int resH = opts[idx].height;

        unsigned int intScale = std::max(1u, std::min(resW / logicalWidth, resH / ScreenHeight));

        // Exact window = logical frame * scale.
        unsigned int windowW = logicalWidth * intScale;
        unsigned int windowH = ScreenHeight * intScale;

        // Hard-clamp against desktop so the window never falls off-screen.
        windowW = std::min(windowW, desktop.x);
        windowH = std::min(windowH, desktop.y > WindowChrome ? desktop.y - WindowChrome : 1u);

        // Recompute float scale from the possibly-clamped window so windowToLogical stays accurate.
        scale = std::min(static_cast<float>(windowW) / static_cast<float>(logicalWidth),
                         static_cast<float>(windowH) / static_cast<float>(ScreenHeight));

        mode = sf::VideoMode({windowW, windowH});
    }

    window.create(mode, "CS202 Super Mario",
                  s.fullscreen ? static_cast<std::uint32_t>(sf::Style::None)
                               : static_cast<std::uint32_t>(sf::Style::Titlebar | sf::Style::Close),
                  s.fullscreen ? sf::State::Fullscreen : sf::State::Windowed);
    
    // Apply VSync
    window.setVerticalSyncEnabled(s.vsync);
    if (!s.vsync) {
        window.setFramerateLimit(60);
    } else {
        window.setFramerateLimit(0); // VSync handles framerate
    }

    // Offscreen target at the logical resolution; the whole frame is composited here and
    // upscaled in one blit (see render()).
    if (!scene.resize({logicalWidth, ScreenHeight})) {
        return;  // fail early; constructor turns this into an exception
    }
    scene.setSmooth(false);

    // The view the states draw through: exactly the logical frame, spanning the whole offscreen target.
    sceneView.setSize({static_cast<float>(logicalWidth), static_cast<float>(ScreenHeight)});
    sceneView.setCenter({static_cast<float>(logicalWidth) * 0.5f,
                         static_cast<float>(ScreenHeight)  * 0.5f});
    sceneView.setViewport({{0.0f, 0.0f}, {1.0f, 1.0f}});

    // Blit the offscreen frame into a centred rectangle on the real window.
    // Clamp 'used' to client size so viewport fractions never exceed 1.
    const sf::Vector2u client = window.getSize();
    const float clientW = static_cast<float>(client.x);
    const float clientH = static_cast<float>(client.y);
    const float usedW = std::min(static_cast<float>(logicalWidth) * scale, clientW);
    const float usedH = std::min(static_cast<float>(ScreenHeight)  * scale, clientH);
    const float offsetX = std::floor((clientW - usedW) * 0.5f);
    const float offsetY = std::floor((clientH - usedH) * 0.5f);
    presentView = sceneView;
    presentView.setViewport({{offsetX / clientW, offsetY / clientH},
                             {usedW  / clientW,  usedH  / clientH}});

    // Store statics so windowToLogical() works without an engine reference.
    displayOffsetX = offsetX;
    displayOffsetY = offsetY;
    displayScale   = scale;

    std::cerr << "display: " << (s.fullscreen ? "fullscreen" : "windowed")
              << " window " << client.x << 'x' << client.y
              << "  logical " << logicalWidth << 'x' << ScreenHeight
              << " (" << logicalWidth / model::TileMap::TileWidth << " cols x "
              << model::TileMap::Rows << " rows)"
              << "  scale " << scale << 'x'
              << "  bars " << offsetX << ',' << offsetY << '\n';

    // Recover graphics context: window recreation destroys all VRAM objects.
    view::AssetManager::instance().reloadAll();

    // Notify the active state so it can reposition its UI.
    if (!states.empty()) {
        states.activeState()->onDisplayModeChanged();
    }
}

void AppEngine::cycleDisplayMode() {
    auto s = model::SettingsManager::instance().get();

    // Cycle: 4x3 Windowed → 16x9 Windowed → Fullscreen → back to 4x3 Windowed
    if (!s.fullscreen) {
        if (s.ratio == model::AspectRatio::Ratio4x3) {
            s.ratio = model::AspectRatio::Ratio16x9;
        } else {
            s.fullscreen = true;
        }
    } else {
        s.fullscreen = false;
        s.ratio = model::AspectRatio::Ratio4x3;
    }

    model::SettingsManager::instance().apply(s);
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
    // double step). One duplicate plus one double-step, several times a second, is the stutter.
    //
    // It is worst in the air because of the camera: it tracks the player horizontally, so
    // sideways a repeated frame barely registers, but it is pinned vertically, so every
    // hitch in y is drawn straight to the screen with nothing moving alongside to mask it.
    constexpr float FrameSnapTolerance = TimeStep * 0.25f;  // ~4ms at 60Hz

    model::LogManager::instance().info("Game start");

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

    model::LogManager::instance().info("Game end");
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
            if (static_cast<int>(key->code) == model::SettingsManager::instance().get().keyCycleDisplay) {
                cycleDisplayMode();
                // cycleDisplayMode calls SettingsManager::apply() which triggers the subscription,
                // setting applyDisplayPending = true.
            }
        }
        states.handleEvent(*event);
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
    // bars: any part of the window the scaled frame does not cover stays black.
    window.clear(sf::Color::Black);
    window.setView(presentView);
    sf::Sprite frame(scene.getTexture());
    window.draw(frame);
    window.display();
}

sf::Vector2f AppEngine::windowToLogical(sf::Vector2i windowPos) {
    // Physical pixels → logical pixels.
    // Subtract the letterbox bars, then divide by the float scale.
    return {std::max(0.0f, static_cast<float>(windowPos.x) - displayOffsetX) / displayScale,
            std::max(0.0f, static_cast<float>(windowPos.y) - displayOffsetY) / displayScale};
}

}  // namespace controller
