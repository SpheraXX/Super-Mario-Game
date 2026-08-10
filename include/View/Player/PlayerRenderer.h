#ifndef VIEW_PLAYERRENDERER_H
#define VIEW_PLAYERRENDERER_H

#include "View/Base/SpriteEntityRenderer.h"

#include <SFML/Graphics/Color.hpp>

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

private:
    // Warm tint that sets Fire Mario apart from Super; also applies to the crouch/death
    // frames, since Fire keeps its identity in every pose.
    static sf::Color fireTint() {
        return sf::Color(255, 175, 150);
    }

    sf::Color characterTint(const model::Player& player) const override;
};

}

#endif
