#ifndef VIEW_ENTITYRENDERUTILS_H
#define VIEW_ENTITYRENDERUTILS_H

#include "Model/Map/TileMap.h"
#include "Model/Core/Vector2.h"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Sprite.hpp>

namespace view {

// World size of one source pixel: character frames are 16x32 while world tiles are
// 32x32, so sprites are scaled up 2x.
inline constexpr float SpriteScaleX = static_cast<float>(model::TileMap::TileWidth) / 16.0f;
inline constexpr float SpriteScaleY = static_cast<float>(model::TileMap::TileHeight) / 16.0f;

// Configure a character sprite so the source frame maps exactly onto the entity's box:
// the frame is stretched to the entity's size, so a *tight* frame (one that bounds the
// artwork with no padding) lines the sprite up with the hitbox by construction. This is
// why renderers pass tight frames rather than whole 16x32 atlas cells — a padded frame
// would misalign the art, and guessing the padding from the entity size cannot work when
// different cells pad differently.
//
// `mirrorHorizontally` is decided by the caller (see SpriteEntityRenderer), since which
// way a frame needs flipping depends on the spritesheet's own facing, not just the entity's.
inline void setupEntitySprite(sf::Sprite& sprite, const sf::IntRect& frame,
                              const model::Vector2& entitySize, bool mirrorHorizontally) {
    const float scaleX = entitySize.x / static_cast<float>(frame.size.x);
    const float scaleY = entitySize.y / static_cast<float>(frame.size.y);
    if (mirrorHorizontally) {
        sprite.setScale({-scaleX, scaleY});
        sprite.setOrigin({static_cast<float>(frame.size.x), 0.0f});
    } else {
        sprite.setScale({scaleX, scaleY});
        sprite.setOrigin({0.0f, 0.0f});
    }
}

}

#endif
