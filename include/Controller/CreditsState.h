#pragma once

#include "Controller/GameState.h"
#include "View/Effect/MetalShineEffect.h"
#include "View/UI/UIButton.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <vector>
#include <string>

namespace controller {

class CreditsState : public GameState {
public:
  CreditsState();

  void onEnter() override;
  void onExit() override;
  void onDisplayModeChanged() override;
  void update(float dt) override;
  void render(sf::RenderTarget &target) override;
  void handleEvent(const sf::Event &event) override;

private:
  void loadCredits();
  void layoutCredits();

  sf::Sprite m_bgaSprite;
  sf::Sprite m_titleSprite;
  view::effect::MetalShineEffect m_shineEffect;

  view::ui::UIButton m_backButton;

  struct CreditText {
    sf::Text text;
    float relativeY;
  };

  std::vector<CreditText> m_creditsText;

  sf::Texture m_logoTexture;
  sf::Sprite m_logoSprite;
  float m_logoRelativeY = 0.f;

  float m_scrollY    = 0.f;
  float m_totalHeight = 0.f;
};

} // namespace controller
