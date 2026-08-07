#include "View/Level/FlagPoleRenderer.h"

#include "Model/Level/FlagPole.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <cmath>

namespace view {

namespace {
// Atlas coordinates in blocks.png (16x16 source tiles).
// Teal pipe-ish tile -> the pole; gold tile -> ball + pennant.
constexpr int PoleTileX = 21 * 16;  // 336
constexpr int PoleTileY = 13 * 16;  // 208
constexpr int GoldTileX = 14 * 16;  // 224
constexpr int GoldTileY = 10 * 16;  // 160
}

FlagPoleRenderer::FlagPoleRenderer()
    : textureLoaded(texture.loadFromFile("assets/blocks.png")) {
    texture.setSmooth(false);
}

void FlagPoleRenderer::renderTyped(sf::RenderTarget& window,
                                   const model::FlagPole& pole) const {
    if (!textureLoaded) return;

    const sf::Vector2f pos{pole.getPosition().x, pole.getPosition().y};
    const sf::Vector2f size{pole.getSize().x, pole.getSize().y};

    // The pole: the source tile is stretched onto the entity's thin, tall box.
    sf::Sprite poleSprite(texture);
    poleSprite.setTextureRect({{PoleTileX, PoleTileY}, {16, 16}});
    poleSprite.setScale({size.x / 16.0f, size.y / 16.0f});
    poleSprite.setPosition({std::round(pos.x), std::round(pos.y)});
    window.draw(poleSprite);

    // Gold ball on top (centred over the 8px-wide pole).
    sf::Sprite ball(texture);
    ball.setTextureRect({{GoldTileX, GoldTileY}, {16, 16}});
    ball.setScale({2.0f, 2.0f});
    ball.setPosition({std::round(pos.x - 4.0f), std::round(pos.y - 32.0f)});
    window.draw(ball);

    // Pennant waving on the pole.
    sf::Sprite flag(texture);
    flag.setTextureRect({{GoldTileX, GoldTileY}, {16, 16}});
    flag.setScale({2.0f, 2.0f});
    flag.setPosition({std::round(pos.x + 8.0f), std::round(pos.y + size.y * 0.4f)});
    window.draw(flag);
}

}
