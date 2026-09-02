#include "Controller/IntroState.h"
#include "Controller/AppEngine.h"
#include "Controller/MainMenuState.h"
#include "Model/Core/LogManager.h"

#include <cstdint>

namespace controller {

IntroState::IntroState() : m_phase(Phase::FadeIn), m_timer(0.f), m_logoSprite(m_logoTexture) {
  if (!m_logoTexture.loadFromFile("assets/images/logo.png")) {
    model::LogManager::instance().error("[IntroState] Failed to load logo.png");
  } else {
    m_logoSprite.setTexture(m_logoTexture, true);
    m_logoSprite.setOrigin({m_logoTexture.getSize().x / 2.f,
                           m_logoTexture.getSize().y / 2.f});
    m_logoSprite.setScale({kLogoScale, kLogoScale});
  }

  m_shineEffect.setInterval(kInfiniteInterval);
  m_alphaAnim = view::effect::LerpAnimator(kAlphaMax, kAlphaMin, kFadeInDuration, view::effect::Easing::Linear);

  // Overlay size and logo position depend on screen dimensions
  onDisplayModeChanged();
}

void IntroState::onDisplayModeChanged() {
  float W = static_cast<float>(AppEngine::screenWidth());
  float H = static_cast<float>(AppEngine::ScreenHeight);
  m_overlay.setSize({W, H});
  m_logoSprite.setPosition({W / 2.f, H / 2.f});
}

void IntroState::onEnter() {
  m_phase = Phase::FadeIn;
  m_timer = 0.f;
  m_alphaAnim = view::effect::LerpAnimator(kAlphaMax, kAlphaMin, kFadeInDuration, view::effect::Easing::Linear);
  m_overlay.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(kAlphaMax)));
}

void IntroState::onExit() {}

void IntroState::update(float dt) {
  m_shineEffect.update(dt);

  switch (m_phase) {
  case Phase::FadeIn: {
    m_alphaAnim.update(dt);
    if (m_alphaAnim.isDone()) {
      m_phase = Phase::Shining;
      m_timer = 0.f;
      
      // Trigger the shine immediately by setting interval to 0 temporarily
      m_shineEffect.setInterval(kShineTriggerInterval); 
    }
    m_overlay.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(m_alphaAnim.value())));
    break;
  }
  case Phase::Shining: {
    m_timer += dt;
    // Reset interval so it doesn't loop
    if (m_timer > kShineResetDelay) {
      m_shineEffect.setInterval(kInfiniteInterval);
    }
    
    if (m_timer >= kShineDuration) {
      m_phase = Phase::FadeOut;
      m_alphaAnim = view::effect::LerpAnimator(kAlphaMin, kAlphaMax, kFadeOutDuration, view::effect::Easing::Linear);
    }
    break;
  }
  case Phase::FadeOut: {
    m_alphaAnim.update(dt);
    if (m_alphaAnim.isDone()) {
      m_phase = Phase::Done;
    }
    m_overlay.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(m_alphaAnim.value())));
    break;
  }
  case Phase::Done: {
    manager->replaceState(std::make_unique<MainMenuState>());
    break;
  }
  }
}

void IntroState::render(sf::RenderTarget &target) {
  target.clear(sf::Color::Black);
  m_shineEffect.draw(target, m_logoSprite);
  target.draw(m_overlay);
}

void IntroState::handleEvent(const sf::Event &event) {
  // Allow skipping the intro
  if (event.is<sf::Event::KeyPressed>() || event.is<sf::Event::MouseButtonPressed>()) {
    m_phase = Phase::Done;
  }
}

} // namespace controller
