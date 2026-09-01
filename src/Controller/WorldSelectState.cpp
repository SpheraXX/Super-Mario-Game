#include "Controller/WorldSelectState.h"
#include "Controller/AppEngine.h"
#include "Controller/LevelSelectState.h"
#include "Controller/MainMenuState.h"
#include "Model/Core/WorldManager.h"
#include "Model/Save/SaveManager.h"
#include "View/AssetManager.h"
#include "View/UI/UITheme.h"
#include <algorithm>
#include <iostream>

namespace controller {

WorldSelectState::WorldSelectState(int initialFocusIndex)
    : m_focusIndex(initialFocusIndex),
      m_background(view::AssetManager::instance().getTexture("assets/images/bga_selection.png")) {
  m_background.setColor(view::ui::theme::BgaDimMenu);

  // Setup back button
  const sf::Font &font = view::AssetManager::instance().getUiFont();
  m_backButton = view::ui::UIButton(
      font, "BACK", view::ui::layout::ButtonFontSize, {0, 0},
      {view::ui::layout::MenuButtonWidth, view::ui::layout::MenuButtonHeight});
  m_backButton.setOnClick(
      [this]() { manager->replaceState(std::make_unique<MainMenuState>()); });
}

void WorldSelectState::onEnter() {
  // BUG-3 fix: clear vectors to prevent duplication if state is re-entered
  m_worldSprites.clear();
  m_worldIds.clear();

  const auto& worlds = model::WorldManager::instance().getWorlds();
  for (const auto &w : worlds) {
    sf::Sprite sprite(
        view::AssetManager::instance().getTexture(w.previewImage));
    // origin at center
    sprite.setOrigin({sprite.getLocalBounds().size.x / 2.f,
                      sprite.getLocalBounds().size.y / 2.f});
    m_worldSprites.push_back(sprite);
    m_worldIds.push_back(w.id);
  }

  // Determine unlocked worlds (simplification: assume all unlocked if none
  // specified, or just let them play) Here we can use SaveManager to check if
  // they are unlocked, but for now let's just show them.

  // m_focusIndex is already set by the constructor
  // Ensure m_targetX is instantly set to avoid sliding from 0
  float W = static_cast<float>(AppEngine::screenWidth());
  float cardSpacing = W * 0.28f;
  m_targetX = -static_cast<float>(m_focusIndex) * cardSpacing;
  m_slideAnimator = view::effect::LerpAnimator(m_targetX, m_targetX, 0.4f,
                                               view::effect::Easing::OutCubic);

  onDisplayModeChanged();
}

void WorldSelectState::onDisplayModeChanged() {
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

  // Layout buttons
  float btnW = view::ui::layout::MenuButtonWidth;
  float btnH = view::ui::layout::MenuButtonHeight;
  float padding = view::ui::layout::MenuButtonGap;

  m_backButton.setSize(btnW, btnH);
  m_backButton.setPosition(padding, H - btnH - padding);

  updateCarousel();
  updateCardLayout(); // Fix 1-frame flicker bug
}

void WorldSelectState::updateCarousel() {
  float W = static_cast<float>(AppEngine::screenWidth());
  float cardSpacing = W * 0.28f; // distance between cards

  m_targetX = -static_cast<float>(m_focusIndex) * cardSpacing;

  // Start lerp
  float currentX = m_slideAnimator.value();
  m_slideAnimator = view::effect::LerpAnimator(currentX, m_targetX, 0.4f,
                                               view::effect::Easing::OutCubic);
}

void WorldSelectState::handleEvent(const sf::Event &event) {
  m_backButton.handleEvent(event);

  if (const auto *mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()) {
    if (mouseBtn->button == sf::Mouse::Button::Left) {
      sf::Vector2f logicalPos = AppEngine::windowToLogical(mouseBtn->position);

      // Check bounding boxes from front to back, or just iterate
      for (size_t i = 0; i < m_worldSprites.size(); ++i) {
        if (m_worldSprites[i].getGlobalBounds().contains(logicalPos)) {
          if (i == static_cast<size_t>(m_focusIndex)) {
            manager->replaceState(
                std::make_unique<LevelSelectState>(m_worldIds[m_focusIndex]));
          } else {
            m_focusIndex = static_cast<int>(i);
            updateCarousel();
          }
          break;
        }
      }
    }
  }

  // BUG-6 fix: use single getIf<> instead of is<> + getIf<>
  if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
    if (key->code == sf::Keyboard::Key::Escape) {
      manager->replaceState(std::make_unique<MainMenuState>());
    }
  }
}

void WorldSelectState::update(float dt) {
  m_slideAnimator.update(dt);
  m_backButton.update(dt);

  updateCardLayout();
}

void WorldSelectState::updateCardLayout() {
  float W = static_cast<float>(AppEngine::screenWidth());
  float H = static_cast<float>(AppEngine::ScreenHeight);
  float cardSpacing = W * 0.28f;
  float centerX = W / 2.f;
  float centerY = H / 2.f;

  for (size_t i = 0; i < m_worldSprites.size(); ++i) {
    float logicalX = i * cardSpacing + m_slideAnimator.value();
    float screenPos = centerX + logicalX;
    m_worldSprites[i].setPosition({screenPos, centerY});

    // Scale and color based on distance from center
    float dist = std::abs(logicalX);
    float animScale = 1.0f - std::min(dist / W, 0.5f);

    float texWidth = m_worldSprites[i].getTexture().getSize().x;
    float baseScale = (W * 0.3f) / texWidth; // 30% of screen width
    float finalScale = baseScale * animScale;

    m_worldSprites[i].setScale({finalScale, finalScale});

    if (i == static_cast<size_t>(m_focusIndex)) {
      m_worldSprites[i].setColor(sf::Color::White);
    } else {
      m_worldSprites[i].setColor(sf::Color(150, 150, 150));
    }
  }
}

void WorldSelectState::render(sf::RenderTarget &target) {
  target.draw(m_background);

  for (const auto &sprite : m_worldSprites) {
    target.draw(sprite);
  }

  m_backButton.render(target);
}

} // namespace controller
