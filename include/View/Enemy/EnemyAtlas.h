#ifndef VIEW_ENEMYATLAS_H
#define VIEW_ENEMYATLAS_H

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>

namespace view {

// Frame layout of the enemy spritesheet, in source pixels. Every enemy and projectile frame
// in the game is named here so the coordinates live in exactly one place — a renderer that
// hardcodes its own rect is the thing that goes stale when the sheet is re-cut.
//
// Frames are *tight*: each rect bounds its artwork with no padding, because the renderer
// stretches the frame onto the entity's box and any padding shows up as the sprite sitting
// off its hitbox.
//
// The sheet has no alpha channel; the background is a flat key colour that is masked out at
// load time (see SpriteEntityRenderer).
//
// Source art is 16px to the world tile's 32, so world sizes are twice the frame size. Model
// classes state their own size in world units; when a frame here changes shape, the matching
// constructor has to change with it or the sprite will be stretched to fit.
namespace atlas {

inline const char* const EnemySheet = "assets/enemies-8.png";

// Masked to transparent when the sheet is loaded.
inline const sf::Color EnemyColorKey(146, 144, 255);

// The sheet is laid out on an 18px column pitch and a 26px row pitch, with the artwork
// itself 16 wide and 23 tall. The taller enemies are therefore 16x23, NOT 16x32: a 32-tall
// frame overruns the 26px row pitch and drags in the top of the sprite row below, which is
// what made the Koopa, Paratroopa and Hammer Bro frames show fragments of their neighbours.
//
// {{x, y}, {width, height}}
inline const sf::IntRect Goomba({0, 16}, {16, 16});          // world 32x32
inline const sf::IntRect GoombaStomped({36, 24}, {16, 8});   // world 32x16, drawn bottom-aligned
inline const sf::IntRect Koopa({0, 113}, {16, 23});          // world 32x46
inline const sf::IntRect KoopaShell({72, 120}, {16, 16});    // world 32x32
inline const sf::IntRect KoopaParatroopa({54, 113}, {16, 23});  // world 32x46
inline const sf::IntRect HammerBro({18, 183}, {16, 23});     // world 32x46
inline const sf::IntRect Hammer({54, 52}, {16, 16});         // world 32x32
inline const sf::IntRect Bowser({102, 208}, {32, 32});       // world 64x64
inline const sf::IntRect BowserFire({102, 242}, {24, 8});    // world 48x16
inline const sf::IntRect Lakitu({54, 138}, {16, 23});        // world 32x46
inline const sf::IntRect SpinyEgg({36, 352}, {16, 16});      // world 32x32
inline const sf::IntRect Spiny({72, 352}, {16, 16});         // world 32x32

// Not yet implemented in the model; recorded so the layout stays complete.
inline const sf::IntRect CheepCheep({0, 370}, {16, 16});     // world 32x32
inline const sf::IntRect PiranhaPlant({0, 139}, {16, 23});   // world 32x46

// Mario's bouncing fireball lives in enemies.png, a DIFFERENT sheet from enemies-8.png
// above: it carries a real alpha channel, so no colour-key masking is needed at load time.
// A 22x22 cell at (26,150)-(48,172) holds four 7x7 rolling poses, one in each corner;
// the roll order is left-to-right, then top-to-bottom (matching the sheet, not the order
// the corner scan happens to visit). The 7x7 source scales 2x into the 14x14 world box.
inline const char* const FireballSheet = "assets/enemies.png";
inline const sf::IntRect FireballRoll[4] = {
    {{26, 150}, {7, 7}},   // frame 0 — top-left
    {{41, 150}, {7, 7}},   // frame 1 — top-right
    {{26, 165}, {7, 7}},   // frame 2 — bottom-left
    {{41, 165}, {7, 7}},   // frame 3 — bottom-right
};

}
}

#endif
