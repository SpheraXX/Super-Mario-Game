#ifndef VIEW_UI_NINESLICESPRITE_H
#define VIEW_UI_NINESLICESPRITE_H

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace view {
namespace ui {

class NineSliceSprite {
public:
    NineSliceSprite();
    
    void setTexture(const sf::Texture& texture);
    void setTextureRect(const sf::IntRect& rect);
    // Margins order: left, right, top, bottom
    void setMargins(float left, float right, float top, float bottom);
    void setSize(float width, float height);
    void setPosition(float x, float y);
    // Tints all 9-slice vertices. Applied on the next updateVertices() call.
    void setColor(const sf::Color& color);

    sf::Vector2f getSize() const { return m_size; }
    
    // Renders the sprite to the target (const: no side-effects during draw)
    void render(sf::RenderTarget& target) const;

private:
    // Rebuilds all 54 vertices from current state.
    // Cheap guard: only runs when m_dirty is true.
    void updateVertices();
    void addQuad(int index, const sf::FloatRect& pos, const sf::FloatRect& tex);

    const sf::Texture* m_texture;
    sf::VertexArray m_vertices;
    sf::IntRect m_textureRect;
    float m_margins[4]; // left, right, top, bottom
    sf::Vector2f m_size;
    sf::Vector2f m_position;
    sf::Color m_color;
    bool m_dirty; // true when geometry needs to be rebuilt
};

} // namespace ui
} // namespace view

#endif
