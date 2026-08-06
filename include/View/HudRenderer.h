#ifndef VIEW_HUDRENDERER_H
#define VIEW_HUDRENDERER_H

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace view {

// Draws the on-screen HUD (level label and control hints) on the fixed, non-scrolling
// view. Owns its font so no game state needs one.
class HudRenderer {
public:
    HudRenderer();
    void render(sf::RenderTarget& window) const;

private:
    sf::Font font;
    bool fontLoaded;
};

}

#endif
