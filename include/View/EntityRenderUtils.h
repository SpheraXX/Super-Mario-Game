#ifndef VIEW_ENTITYRENDERUTILS_H
#define VIEW_ENTITYRENDERUTILS_H

#include "Model/TileMap.h"
#include "Model/Vector2.h"

#include <SFML/Graphics/Sprite.hpp>

namespace view {

// World size of one source pixel: character frames are 16x32 while world tiles are
// 32x32, so sprites are scaled up 2x.
inline constexpr float SpriteScaleX = static_cast<float>(model::TileMap::TileWidth) / 16.0f;
inline constexpr float SpriteScaleY = static_cast<float>(model::TileMap::TileHeight) / 16.0f;

// Configure a character sprite for one entity:
//  - the 16x32 source frame is scaled up to one world tile per 16 source pixels;
//  - the origin is lowered so the frame's bottom edge (the character's feet) sits exactly
//    on the entity's bottom edge, instead of the sprite hanging below the hitbox;
//  - the frame is mirrored horizontally when the entity faces left.
inline void setupEntitySprite(sf::Sprite& sprite, const model::Vector2& entitySize, bool facingRight) {
    const float originY = 32.0f - entitySize.y / SpriteScaleY;
    if (facingRight) {
        sprite.setScale({-SpriteScaleX, SpriteScaleY});
        sprite.setOrigin({16.0f, originY});
    } else {
        sprite.setScale({SpriteScaleX, SpriteScaleY});
        sprite.setOrigin({0.0f, originY});
    }
}

}

#endif
