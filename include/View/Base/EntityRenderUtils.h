#ifndef VIEW_ENTITYRENDERUTILS_H
#define VIEW_ENTITYRENDERUTILS_H

#include "Model/Map/TileMap.h"
#include "Model/Core/Vector2.h"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Sprite.hpp>

namespace view {

// World size of one source pixel. The tile is the same 16px as the source art, so
// these are 1.0 and a frame draws at its native size; they stay expressed in terms of
// TileWidth/TileHeight so a future change of tile size still rescales the art.
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
//
// `flipVertically` inverts the frame on the vertical axis (a kicked enemy spinning upside
// down). A negative scale draws around the opposite corner, so a flipped axis also moves
// the origin to the far edge of the frame — the sprite then stays inside its box while
// inverted rather than being displaced by its own height.
inline void setupEntitySprite(sf::Sprite& sprite, const sf::IntRect& frame,
                              const model::Vector2& entitySize, bool mirrorHorizontally,
                              bool flipVertically = false) {
    const float baseScaleX = entitySize.x / static_cast<float>(frame.size.x);
    const float baseScaleY = entitySize.y / static_cast<float>(frame.size.y);
    const float scaleX = mirrorHorizontally ? -baseScaleX : baseScaleX;
    const float scaleY = flipVertically ? -baseScaleY : baseScaleY;
    const float originX = mirrorHorizontally ? static_cast<float>(frame.size.x) : 0.0f;
    const float originY = flipVertically ? static_cast<float>(frame.size.y) : 0.0f;
    sprite.setScale({scaleX, scaleY});
    sprite.setOrigin({originX, originY});
}

}

#endif
