#ifndef VIEW_SPRITEENTITYRENDERER_H
#define VIEW_SPRITEENTITYRENDERER_H

#include "View/EntityRenderer.h"
#include "View/EntityRenderUtils.h"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
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
    explicit SpriteEntityRenderer(const std::string& texturePath)
        : textureLoaded(texture.loadFromFile(texturePath)) {
        texture.setSmooth(false);
    }

protected:
    // Draw one 16x32 character frame for the entity, snapped to integer pixels and
    // mirrored to face the entity's direction.
    void drawCharacterFrame(sf::RenderWindow& window, const T& entity, sf::IntRect frame) const {
        if (!textureLoaded) return;

        sf::Sprite sprite(texture);
        sprite.setTextureRect(frame);
        setupEntitySprite(sprite, entity.getSize(), entity.isFacingRight());
        sprite.setPosition({std::round(entity.getPosition().x),
                            std::round(entity.getPosition().y)});
        window.draw(sprite);
    }

    sf::Texture texture;

private:
    bool textureLoaded;
};

}

#endif
