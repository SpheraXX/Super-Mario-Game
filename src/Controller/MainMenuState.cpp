#include "Controller/MainMenuState.h"

// Game-logic dependencies are ONLY in this .cpp, never in the header.
// This is the DIP boundary: the header (and thus every file that includes it)
// stays decoupled from gameplay internals.
#include "Controller/AppEngine.h"
#include "Controller/PlayState.h"
#include "Controller/OptionsState.h"
#include "Controller/StateManager.h"
#include "Controller/WarningPopupState.h"
#include "Model/Core/GameManager.h"
#include "Model/Save/SaveData.h"
#include "Model/Save/SaveManager.h"
#include "View/AssetManager.h"
#include "Controller/IAudioManager.h"
#include "View/UI/UITheme.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <memory>

namespace controller {

// ── Colour palette ────────────────────────────────────────────────────────────
namespace {
constexpr sf::Color BgColor       = sf::Color(10, 10, 30);
constexpr sf::Color TitleColor    = sf::Color(230, 90, 30);
constexpr sf::Color BtnNormal     = sf::Color(40, 40, 70);
constexpr sf::Color BtnHovered    = sf::Color(80, 80, 130);
constexpr sf::Color BtnText       = sf::Color(220, 220, 240);
}

// ── Lifecycle ─────────────────────────────────────────────────────────────────

void MainMenuState::onEnter() {
    model::GameManager::instance().reset();

    fontLoaded = view::AssetManager::instance().isFontLoaded();
    if (!fontLoaded) return;

    if (context && context->audio) {
        context->audio->playMusic("menu");
    }

    buildUI();
}

void MainMenuState::buildUI() {
    const float W = static_cast<float>(AppEngine::screenWidth());
    const float H = static_cast<float>(AppEngine::ScreenHeight);
    const sf::Font& font = view::AssetManager::instance().getUiFont();

    // ── Backdrop ──────────────────────────────────────────────────────────────
    backdrop.setSize({W, H});
    backdrop.setFillColor(BgColor);

    // ── Title ─────────────────────────────────────────────────────────────────
    titleLabel = view::ui::UILabel(font, "SUPER MARIO", view::ui::layout::TitleFontSize, TitleColor);
    titleLabel.setPosition(0.f, H * 0.18f);
    titleLabel.setSize(W, static_cast<float>(view::ui::layout::TitleFontSize) * 2.f);
    titleLabel.setCentered(true);

    // ── Button list (Vertical UIContainer) ───────────────────────────────────
    menuList = view::ui::UIContainer(view::ui::UIContainer::Layout::Vertical, view::ui::layout::MenuButtonGap);
    const float listX = (W - view::ui::layout::MenuButtonWidth) / 2.f;
    const float listY = H * 0.38f;
    menuList.setPosition(listX, listY);
    menuList.setSize(view::ui::layout::MenuButtonWidth, 0.f);  // height auto-computed by relayout

    // Helper: build one button and inject its Command callback.
    auto makeBtn = [&](const std::string& label, std::function<void()> cmd) {
        auto btn = std::make_unique<view::ui::UIButton>(
            font, label, view::ui::layout::ButtonFontSize, sf::Vector2f{listX, 0.f}, 
            sf::Vector2f{view::ui::layout::MenuButtonWidth, view::ui::layout::MenuButtonHeight});
        btn->setColors(BtnNormal, BtnHovered, BtnText);
        btn->setOnClick(std::move(cmd));
        menuList.add(std::move(btn));
    };

    // ── Commands injected here — this is the ONLY place game logic is touched. ──
    makeBtn("START GAME", [this]() {
        if (model::SaveManager::instance().hasSaveFile()) {
            manager->pushState(std::make_unique<WarningPopupState>(
                "Found saved game.\nContinue where you left off?",
                WarningPopupState::Type::YesNo,
                [this]() {
                    model::GameSaveData save;
                    if (model::SaveManager::instance().load(save)) {
                        manager->clear();
                        manager->pushState(std::make_unique<PlayState>(save));
                    } else {
                        model::GameManager::instance().reset();
                        manager->clear();
                        manager->pushState(std::make_unique<PlayState>());
                    }
                },
                [this]() {
                    model::SaveManager::instance().deleteSave();
                    model::GameManager::instance().reset();
                    manager->clear();
                    manager->pushState(std::make_unique<PlayState>());
                },
                "CONTINUE",
                "NEW GAME"
            ));
        } else {
            model::GameManager::instance().reset();
            manager->replaceState(std::make_unique<PlayState>());
        }
    });

    makeBtn("OPTIONS", [this]() {
        manager->pushState(std::make_unique<OptionsState>());
    });

    makeBtn("PROFILE",    [this]() {
        // TODO: push ProfileState when Phase 3 is ready.
    });

    makeBtn("CREDITS",    [this]() {
        // TODO: push CreditsState.
    });

    makeBtn("EXIT",       [this]() {
        manager->clear();
    });

    menuList.relayout();
}

// ── Per-frame ─────────────────────────────────────────────────────────────────

void MainMenuState::onDisplayModeChanged() {
    const float W = static_cast<float>(AppEngine::screenWidth());
    const float H = static_cast<float>(AppEngine::ScreenHeight);
    
    backdrop.setSize({W, H});
    
    titleLabel.setSize(W, static_cast<float>(view::ui::layout::TitleFontSize) * 2.f);
    titleLabel.setPosition(0.f, H * 0.18f);

    const float listX = (W - view::ui::layout::MenuButtonWidth) / 2.f;
    const float listY = H * 0.38f;
    menuList.setPosition(listX, listY);
    menuList.relayout();
}

void MainMenuState::onResume() {
    onDisplayModeChanged();
}

void MainMenuState::handleEvent(const sf::Event& event) {
    menuList.handleEvent(event);
}

void MainMenuState::update(float deltaTime) {
    menuList.update(deltaTime);
}

void MainMenuState::render(sf::RenderTarget& target) {
    target.draw(backdrop);
    if (!fontLoaded) return;
    titleLabel.render(target);
    menuList.render(target);
}

}  // namespace controller
