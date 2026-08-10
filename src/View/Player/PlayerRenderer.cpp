#include "View/Player/PlayerRenderer.h"

#include "Model/Player/Player.h"

#include <SFML/Graphics/RenderTarget.hpp>

#include <array>
#include <cmath>
#include <cstdint>

namespace view {

namespace {
// mario-luigi.png: the small-Mario poses share one row in the atlas and are drawn
// facing right (hence the `true` passed to the base renderer, which mirrors them when
// Mario walks left). The big-Mario row sits directly above the small row; Super and
// Fire share these big frames — Fire is told apart by a colour tint (characterTint)
// rather than a different sheet row.
//
// Luigi lives in the matching block directly above Mario's: the big-Luigi row sits just
// above the big-Mario row (192 vs 240) and the small-Luigi row just above the small-Mario
// row (224 vs 272). Every pose is in the same column as Mario's, so the only difference
// from Mario is which two rows the frame rect is taken from.
constexpr int SmallMarioRow = 272;
constexpr int BigMarioRow = 240;
constexpr int SmallLuigiRow = 224;
constexpr int BigLuigiRow = 192;
constexpr int SmallMarioDrawSize = 32;

struct FrameRect {
    int x;
    int width;
};

// Keep every pose at the same source width so the sprite does not get horizontally
// stretched when the renderer scales it to the 32x32 player box.
constexpr FrameRect StandFrame{176, 16};
constexpr FrameRect WalkFrames[] = {
    {80, 16},
    {96, 16},
    {112, 16},
};
constexpr FrameRect SprintReadyFrame{128, 16};
// Run cycles three poses: the dedicated run frame plus two walk frames (the third walk
// pose included), so the sprint reads as a three-step loop rather than one frozen sprite.
// Works for both sizes since the frame column is shared, only the row differs.
constexpr FrameRect RunFrames[] = {
    {144, 16},
    {112, 16},
    {80, 16},
};
constexpr FrameRect DieFrame{161, 16};
// The sitting/crouch pose shares its column with the small-Mario dead pose but lives in
// the big row directly above it. Only big Mario uses it (small Mario never crouches).
constexpr FrameRect SitFrame{161, 16};
constexpr FrameRect JumpFrame{144, 16};

constexpr float WalkFrameDuration = 0.10f;

FrameRect walkFrameFor(float time) {
    const std::size_t index = static_cast<std::size_t>(time / WalkFrameDuration) % std::size(WalkFrames);
    return WalkFrames[index];
}

FrameRect runFrameFor(float time) {
    const std::size_t index = static_cast<std::size_t>(time / WalkFrameDuration) % std::size(RunFrames);
    return RunFrames[index];
}

FrameRect frameFor(const model::Player& player) {
    switch (player.getAnimState()) {
        case model::AnimState::Walk:
            return walkFrameFor(player.getAnimationClock());
        case model::AnimState::Run:
            return runFrameFor(player.getAnimationClock());
        case model::AnimState::Jump:
        case model::AnimState::Fall:
            return JumpFrame;
        case model::AnimState::Crouch:
            return SitFrame;
        case model::AnimState::Die:
            return DieFrame;
        case model::AnimState::Idle:
        default:
            if (player.isSprinting() && player.getHorizontalInput() == 0) {
                return SprintReadyFrame;
            }
            return StandFrame;
    }
}

sf::IntRect toIntRect(FrameRect frame, int row, int height) {
    return {{frame.x, row}, {frame.width, height}};
}

// Smooth blink for the invincibility windows (post-damage and Star): the sprite's alpha
// breathes faint -> clear -> faint on a sine wave, so the character stays visible but
// clearly flickering — never an on/off toggle. Driven by a countdown timer, so the
// pulsing simply stops when the window runs out. A dead player (or one with no active
// window) is always fully opaque.
constexpr float MinBlinkAlpha = 100.0f;
constexpr float MaxBlinkAlpha = 255.0f;
// How many faint dips per second of countdown time. Damage blinks a touch slower than
// Star so the two read differently.
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

}

PlayerRenderer::PlayerRenderer()
    : SpriteEntityRenderer("assets/mario-luigi.png", /*sourceFacesRight=*/true) {
}

sf::Color PlayerRenderer::characterTint(const model::Player& player) const {
    sf::Color color = player.isFire() ? fireTint() : sf::Color::White;
    color.a = static_cast<std::uint8_t>(std::lround(blinkAlpha(player)));
    return color;
}

void PlayerRenderer::renderTyped(sf::RenderTarget& window, const model::Player& player) const {
    const model::AnimState anim = player.getAnimState();

    // Both characters lay out identically on the sheet, so a single pair of rows serves
    // both: Mario's block and Luigi's block directly above it.
    const int smallRow = player.isLuigi() ? SmallLuigiRow : SmallMarioRow;
    const int bigRow = player.isLuigi() ? BigLuigiRow : BigMarioRow;

    // Death: both sizes show the small dead pose. There is no big dead frame — the big
    // cell above the small dead pose is the sitting pose (used for crouching), and a dying
    // Mario is not sitting. The dead sprite is one tile tall, so it is drawn into a 32x32
    // box anchored at the bottom of the (still full-size) body.
    if (anim == model::AnimState::Die) {
        constexpr float DeadDrawSize = 32.0f;
        const float offsetY = player.getSize().y - DeadDrawSize;
        drawCharacterFrame(window, player, toIntRect(DieFrame, smallRow, 16),
                           {DeadDrawSize, DeadDrawSize}, {0.0f, offsetY});
        return;
    }

    // Crouch: the sitting pose in the big row, at the column of the small dead pose. Its
    // artwork spans y=250..271 of the 32-tall cell; the crouch box is sized to exactly that
    // (22px * 2x = 44px, Player::CrouchHeight), so drawing the tight art rect fills the box
    // at the sheet's natural 2x scale — no squashing into 32x32.
    if (anim == model::AnimState::Crouch) {
        drawCharacterFrame(window, player, sf::IntRect{{SitFrame.x, bigRow + 10}, {16, 22}});
        return;
    }

    // The model grows the player's box to 32x64 on power-up and shrinks it back on hit
    // (Player::syncPowerSize), so the size alone tells us which row to draw — no state-type
    // checks here, and a Star automatically keeps whatever size it entered with.
    const bool big = player.getSize().y > SmallMarioDrawSize;
    const int row = big ? bigRow : smallRow;
    const int height = big ? 32 : 16;
    const FrameRect frame = frameFor(player);
    drawCharacterFrame(window, player, toIntRect(frame, row, height));
}

}
