#ifndef VIEW_PLAYERRENDERER_H
#define VIEW_PLAYERRENDERER_H

#include "View/Base/SpriteEntityRenderer.h"

namespace model {
class Player;
}

namespace view {

// Draws Mario/Luigi from the characters spritesheet. Mario's forms each live in their own
// band of the sheet (normal, fire, star rows); Luigi borrows Mario's art. Fire and Star
// select the band, and the alpha channel only carries the post-damage invulnerability
// blink.
class PlayerRenderer : public SpriteEntityRenderer<model::Player> {
public:
    PlayerRenderer();

protected:
    void renderTyped(sf::RenderTarget& window, const model::Player& player,
                     const RenderContext& ctx) const override;

    // The post-damage invulnerability blink; nothing else tints the player.
    sf::Color characterTint(const model::Player& player) const override;
};

}

#endif
