#ifndef VIEW_UI_UITHEME_H
#define VIEW_UI_UITHEME_H

#include <SFML/Graphics/Color.hpp>

namespace view {
namespace ui {
namespace theme {

// Normal button colors
constexpr sf::Color ColorNormal  = sf::Color(60, 60, 80);
constexpr sf::Color ColorHovered = sf::Color(100, 100, 140);
constexpr sf::Color ColorText    = sf::Color(255, 255, 255);

// Warning states (e.g., waiting for key press)
constexpr sf::Color ColorWarningNormal  = sf::Color(150, 150, 50); // Yellow
constexpr sf::Color ColorWarningHovered = sf::Color(180, 180, 80);

// Error states (e.g., key conflict)
constexpr sf::Color ColorErrorNormal  = sf::Color(150, 50, 50); // Red
constexpr sf::Color ColorErrorHovered = sf::Color(180, 80, 80);

// Success states (e.g., settings changed, ready to apply)
constexpr sf::Color ColorSuccessNormal  = sf::Color(50, 150, 200); // Light blue
constexpr sf::Color ColorSuccessHovered = sf::Color(80, 180, 230);

// Background dimming
constexpr sf::Color ColorOverlay = sf::Color(0, 0, 0, 140);

// Special colors for UICycleButton (slightly darker)
constexpr sf::Color CycleNormal  = sf::Color(40, 40, 70);
constexpr sf::Color CycleHovered = sf::Color(70, 70, 110);

// Scrollbar colors
constexpr sf::Color ScrollbarNormal  = sf::Color(100, 100, 100, 100);
constexpr sf::Color ScrollbarHovered = sf::Color(180, 180, 180, 200);

} // namespace theme
} // namespace ui
} // namespace view

#endif
