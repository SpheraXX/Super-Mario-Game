#ifndef VIEW_PLAYERRENDERER_H
#define VIEW_PLAYERRENDERER_H

#include "View/Base/SpriteEntityRenderer.h"

namespace model {
class Player;
}

namespace view {

// Draws Mario/Luigi from the characters spritesheet.
class PlayerRenderer : public SpriteEntityRenderer<model::Player> {
public:
    PlayerRenderer();

protected:
    void renderTyped(sf::RenderTarget& window, const model::Player& player) const override;
};

}

#endif
