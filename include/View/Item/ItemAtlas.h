#ifndef VIEW_ITEMATLAS_H
#define VIEW_ITEMATLAS_H

#include <SFML/Graphics/Rect.hpp>

namespace view {

// Frame layout of the item spritesheet, in source pixels. Like the enemy atlas, every
// item frame lives here so the coordinates are defined in exactly one place.
//
// The sheet has one 16x16 item per cell. The cells reported by the artist were
// (2,10)-(19,27) for the Super Mushroom and (42,10)-(59,27) for the Fire Flower, but the
// outer 1px ring of each cell is fully transparent — the actual artwork occupies only
// 16x16 pixels. Frames below are cut to that tight art, not the padded cell: the renderer
// stretches the frame onto the entity's 32x32 box, so a padded frame would shrink the
// sprite by ~11% and leave it floating off its hitbox.
//
// The sheet carries a real alpha channel, so no colour-key masking is needed at load time.
namespace atlas {

inline const char* const ItemSheet =
    "assets/Item/Super Mushroom & Fire Flower (SMB1 Design, SMW-Style).png";

inline const sf::IntRect Mushroom({3, 11}, {16, 16});     // world 32x32
inline const sf::IntRect FireFlower({43, 11}, {16, 16});  // world 32x32
inline const sf::IntRect Starman({114, 10}, {17, 18});    // world 32x32

}

}

#endif
