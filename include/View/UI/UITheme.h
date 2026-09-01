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
constexpr sf::Color ColorOverlay      = sf::Color(0, 0, 0, 140);
constexpr sf::Color ScreenBackground  = sf::Color(20, 20, 30);
constexpr sf::Color GameOverBackground = sf::Color(60, 10, 10);

// Disabled state
constexpr sf::Color ColorDisabled     = sf::Color(100, 100, 100);
constexpr sf::Color ColorTextDisabled = sf::Color(150, 150, 150);

// Icon Tints
constexpr sf::Color ColorIconNormal = sf::Color(255, 255, 255);
constexpr sf::Color ColorIconWaiting = sf::Color(150, 150, 150);
constexpr sf::Color ColorIconConflict = sf::Color(255, 100, 100);

// Invisible Mask (for overlapping buttons)
constexpr sf::Color ColorMaskNormal = sf::Color::Transparent;
constexpr sf::Color ColorMaskHovered = sf::Color(255, 255, 255, 60);

// Special colors for UICycleButton (slightly darker)
constexpr sf::Color CycleNormal  = sf::Color(40, 40, 70);
constexpr sf::Color CycleHovered = sf::Color(70, 70, 110);

// Scrollbar colors
constexpr sf::Color ScrollbarNormal  = sf::Color(100, 100, 100, 100);
constexpr sf::Color ScrollbarHovered = sf::Color(180, 180, 180, 200);

// Slider colors
constexpr sf::Color SliderTrack = sf::Color(80, 80, 80);
constexpr sf::Color SliderFill  = sf::Color(100, 100, 100);
constexpr sf::Color SliderKnob  = sf::Color(120, 120, 120);

// Popup / Modal colors
constexpr sf::Color PopupBackground = sf::Color(0, 0, 0, 200);
constexpr sf::Color PopupBox        = sf::Color(40, 40, 50, 240);
constexpr sf::Color PopupBorder     = sf::Color(200, 50, 50);
constexpr sf::Color PopupTitleText  = sf::Color(255, 100, 100);

// Dimming level cho từng loại màn hình BGA
constexpr sf::Color BgaDimMenu    = sf::Color(160, 160, 160); // 63% - đủ nhìn logo
constexpr sf::Color BgaDimOptions = sf::Color(130, 130, 130); // 51% - tối hơn vì nhiều chữ

} // namespace theme

namespace layout {
// Standardized sizes to avoid magic numbers across states
constexpr float MenuButtonWidth  = 160.f;
constexpr float MenuButtonHeight = 20.f;
constexpr float MenuButtonGap    = 8.f;

constexpr float SmallButtonWidth = 80.f;
constexpr float SmallButtonHeight = 20.f;

constexpr unsigned int TitleFontSize  = 16u;
constexpr unsigned int ButtonFontSize = 8u;
constexpr unsigned int SmallFontSize  = 6u;

// BGA scaling constants
constexpr float BgaScaleMultiplier = 2.f;   // Scale factor beyond screen-height fit
constexpr float TitleWidthRatio    = 0.4f;  // Title image target width = 40% of screen
} // namespace layout

} // namespace ui
} // namespace view

#endif
