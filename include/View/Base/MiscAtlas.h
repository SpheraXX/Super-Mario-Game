#ifndef VIEW_MISCATLAS_H
#define VIEW_MISCATLAS_H

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>

namespace view {

// Frame layout of assets/misc.png, plus the two castle-furniture tiles that live on the
// main Mario sheet. Same contract as View/Enemy/EnemyAtlas.h: every rect in use is named
// here so no renderer hardcodes its own, and every rect TIGHTLY bounds its artwork,
// because the renderer stretches the frame onto whatever box it is drawn into.
//
// misc.png has no alpha. Two different backdrops are keyed out of it, and which one
// applies depends on the region of the sheet, not on the sheet itself:
//
//   MiscColorKey    pure black — the fireballs, the Cheep Cheep, the Mushroom Retainer.
//                   Safe to key: none of those sprites uses pure black internally (the
//                   Retainer's outline is (32,32,32), a hair off black, precisely so it
//                   survives this).
//   HammerColorKey  the flat blue patch the four hammer poses sit on. The hammer DOES
//                   outline itself in black, so keying black there would gut it.
namespace atlas {

inline const char* const MiscSheet = "assets/misc.png";
inline const sf::Color MiscColorKey(0, 0, 0);
inline const sf::Color HammerColorKey(0, 136, 255);

// --- Firebar ------------------------------------------------------------------------
// One link of a firebar: an 8x8 flame in four rotations. The bar is a LINE of these
// (see Model/Level/FirebarBall.h) sharing a pivot, so this is the art for a single ball,
// not for the whole bar. Frames are in spin order.
//
// The four cells sit on a regular 15px pitch — x in {26, 41}, y in {150, 165} — and the
// rects MUST be taken from that grid rather than from each pose's tight bounding box.
// Every pose trails a few detached spark pixels one or two columns clear of the flame
// body, so a bbox-derived rect drops whichever sparks fall outside it: it shaved the left
// column off the (41,150) pose and the top row off the (41,165) one, which is what made
// some balls look like they had a piece missing.
inline const sf::IntRect FirebarBall[4] = {
    {{26, 150}, {8, 8}},
    {{41, 150}, {8, 8}},
    {{26, 165}, {8, 8}},
    {{41, 165}, {8, 8}},
};

// --- Lava bubble (the big fireball that leaps out of the lava) -----------------------
// The artwork is 14 wide inside a 16-wide cell, so the rect is taken as the full 16x16
// with a symmetric 1px margin rather than tight: that keeps the world box a clean tile
// and the 1px is not visible against the flame's round silhouette.
inline const sf::IntRect LavaBubble({0, 154}, {16, 16});

// --- Cheep Cheep --------------------------------------------------------------------
// Two swim poses (fins up / fins down). The artwork faces LEFT, which is what the
// renderers' `sourceFacesRight = false` default already assumes.
inline const sf::IntRect CheepCheep[2] = {
    {{240, 184}, {16, 16}},
    {{270, 184}, {16, 16}},
};

// --- Mushroom Retainer (Toad) -------------------------------------------------------
// 16x24, not 16x32: the bottom 8 rows of that cell are backdrop, and including them
// would squash the sprite when it is stretched onto the model's box.
inline const sf::IntRect MushroomRetainer({244, 272}, {16, 24});

// --- Hammer Bro's hammer ------------------------------------------------------------
// Four poses of one spin, alternating shape as the head goes over: the two horizontal
// poses are 14x8 and the two vertical ones are 8x16. They are NOT a common size, so the
// renderer draws each into its own box centred on the hammer's 16x16 entity rather than
// stretching every pose onto the full tile.
inline const sf::IntRect HammerSpin[4] = {
    {{264,  87}, {14, 8}},
    {{284,  87}, {8, 16}},
    {{278, 108}, {14, 8}},
    {{264, 100}, {8, 16}},
};

// --- Castle furniture ---------------------------------------------------------------
// The bridge deck and the axe are terrain//set-dressing art, so they live on the main
// Mario sheet with the rest of the tiles rather than on misc.png. Both cells are cut
// with a black backdrop (they are drawn against a castle's black sky in the original),
// so black is the key here even though the same sheet uses (148,148,255) elsewhere.
inline const char* const CastleSheet = "assets/super_mario_asset.png";
inline const sf::Color CastleColorKey(0, 0, 0);
inline const sf::IntRect ChainLink({548, 476}, {16, 16});
inline const sf::IntRect ChainAxe({580, 460}, {16, 16});

}
}

#endif
