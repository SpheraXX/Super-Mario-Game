#include "View/Player/PlayerRenderer.h"

#include "Model/Player/Player.h"

#include <SFML/Graphics/RenderTarget.hpp>

#include <cmath>
#include <cstdint>
#include <iterator>

namespace view {

namespace {
// mario-luigi.png: the small poses share one row and are drawn facing right (hence the
// `true` passed to the base renderer, which mirrors them when the player walks left). The
// big row sits directly above the small one; Super and Fire share those big frames — Fire
// is told apart by a colour tint (characterTint) rather than a separate sheet row.
//
// Luigi lives in the matching block directly above Mario's: big-Luigi just above big-Mario
// (192 vs 240) and small-Luigi just above small-Mario (224 vs 272). Every pose sits in the
// same column for both, so the only difference is which two rows the rect is taken from.
constexpr int SmallMarioRow = 272;
constexpr int BigMarioRow = 240;
constexpr int SmallLuigiRow = 224;
constexpr int BigLuigiRow = 192;

// Source frame heights. A small pose is a 16x16 cell; a big pose is 16x32.
constexpr int SmallFrameHeight = 16;
constexpr int BigFrameHeight = 32;

// World height of the one-tile (Small) form — the threshold that decides which row to draw.
constexpr float SmallDrawSize = 32.0f;

// x offset of each pose within its row. Shared by both sizes and both characters.
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
constexpr float DistancePerWalkFrame = 18.0f;

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

// Smooth blink for the invulnerability windows (post-damage and Star): the sprite's alpha
// breathes faint -> clear -> faint on a sine wave, so the player stays visible but clearly
// flickering, never an on/off toggle. Driven by the countdown itself, so the pulsing stops
// exactly when the window runs out — the blink can never outlast the invulnerability it
// advertises. A dead player is always fully opaque.
constexpr float MinBlinkAlpha = 100.0f;
constexpr float MaxBlinkAlpha = 255.0f;
constexpr float DamageBlinkDipsPerSecond = 4.0f;
constexpr float StarBlinkDipsPerSecond = 8.0f;

float blinkAlpha(const model::Player& player) {
    if (!player.isAlive()) return MaxBlinkAlpha;

    float countdown;
    float dipsPerSecond;
    if (player.isStar()) {
        countdown = player.getRemainingTime();
        dipsPerSecond = StarBlinkDipsPerSecond;
    } else if (player.getBlinkRemaining() > 0.0f) {
        countdown = player.getBlinkRemaining();
        dipsPerSecond = DamageBlinkDipsPerSecond;
    } else {
        return MaxBlinkAlpha;
    }

    const float wave = (std::sin(countdown * dipsPerSecond * 3.14159265f) + 1.0f) * 0.5f;
    return MinBlinkAlpha + (MaxBlinkAlpha - MinBlinkAlpha) * wave;
}

// Fire Mario reuses the big frames with a warmer palette rather than its own sheet row.
sf::Color fireTint() {
    return sf::Color(255, 236, 214);
}
}

PlayerRenderer::PlayerRenderer()
    : SpriteEntityRenderer("assets/mario-luigi.png", /*sourceFacesRight=*/true) {
}

sf::Color PlayerRenderer::characterTint(const model::Player& player) const {
    sf::Color color = player.isFire() ? fireTint() : sf::Color::White;
    color.a = static_cast<std::uint8_t>(std::lround(blinkAlpha(player)));
    return color;
}

void PlayerRenderer::renderTyped(sf::RenderTarget& window, const model::Player& player,
                                 const RenderContext& /* ctx */) const {
    // Mario and Luigi lay out identically on the sheet, so one pair of rows serves both.
    const int smallRow = player.isLuigi() ? SmallLuigiRow : SmallMarioRow;
    const int bigRow = player.isLuigi() ? BigLuigiRow : BigMarioRow;

    // Death: both sizes show the small dead pose. There is no big dead frame — the big cell
    // above it is the sitting pose, and a dying Mario is not sitting. That sprite is one
    // tile tall, so it is drawn into a 32x32 box anchored at the bottom of the (still
    // full-size) body rather than stretched over it.
    if (player.getAnimState() == model::AnimState::Die) {
        const float offsetY = player.getSize().y - SmallDrawSize;
        drawCharacterFrame(window, player,
                           {{DieFrameX, smallRow}, {16, SmallFrameHeight}},
                           {player.getSize().x, SmallDrawSize}, {0.0f, offsetY});
        return;
    }

    // Player::syncPowerSize grows the box to 32x64 on power-up and shrinks it back on hit,
    // so the size alone says which row to draw — no state-type checks here, and a Star
    // automatically keeps whatever size it entered with. Drawing the small 16x16 frame over
    // a grown box is exactly what stretched the sprite before the big row was wired up.
    const bool big = player.getSize().y > SmallDrawSize;
    drawCharacterFrame(window, player,
                       {{frameXFor(player), big ? bigRow : smallRow},
                        {16, big ? BigFrameHeight : SmallFrameHeight}});
}

}
