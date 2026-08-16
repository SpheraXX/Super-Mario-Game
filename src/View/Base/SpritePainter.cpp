#include "View/Base/SpritePainter.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <cstdint>
#include <cmath>
#include <stdexcept>

namespace view {

namespace {
bool nearlyEqual(sf::Color a, sf::Color b, int tolerance) {
    const auto channelDiff = [](std::uint8_t lhs, std::uint8_t rhs) {
        return std::abs(static_cast<int>(lhs) - static_cast<int>(rhs));
    };
    return channelDiff(a.r, b.r) <= tolerance
        && channelDiff(a.g, b.g) <= tolerance
        && channelDiff(a.b, b.b) <= tolerance;
}
}

SpritePainter::SpritePainter(const std::string& texturePath) {
    load(texturePath);
}

bool SpritePainter::load(const std::string& texturePath) {
    if (!image.loadFromFile(texturePath)) {
        loaded = false;
        return false;
    }
    if (!texture.loadFromImage(image)) {
        loaded = false;
        return false;
    }
    texture.setSmooth(false);
    loaded = true;
    return true;
}

bool SpritePainter::isLoaded() const {
    return loaded;
}

void SpritePainter::applyColorKey(const sf::IntRect& area, sf::Color transparentColor) {
    if (!loaded) {
        return;
    }

    const unsigned int left = static_cast<unsigned int>(area.position.x);
    const unsigned int top = static_cast<unsigned int>(area.position.y);
    const unsigned int width = static_cast<unsigned int>(area.size.x);
    const unsigned int height = static_cast<unsigned int>(area.size.y);

    for (unsigned int py = top; py < top + height; ++py) {
        for (unsigned int px = left; px < left + width; ++px) {
            const sf::Vector2u pixel(px, py);
            if (nearlyEqual(image.getPixel(pixel), transparentColor, ColorKeyTolerance)) {
                image.setPixel(pixel, sf::Color::Transparent);
            }
        }
    }
    colorKeyDirty = true;
}

void SpritePainter::commitColorKeys() {
    if (!loaded || !colorKeyDirty) {
        return;
    }
    if (!texture.loadFromImage(image)) {
        throw std::runtime_error("Could not upload tileset texture");
    }
    texture.setSmooth(false);
    colorKeyDirty = false;
}

void SpritePainter::draw(sf::RenderTarget& window, const sf::IntRect& frame,
                         const sf::Vector2f& position, const sf::Vector2f& scale) const {
    if (!loaded) {
        return;
    }

    sf::Sprite sprite(texture);
    sprite.setTextureRect(frame);
    sprite.setScale(scale);
    sprite.setPosition({std::round(position.x), std::round(position.y)});
    window.draw(sprite);
}

void SpritePainter::drawCell(sf::RenderTarget& window, const sf::IntRect& frame,
                             const sf::Vector2f& origin) const {
    const float scale = static_cast<float>(model::TileMap::TileWidth) / static_cast<float>(SourceTileSize);
    draw(window, frame, origin, {scale, scale});
}

}