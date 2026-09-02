#include "Controller/GameOverState.h"
#include "Controller/AppEngine.h"
#include "Controller/IAudioManager.h"
#include "Controller/MainMenuState.h"
#include "Controller/LevelSelectState.h"
#include "Controller/StateManager.h"
#include "Model/Core/GameManager.h"
#include "Model/Core/WorldManager.h"
#include "View/AssetManager.h"
#include "View/UI/UIButton.h"
#include "View/UI/UITheme.h"


#include <SFML/Graphics/RenderTarget.hpp>
#include <string>

namespace controller {

GameOverState::GameOverState(std::function<void()> onRestartCallback)
    : m_onRestartCallback(std::move(onRestartCallback)),
      m_buttonList(view::ui::UIContainer::Layout::Vertical,
                   view::ui::layout::MenuButtonGap),
      m_background(view::AssetManager::instance().getTexture(
          "assets/images/bga_gameover.png")) {}

void GameOverState::onEnter() {
  if (context && context->audio) {
    context->audio->playMusic("game_over");
  }

  fontLoaded = view::AssetManager::instance().isFontLoaded();
  if (fontLoaded) {
    buildUI();
  }
}

void GameOverState::buildUI() {
  const sf::Font &font = view::AssetManager::instance().getUiFont();

  int score = model::GameManager::instance().getScore();
  m_scoreLabel =
      view::ui::UILabel(font, "Final Score: " + std::to_string(score),
                        view::ui::layout::ButtonFontSize, sf::Color::White);
  m_scoreLabel.setCentered(true);

  auto makeBtn = [&](const std::string &label, std::function<void()> cmd) {
    auto btn = std::make_unique<view::ui::UIButton>(
        font, label, view::ui::layout::ButtonFontSize, sf::Vector2f{0.f, 0.f},
        sf::Vector2f{view::ui::layout::MenuButtonWidth,
                     view::ui::layout::MenuButtonHeight});
    btn->setOnClick(std::move(cmd));
    m_buttonList.add(std::move(btn));
  };

  makeBtn("RESTART", [this]() {
    if (m_onRestartCallback)
      m_onRestartCallback();
  });

  makeBtn("QUIT", [this]() {
    std::string mapPath = model::GameManager::instance().getCurrentMapPath();
    std::string worldId = model::WorldManager::instance().getWorldIdFromMapPath(mapPath);
    if (worldId.empty()) worldId = "world_1";

    manager->replaceState(std::make_unique<LevelSelectState>(worldId));
  });

  onDisplayModeChanged();
  // Init animator ONCE: buttons slide in from bottom of screen.
  float H = static_cast<float>(AppEngine::ScreenHeight);
  m_slideIn = view::effect::LerpAnimator(H, H * 0.75f, 0.6f,
                                         view::effect::Easing::OutQuad);
  m_buttonList.setPosition(m_buttonList.getPosition().x, m_slideIn.value());
}

void GameOverState::onDisplayModeChanged() {
  if (!fontLoaded)
    return;

  float W = static_cast<float>(AppEngine::screenWidth());
  float H = static_cast<float>(AppEngine::ScreenHeight);

  const sf::Texture &tex = m_background.getTexture();
  float scaleX = W / static_cast<float>(tex.getSize().x);
  float scaleY = H / static_cast<float>(tex.getSize().y);
  float scale = std::max(scaleX, scaleY) * view::ui::layout::BgaScaleMultiplier;
  m_background.setScale({scale, scale});
  m_background.setOrigin({static_cast<float>(tex.getSize().x) / 2.f,
                          static_cast<float>(tex.getSize().y) / 2.f});
  m_background.setPosition({W / 2.f, H / 2.f});

  m_scoreLabel.setSize(W, static_cast<float>(view::ui::layout::ButtonFontSize) *
                              2.f);
  m_scoreLabel.setPosition(0.f, H * 0.55f);

  m_buttonList.relayout();
  float startX = (W - view::ui::layout::MenuButtonWidth) / 2.f;
  // On resize: re-anchor X position; Y is driven by animator (don't reset it).
  m_buttonList.setPosition(startX, m_buttonList.getPosition().y);
}

void GameOverState::handleEvent(const sf::Event &event) {
  if (!fontLoaded)
    return;
  m_buttonList.handleEvent(event);
}

void GameOverState::update(float deltaTime) {
  if (!fontLoaded)
    return;
  m_slideIn.update(deltaTime);
  m_buttonList.setPosition(m_buttonList.getPosition().x, m_slideIn.value());
  m_buttonList.update(deltaTime);
}

void GameOverState::render(sf::RenderTarget &window) {
  window.draw(m_background);
  if (!fontLoaded)
    return;

  m_scoreLabel.render(window);
  m_buttonList.render(window);
}

} // namespace controller