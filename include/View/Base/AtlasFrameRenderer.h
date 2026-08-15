#ifndef VIEW_ATLASFRAMERENDERER_H
#define VIEW_ATLASFRAMERENDERER_H

#include "View/Base/SpriteEntityRenderer.h"
#include "View/Enemy/EnemyAtlas.h"

namespace view {

// Draws an entity as a single fixed frame from the enemy spritesheet.
//
// Most of the roster has exactly one pose and needs no per-type drawing logic, so they share
// this template instead of each carrying a near-empty renderer class. Types with real state
// to show — a Koopa's shell, a squished Goomba — still get their own renderer.
//
// The frame is a constructor argument rather than a template parameter so that every rect in
// use is visible together at registration time, next to the atlas that defines them.
template <typename T>
class AtlasFrameRenderer : public SpriteEntityRenderer<T> {
public:
    explicit AtlasFrameRenderer(sf::IntRect frame)
        : SpriteEntityRenderer<T>(atlas::EnemySheet, atlas::EnemyColorKey), frame(frame) {}

protected:
    void renderTyped(sf::RenderTarget& window, const T& entity,
                     const RenderContext& /* ctx */) const override {
        this->drawCharacterFrame(window, entity, frame);
    }

    // The generic single-frame enemies die feet-first: a shell-killed HammerBro, Spiny,
    // Lakitu, Bowser or PiranhaPlant inverts before its death pop.
    bool flipWhenDying() const override { return true; }

private:
    sf::IntRect frame;
};

}

#endif
