#include "Model/Settings.h"
#include <SFML/Window/Keyboard.hpp>

namespace model {

Settings Settings::defaults() {
  Settings s;
  s.fullscreen = false;
  s.ratio = AspectRatio::Ratio4x3;
  s.resolutionIndex = 0; // 800x600 for 4x3 by default
  s.quality = GraphicsQuality::Medium;
  s.luigiSelected = false;

  s.masterVolume = 100;
  s.musicVolume = 80;
  s.sfxVolume = 100;

  s.language = Language::English;

  // Mapping virtual keys directly to SFML's default keyboard buttons
  s.keyMoveLeft = static_cast<int>(sf::Keyboard::Key::Left);
  s.keyMoveRight = static_cast<int>(sf::Keyboard::Key::Right);
  s.keyJump = static_cast<int>(sf::Keyboard::Key::Up);
  s.keyRun = static_cast<int>(sf::Keyboard::Key::LShift);
  s.keyAttack = static_cast<int>(sf::Keyboard::Key::Z);
  s.keyCrouch = static_cast<int>(sf::Keyboard::Key::Down);
  s.keyPause = static_cast<int>(sf::Keyboard::Key::Escape);
  s.keyCycleDisplay = static_cast<int>(sf::Keyboard::Key::F2);

  s.controlSlot = 0;
  return s;
}

} // namespace model
