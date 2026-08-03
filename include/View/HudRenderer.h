#ifndef VIEW_HUDRENDERER_H
#define VIEW_HUDRENDERER_H

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace view {

// Draws the on-screen HUD (level label and control hints) on the fixed, non-scrolling
// view. Owns its font so no game state needs one.
class HudRenderer {
public:
    HudRenderer();
    void render(sf::RenderWindow& window) const;

private:
    sf::Font font;
    bool fontLoaded;
};

}

#endif
