#pragma once

#include "Controller/GameState.h"
#include "View/Effect/LerpAnimator.h"
#include "View/Effect/MetalShineEffect.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace controller {

class IntroState : public GameState {
public:
  IntroState();

  void onEnter() override;
  void onExit() override;
  void onDisplayModeChanged() override;
  void update(float dt) override;
  void render(sf::RenderTarget &target) override;
  void handleEvent(const sf::Event &event) override;

private:
  sf::Texture m_logoTexture;
  sf::Sprite m_logoSprite;
  sf::RectangleShape m_overlay;

  view::effect::MetalShineEffect m_shineEffect;
  view::effect::LerpAnimator m_alphaAnim;

  enum class Phase { FadeIn, Shining, FadeOut, Done };
  Phase m_phase;

  float m_timer;

  // Timings (compile-time constants, no per-instance storage)
  static constexpr float kFadeInDuration      = 2.0f;
  static constexpr float kShineDuration       = 1.5f;
  static constexpr float kFadeOutDuration     = 1.0f;

  // Visuals & Effects
  static constexpr float kLogoScale           = 0.5f;
  static constexpr float kAlphaMax            = 255.f;
  static constexpr float kAlphaMin            = 0.f;
  static constexpr float kInfiniteInterval    = 9999.f;
  static constexpr float kShineTriggerInterval = 0.f;
  static constexpr float kShineResetDelay     = 0.1f;
};

} // namespace controller
