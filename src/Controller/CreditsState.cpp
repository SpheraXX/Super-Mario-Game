#include "Controller/CreditsState.h"
#include "Controller/AppEngine.h"
#include "View/AssetManager.h"
#include "View/UI/UITheme.h"
#include "Model/Core/LogManager.h"
#include "ext/json.hpp"

#include <fstream>
#include <iostream>

using json = nlohmann::json;

namespace controller {

CreditsState::CreditsState()
    : m_bgaSprite(view::AssetManager::instance().getTexture(
          "assets/images/bga_mainmenu.png")),
      m_titleSprite(view::AssetManager::instance().getTexture(
          "assets/images/bga_mainmenu_title.png")),
      m_backButton(view::AssetManager::instance().getUiFont(), "BACK",
                   view::ui::layout::ButtonFontSize,
                   {0.f, 0.f},
                   {view::ui::layout::MenuButtonWidth, view::ui::layout::MenuButtonHeight}),
      m_logoSprite(m_logoTexture) {

  m_bgaSprite.setColor(view::ui::theme::BgaDimMenu);
  m_shineEffect.setInterval(view::ui::layout::CreditsShineInterval);

  m_backButton.setOnClick([this]() { manager->popState(); });

  // Load Group Logo
  if (!m_logoTexture.loadFromFile("assets/images/logo.png")) {
    model::LogManager::instance().error("[CreditsState] Failed to load logo.png");
  } else {
    m_logoSprite.setTexture(m_logoTexture, true);
    m_logoSprite.setOrigin(
        {m_logoTexture.getSize().x / 2.f, m_logoTexture.getSize().y / 2.f});
    m_logoSprite.setScale(
        {view::ui::layout::CreditsLogoScale, view::ui::layout::CreditsLogoScale});
  }

  loadCredits();
  layoutCredits();

  // Positions that depend on screen size
  onDisplayModeChanged();
}

void CreditsState::onDisplayModeChanged() {
  float W = static_cast<float>(AppEngine::screenWidth());
  float H = static_cast<float>(AppEngine::ScreenHeight);

  // Background
  float scale = H / static_cast<float>(m_bgaSprite.getTexture().getSize().y);
  m_bgaSprite.setScale({scale, scale});
  m_bgaSprite.setOrigin({m_bgaSprite.getTexture().getSize().x / 2.f,
                         m_bgaSprite.getTexture().getSize().y / 2.f});
  m_bgaSprite.setPosition({W / 2.f, H / 2.f});

  // Title
  float titleTargetW = W * view::ui::layout::TitleWidthRatio;
  float titleScale = titleTargetW / m_titleSprite.getTexture().getSize().x;
  m_titleSprite.setScale({titleScale, titleScale});
  m_titleSprite.setOrigin({m_titleSprite.getTexture().getSize().x / 2.f,
                           m_titleSprite.getTexture().getSize().y / 2.f});
  m_titleSprite.setPosition({W / 2.f, H * view::ui::layout::TitleYRatio});

  // Back button
  float btnH = view::ui::layout::MenuButtonHeight;
  float gap  = view::ui::layout::MenuButtonGap;
  m_backButton.setSize(view::ui::layout::MenuButtonWidth, btnH);
  m_backButton.setPosition(gap, H - btnH - gap);
}

void CreditsState::loadCredits() {
  std::ifstream file("assets/data/credits.json");
  if (!file.is_open()) {
    model::LogManager::instance().error("[CreditsState] Failed to open assets/data/credits.json");
    return;
  }

  try {
    json j;
    file >> j;

    const sf::Font &font = view::AssetManager::instance().getUiFont();

    if (j.contains("sections")) {
      for (const auto &section : j["sections"]) {
        std::string title = section["title"];

        sf::Text titleText(font, title, view::ui::layout::TitleFontSize);
        titleText.setFillColor(sf::Color::Yellow);
        m_creditsText.push_back({titleText, 0.f});

        for (const auto &line : section["lines"]) {
          std::string textLine = line;
          sf::Text normalText(font, textLine, view::ui::layout::ButtonFontSize);
          normalText.setFillColor(sf::Color::White);
          m_creditsText.push_back({normalText, 0.f});
        }

        // Empty space after section
        sf::Text emptyText(font, "", view::ui::layout::TitleFontSize);
        m_creditsText.push_back({emptyText, 0.f});
      }
    }
  } catch (const std::exception &e) {
    model::LogManager::instance().error("[CreditsState] JSON parse error: " + std::string(e.what()));
  }
}

void CreditsState::layoutCredits() {
  float currentY = 0.f;

  for (auto &item : m_creditsText) {
    sf::FloatRect bounds = item.text.getLocalBounds();
    item.text.setOrigin({bounds.position.x + bounds.size.x / 2.f,
                         bounds.position.y + bounds.size.y / 2.f});
    item.text.setPosition(
        {AppEngine::screenWidth() / 2.f, 0.f}); // Y updated in render

    item.relativeY = currentY;
    currentY += view::ui::layout::CreditsLineSpacing;
  }

  // Extra space before logo
  currentY += view::ui::layout::CreditsLogoPreSpace;

  m_logoRelativeY =
      currentY + (m_logoTexture.getSize().y * m_logoSprite.getScale().y) / 2.f;

  // Total height includes gap for looping
  m_totalHeight =
      m_logoRelativeY +
      (m_logoTexture.getSize().y * m_logoSprite.getScale().y) / 2.f +
      static_cast<float>(AppEngine::ScreenHeight);
}

void CreditsState::onEnter() { m_scrollY = static_cast<float>(AppEngine::ScreenHeight); }

void CreditsState::onExit() {}

void CreditsState::update(float dt) {
  m_shineEffect.update(dt);
  m_backButton.update(dt);

  m_scrollY -= view::ui::layout::CreditsScrollSpeed * dt;

  if (m_scrollY < -m_totalHeight + static_cast<float>(AppEngine::ScreenHeight)) {
    m_scrollY = static_cast<float>(AppEngine::ScreenHeight);
  }
}

void CreditsState::render(sf::RenderTarget &target) {
  target.draw(m_bgaSprite);

  // Draw scrolling texts
  for (auto &item : m_creditsText) {
    float drawY = m_scrollY + item.relativeY;

    if (drawY > view::ui::layout::CreditsCullMarginTop &&
        drawY < static_cast<float>(AppEngine::ScreenHeight) + view::ui::layout::CreditsCullMarginBottom) {
      item.text.setPosition({AppEngine::screenWidth() / 2.f, drawY});
      target.draw(item.text);
    }
  }

  // Draw scrolling logo
  float logoDrawY = m_scrollY + m_logoRelativeY;
  if (logoDrawY > view::ui::layout::CreditsCullMarginTop &&
      logoDrawY < static_cast<float>(AppEngine::ScreenHeight) + view::ui::layout::CreditsCullLogoMargin) {
    m_logoSprite.setPosition({AppEngine::screenWidth() / 2.f, logoDrawY});
    target.draw(m_logoSprite);
  }

  // Draw fixed elements on top
  m_shineEffect.draw(target, m_titleSprite);
  m_backButton.render(target);
}

void CreditsState::handleEvent(const sf::Event &event) {
  if (m_backButton.handleEvent(event)) {
    return;
  }

  if (const auto *p = event.getIf<sf::Event::KeyPressed>()) {
    if (p->code == sf::Keyboard::Key::Escape) {
      manager->popState();
      return;
    }
  }
}

} // namespace controller
