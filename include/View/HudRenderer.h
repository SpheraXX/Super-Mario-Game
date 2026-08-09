#ifndef VIEW_HUDRENDERER_H
#define VIEW_HUDRENDERER_H

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <string>

namespace view {

// Draws the on-screen HUD on the fixed, non-scrolling view: the level label and control
// hints, plus the score / coins / lives / time counters stacked in the top-right corner.
// Owns its font so no game state needs one, and reads the counters straight from
// GameManager rather than being fed them.
class HudRenderer {
public:
    HudRenderer();
    void render(sf::RenderTarget& window) const;

private:
    // Shared styling for every HUD string. `x` is the left edge, or the right edge when
    // rightAligned — which is what keeps the corner column flush as numbers change width.
    void drawText(sf::RenderTarget& window, const std::string& content, float x, float y,
                  unsigned int size, bool rightAligned) const;

    sf::Font font;
    bool fontLoaded;
};

}

#endif
