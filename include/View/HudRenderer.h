#ifndef VIEW_HUDRENDERER_H
#define VIEW_HUDRENDERER_H

#include "View/HudData.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace view {

// Draws the classic SMB top bar on the fixed, non-scrolling view:
//
//   MARIO    COINS    WORLD    TIME
//   110600   67       1-1      369
//
// The values come from a HudData snapshot built by the controller each frame, so the
// renderer never reaches into the game state itself. Labels and values share the same
// 8px size and left-aligned columns, so the monospaced pixel font makes the four
// groups perfectly equidistant.
class HudRenderer {
public:
    HudRenderer();
    void render(sf::RenderTarget& window, const HudData& data) const;

private:
    sf::Font font;
    bool fontLoaded;
    // Fitted lazily against the view width in force, and re-fitted only when that width
    // changes (cycling the window size changes it). Mutable because the fit is a render-time
    // cache, not observable state: render() stays const.
    mutable unsigned int hintSize = 6;
    mutable float fittedForWidth = -1.0f;
};

}

#endif
