#include "View/Player/PlayerRenderer.h"

#include "Model/Player/Player.h"

#include <SFML/Graphics/RenderTarget.hpp>

#include <cmath>
#include <cstdint>
#include <iterator>

namespace view {

namespace {
// mario-luigi.png: a strict 16x16 grid; the actual spritesheet starts at x=80, so every
// pose X below is written with that offset already applied. From the top, each band is
// one form (sprites drawn facing right, hence the `true` passed to the base renderer,
// which mirrors them when the player walks left):
//   y  0-31  big Mario        y 48-79  big fire Mario
//   y 32-47  Mario            y 80-95  fire Mario
//   y 96-127 big Luigi        y128-143 Luigi            (unused: Luigi borrows Mario's art)
//   y144-175 big star Mario   y176-191 star Mario
// The bands below y192 are leftovers; nothing references them.
constexpr int MarioRow = 32;
constexpr int BigMarioRow = 0;
constexpr int FireRow = 80;
constexpr int BigFireRow = 48;
constexpr int StarRow = 176;
constexpr int BigStarRow = 144;

// Source frame heights. A small pose is a 16x16 cell; a big pose is 16x32.
constexpr int SmallFrameHeight = 16;
constexpr int BigFrameHeight = 32;

// World height of the one-tile (Small) form — the threshold that decides which row to draw.
constexpr float SmallDrawSize = 16.0f;

// x offset of each pose within its row. Shared by every band: the pose columns are valid
// throughout all rows.
constexpr int StandFrameX = 176;
constexpr int JumpFrameX = 144;
constexpr int DieFrameX = 161;

// The walk cycle is three frames, laid out consecutively on the sheet. Walking and running
// use the same three: the original has no separate running artwork, the legs simply move
// faster. Pinning Walk to the first frame and Run to the third — which is what the code did
// before — meant neither ever animated, so a moving player looked frozen mid-stride.
constexpr int WalkFrames[] = {80, 96, 112};
constexpr int WalkFrameCount = static_cast<int>(std::size(WalkFrames));

// World units of ground covered per frame of the cycle. Because the phase comes from
// distance rather than time, walking (180 u/s) cycles at ~10fps and running (320 u/s) at
// ~18fps for free, and the cycle freezes when the player is stopped against a wall.
constexpr float DistancePerWalkFrame = 9.0f;

int frameXFor(const model::Player& player) {
    switch (player.getAnimState()) {
        case model::AnimState::Walk:
        case model::AnimState::Run: {
            // fmod first: the accumulator only ever grows, and folding it before the cast
            // keeps the conversion well away from int range on a long level.
            const float cycleLength = DistancePerWalkFrame * WalkFrameCount;
            const float phase = std::fmod(player.getWalkCycleDistance(), cycleLength);
            int index = static_cast<int>(phase / DistancePerWalkFrame);
            if (index < 0 || index >= WalkFrameCount) index = 0;  // guard against fp edges
            return WalkFrames[index];
        }
        // Rising and falling share the airborne pose; there is no separate falling frame.
        case model::AnimState::Jump:
        case model::AnimState::Fall:
            return JumpFrameX;
        case model::AnimState::Die:
            return DieFrameX;
        case model::AnimState::Idle:
        default:
            return StandFrameX;
    }
}

// Smooth blink for the post-damage invulnerability window only: the sprite's alpha
// breathes faint -> clear -> faint on a sine wave, so the player stays visible but clearly
// flickering, never an on/off toggle. Driven by the countdown itself, so the pulsing stops
// exactly when the window runs out — the blink can never outlast the invulnerability it
// advertises. Star no longer blinks alpha (it swaps sprite rows, see renderTyped), and a
// dead player is always fully opaque.
constexpr float MinBlinkAlpha = 100.0f;
constexpr float MaxBlinkAlpha = 255.0f;
constexpr float DamageBlinkDipsPerSecond = 4.0f;

float blinkAlpha(const model::Player& player) {
    if (!player.isAlive()) return MaxBlinkAlpha;
    if (player.getBlinkRemaining() <= 0.0f) return MaxBlinkAlpha;

    const float wave = (std::sin(player.getBlinkRemaining() * DamageBlinkDipsPerSecond * 3.14159265f) + 1.0f) * 0.5f;
    return MinBlinkAlpha + (MaxBlinkAlpha - MinBlinkAlpha) * wave;
}

// Star flicker rate: sprite swaps per second between the base form and the star row.
// Square wave off the remaining time, so the alternation stops exactly when the star
// expires.
constexpr float StarSwapsPerSecond = 8.0f;

bool starFrameActive(const model::Player& player) {
    if (!player.isStar()) return false;
    return static_cast<int>(player.getRemainingTime() * StarSwapsPerSecond) % 2 == 0;
}
}

PlayerRenderer::PlayerRenderer()
    : SpriteEntityRenderer("assets/mario-luigi.png", /*sourceFacesRight=*/true) {
}

sf::Color PlayerRenderer::characterTint(const model::Player& player) const {
    // Fire is drawn from its own sheet rows; the tint only ever carries the post-damage
    // invulnerability blink (and nothing else — a dead player is opaque).
    sf::Color color = sf::Color::White;
    color.a = static_cast<std::uint8_t>(std::lround(blinkAlpha(player)));
    return color;
}

void PlayerRenderer::renderTyped(sf::RenderTarget& window, const model::Player& player,
                                 const RenderContext& /* ctx */) const {
    // Death: both sizes show the small dead pose. There is no big dead frame — the big cell
    // above it is the sitting pose, and a dying Mario is not sitting. That sprite is one
    // tile tall, so it is drawn into a 32x32 box anchored at the bottom of the (still
    // full-size) body rather than stretched over it. It always comes from the plain Mario
    // row: a fire/star death is still the classic sprite.
    if (player.getAnimState() == model::AnimState::Die) {
        const float offsetY = player.getSize().y - SmallDrawSize;
        drawCharacterFrame(window, player,
                           {{DieFrameX, MarioRow}, {16, SmallFrameHeight}},
                           {player.getSize().x, SmallDrawSize}, {0.0f, offsetY});
        return;
    }

    // Player::syncPowerSize grows the box to 32x64 on power-up and shrinks it back on hit,
    // so the size alone says which height band to draw — no state-type checks here, and a
    // Star automatically keeps whatever size it entered with. Drawing the small 16x16 frame
    // over a grown box is exactly what stretched the sprite before the big row was wired up.
    const bool big = player.getSize().y > SmallDrawSize;

    // The band: Luigi borrows Mario's art, fire has its own rows, and a starred player
    // alternates between its base band and the star band (fire + star alternates fire rows
    // and star rows). The pose columns are the same in every band.
    const int row = starFrameActive(player)
        ? (big ? BigStarRow : StarRow)
        : (player.isFire() ? (big ? BigFireRow : FireRow)
                           : (big ? BigMarioRow : MarioRow));
    drawCharacterFrame(window, player,
                       {{frameXFor(player), row},
                        {16, big ? BigFrameHeight : SmallFrameHeight}});
}

}