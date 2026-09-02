#include "Controller/MainMenuState.h"

// Game-logic dependencies are ONLY in this .cpp, never in the header.
// This is the DIP boundary: the header (and thus every file that includes it)
// stays decoupled from gameplay internals.
#include "Controller/AppEngine.h"
#include "Controller/CreditsState.h"
#include "Controller/CustomMapHubState.h"
#include "Controller/IAudioManager.h"
#include "Controller/OptionsState.h"
#include "Controller/PlayState.h"
#include "Controller/StateManager.h"
#include "Controller/ProfileMenuState.h"
#include "Controller/WarningPopupState.h"
#include "Controller/WorldSelectState.h"
#include "Model/Core/GameManager.h"
#include "Model/Save/SaveData.h"
#include "Model/Save/SaveManager.h"
#include "View/AssetManager.h"
#include "View/UI/UITheme.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <cmath>
#include <memory>

namespace controller {

// ── Colour palette
// ────────────────────────────────────────────────────────────
namespace {
constexpr sf::Color BgColor = sf::Color(10, 10, 30);
constexpr sf::Color TitleColor = sf::Color(230, 90, 30);
} // namespace

// ── Lifecycle
// ─────────────────────────────────────────────────────────────────

MainMenuState::MainMenuState()
    : bgaSprite(view::AssetManager::instance().getTexture(
          "assets/images/bga_mainmenu.png")),
      titleSprite(view::AssetManager::instance().getTexture(
          "assets/images/bga_mainmenu_title.png")) {}

void MainMenuState::onEnter() {
  model::GameManager::instance().reset();

  fontLoaded = view::AssetManager::instance().isFontLoaded();
  if (!fontLoaded)
    return;

  if (context && context->audio) {
    context->audio->playMusic("menu");
  }

  buildUI();
}

void MainMenuState::buildUI() {
  const sf::Font &font = view::AssetManager::instance().getUiFont();
  const float W = static_cast<float>(AppEngine::screenWidth());
  const float H = static_cast<float>(AppEngine::ScreenHeight);

  // ── Backdrop ──────────────────────────────────────────────────────────────
  // (scale/position applied via onDisplayModeChanged() at end of buildUI)
  bgaSprite.setColor(view::ui::theme::BgaDimMenu);

  // ── Title ─────────────────────────────────────────────────────────────────
  // Origin set once — scale/position handled by onDisplayModeChanged().
  titleSprite.setOrigin({static_cast<float>(titleSprite.getTexture().getSize().x) / 2.f,
                         static_cast<float>(titleSprite.getTexture().getSize().y) / 2.f});

  // ── Button list (Vertical UIContainer) ───────────────────────────────────
  menuList = view::ui::UIContainer(view::ui::UIContainer::Layout::Vertical,
                                   view::ui::layout::MenuButtonGap);
  const float listX = (W - view::ui::layout::MenuButtonWidth) / 2.f;

  // Set up animation target and initial position
  float targetContainerY = H * 0.38f;
  m_menuSlideIn = view::effect::LerpAnimator(H, targetContainerY, 0.6f,
                                             view::effect::Easing::OutQuad);
  menuList.setPosition(listX, m_menuSlideIn.value());
  menuList.setSize(view::ui::layout::MenuButtonWidth,
                   0.f); // height auto-computed by relayout

  // Helper: build one button and inject its Command callback.
  auto makeBtn = [&](const std::string &label, std::function<void()> cmd) {
    auto btn = std::make_unique<view::ui::UIButton>(
        font, label, view::ui::layout::ButtonFontSize, sf::Vector2f{listX, 0.f},
        sf::Vector2f{view::ui::layout::MenuButtonWidth,
                     view::ui::layout::MenuButtonHeight});
    btn->setOnClick(std::move(cmd));
    menuList.add(std::move(btn));
  };

  // ── Commands injected here — this is the ONLY place game logic is touched.
  // ──
  makeBtn("START GAME", [this]() {
    if (model::SaveManager::instance().hasSaveFile()) {
      manager->pushState(std::make_unique<WarningPopupState>(
          "Found saved game.\nContinue where you left off?",
          WarningPopupState::Type::YesNo,
          [this]() {
            model::GameSaveData save;
            if (model::SaveManager::instance().load(save)) {
              auto& game = model::GameManager::instance();
              game.setScore(save.score);
              game.setLives(save.lives);
              game.setCoins(save.coins);
              manager->clear();
              manager->pushState(std::make_unique<WorldSelectState>());
            } else {
              model::GameManager::instance().reset();
              manager->clear();
              manager->pushState(std::make_unique<WorldSelectState>());
            }
          },
          [this]() {
            model::SaveManager::instance().deleteSave();
            model::GameManager::instance().reset();
            manager->clear();
            manager->pushState(std::make_unique<WorldSelectState>());
          },
          "CONTINUE", "NEW GAME"));
    } else {
      model::GameManager::instance().reset();
      manager->replaceState(std::make_unique<WorldSelectState>());
    }
  });

  makeBtn("OPTIONS",
          [this]() { manager->pushState(std::make_unique<OptionsState>()); });

  makeBtn("PROFILE", [this]() {
    manager->pushState(std::make_unique<ProfileMenuState>());
  });

  makeBtn("CREDITS", [this]() {
    manager->pushState(std::make_unique<CreditsState>());
  });

  makeBtn("CUSTOM MAP", [this]() {
    manager->pushState(std::make_unique<CustomMapHubState>());
  });

  makeBtn("EXIT", [this]() { manager->clear(); });

  onDisplayModeChanged();
}

// ── Per-frame
// ─────────────────────────────────────────────────────────────────

void MainMenuState::onDisplayModeChanged() {
  const float W = static_cast<float>(AppEngine::screenWidth());
  const float H = static_cast<float>(AppEngine::ScreenHeight);

  const sf::Texture &tex = bgaSprite.getTexture();
  float scaleX = W / static_cast<float>(tex.getSize().x);
  float scaleY = H / static_cast<float>(tex.getSize().y);
  float scale = std::max(scaleX, scaleY) * view::ui::layout::BgaScaleMultiplier;
  bgaSprite.setScale({scale, scale});
  bgaSprite.setOrigin({static_cast<float>(tex.getSize().x) / 2.f,
                       static_cast<float>(tex.getSize().y) / 2.f});
  bgaSprite.setPosition({W / 2.f, H / 2.f});

  baseTitleY = H * 0.18f;

  // Scale title proportionally (e.g. 60% of screen width)
  float titleScale = (W * view::ui::layout::TitleWidthRatio) / static_cast<float>(titleSprite.getTexture().getSize().x);
  titleSprite.setScale({titleScale, titleScale});
  titleSprite.setPosition({W / 2.f, baseTitleY});

  const float listX = (W - view::ui::layout::MenuButtonWidth) / 2.f;
  menuList.setPosition(
      listX, m_menuSlideIn.value()); // Keep current animated Y position
  menuList.relayout();
}

void MainMenuState::onResume() { onDisplayModeChanged(); }

void MainMenuState::handleEvent(const sf::Event &event) {
  menuList.handleEvent(event);
}

void MainMenuState::update(float deltaTime) {
  m_menuSlideIn.update(deltaTime);
  m_titleShine.update(deltaTime);
  menuList.setPosition(menuList.getPosition().x, m_menuSlideIn.value());

  menuList.update(deltaTime);
}

void MainMenuState::render(sf::RenderTarget &target) {
  target.draw(bgaSprite);
  if (!fontLoaded)
    return;
  m_titleShine.draw(target, titleSprite);
  menuList.render(target);
}

} // namespace controller
