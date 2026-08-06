#ifndef VIEW_SPRITEENTITYRENDERER_H
#define VIEW_SPRITEENTITYRENDERER_H

#include "View/Base/EntityRenderer.h"
#include "View/Base/EntityRenderUtils.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <cmath>
#include <string>

namespace view {

// Base for renderers that draw one entity from a sprite sheet: owns the texture and
// provides a shared character-frame drawing helper. T must be a Character (or subclass)
// so the frame can mirror against the entity's facing direction.
template <typename T>
class SpriteEntityRenderer : public TypedEntityRenderer<T> {
public:
    // `sourceFacesRight` describes the spritesheet, not the entity: sheets differ in which
    // way their artwork is drawn, so the renderer mirrors only when the sheet and the
    // entity disagree. Stating it per sheet avoids every renderer re-deriving the flip.
    explicit SpriteEntityRenderer(const std::string& texturePath, bool sourceFacesRight = false)
        : sourceFacesRight(sourceFacesRight) {
        textureLoaded = texture.loadFromFile(texturePath);
        texture.setSmooth(false);
    }

    // For sheets with no alpha channel, where a flat background colour stands in for
    // transparency. The key is masked out once, at load, rather than per draw.
    SpriteEntityRenderer(const std::string& texturePath, sf::Color colorKey,
                         bool sourceFacesRight = false)
        : sourceFacesRight(sourceFacesRight) {
        sf::Image sheet;
        if (sheet.loadFromFile(texturePath)) {
            sheet.createMaskFromColor(colorKey);
            textureLoaded = texture.loadFromImage(sheet);
        }
        texture.setSmooth(false);
    }

protected:
    // Draw one character frame for the entity, snapped to integer pixels and mirrored to
    // face the entity's direction. `frame` must tightly bound the artwork: it is stretched
    // to fill the entity's box, so any padding inside it shows up as misalignment between
    // the sprite and the hitbox.
    void drawCharacterFrame(sf::RenderTarget& window, const T& entity, sf::IntRect frame) const {
        drawCharacterFrame(window, entity, frame, entity.getSize(), {0.0f, 0.0f});
    }

    // Draw into a box that is not the entity's own. Needed when a pose is a different shape
    // from the entity's collision box — a squished Goomba is half the height of the Goomba
    // that was walking a moment ago, and stretching that frame over the full box would
    // un-squash it. `drawOffset` is relative to the entity's top-left, so a caller lowers a
    // short frame onto the ground rather than leaving it floating at head height.
    void drawCharacterFrame(sf::RenderTarget& window, const T& entity, sf::IntRect frame,
                            model::Vector2 drawSize, model::Vector2 drawOffset) const {
        if (!textureLoaded) return;

        sf::Sprite sprite(texture);
        sprite.setTextureRect(frame);
        setupEntitySprite(sprite, frame, drawSize, sourceFacesRight != entity.isFacingRight());
        sprite.setPosition({std::round(entity.getPosition().x + drawOffset.x),
                            std::round(entity.getPosition().y + drawOffset.y)});
        window.draw(sprite);
    }

    sf::Texture texture;

private:
    bool textureLoaded = false;
    bool sourceFacesRight;
};

}

#endif
