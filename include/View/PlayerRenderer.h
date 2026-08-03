#ifndef VIEW_PLAYERRENDERER_H
#define VIEW_PLAYERRENDERER_H

#include "View/SpriteEntityRenderer.h"

namespace model {
class Player;
}

namespace view {

// Draws Mario/Luigi from the characters spritesheet.
class PlayerRenderer : public SpriteEntityRenderer<model::Player> {
public:
    PlayerRenderer();

protected:
    void renderTyped(sf::RenderWindow& window, const model::Player& player) const override;
};

}

#endif
