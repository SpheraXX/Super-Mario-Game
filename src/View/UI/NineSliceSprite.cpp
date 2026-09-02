#include "View/UI/NineSliceSprite.h"

namespace view {
namespace ui {

NineSliceSprite::NineSliceSprite() 
    : m_texture(nullptr)
    , m_vertices(sf::PrimitiveType::Triangles, 54) // 9 quads * 6 vertices each
    , m_textureRect({0, 0}, {0, 0})
    , m_size(0, 0)
    , m_position(0, 0)
    , m_color(sf::Color::White)
    , m_dirty(false) {
    for (int i = 0; i < 4; ++i) m_margins[i] = 0.f;
}

void NineSliceSprite::setTexture(const sf::Texture& texture) {
    m_texture = &texture;
    if (m_textureRect.size.x == 0) {
        m_textureRect = sf::IntRect({0, 0},
            {static_cast<int>(texture.getSize().x),
             static_cast<int>(texture.getSize().y)});
    }
    m_dirty = true;
}

void NineSliceSprite::setTextureRect(const sf::IntRect& rect) {
    if (m_textureRect == rect) return; // avoid redundant rebuild
    m_textureRect = rect;
    m_dirty = true;
}

void NineSliceSprite::setMargins(float left, float right, float top, float bottom) {
    m_margins[0] = left;
    m_margins[1] = right;
    m_margins[2] = top;
    m_margins[3] = bottom;
    m_dirty = true;
}

void NineSliceSprite::setSize(float width, float height) {
    if (m_size.x == width && m_size.y == height) return;
    m_size = sf::Vector2f(width, height);
    m_dirty = true;
}

void NineSliceSprite::setPosition(float x, float y) {
    if (m_position.x == x && m_position.y == y) return;
    m_position = sf::Vector2f(x, y);
    m_dirty = true;
}

// setColor only stores the new color and marks dirty.
// The color is applied to vertices inside updateVertices(),
// so there is exactly one place that writes vertex colors.
void NineSliceSprite::setColor(const sf::Color& color) {
    if (m_color == color) return;
    m_color = color;
    m_dirty = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────────────────────────────────────

void NineSliceSprite::addQuad(int index, const sf::FloatRect& pos, const sf::FloatRect& tex) {
    int vIdx = index * 6;
    
    // Triangle 1: TL, TR, BL
    m_vertices[vIdx + 0].position  = { pos.position.x,                pos.position.y                };
    m_vertices[vIdx + 0].texCoords = { tex.position.x,                tex.position.y                };
    m_vertices[vIdx + 0].color     = m_color;

    m_vertices[vIdx + 1].position  = { pos.position.x + pos.size.x,  pos.position.y                };
    m_vertices[vIdx + 1].texCoords = { tex.position.x + tex.size.x,  tex.position.y                };
    m_vertices[vIdx + 1].color     = m_color;

    m_vertices[vIdx + 2].position  = { pos.position.x,                pos.position.y + pos.size.y  };
    m_vertices[vIdx + 2].texCoords = { tex.position.x,                tex.position.y + tex.size.y  };
    m_vertices[vIdx + 2].color     = m_color;

    // Triangle 2: BL, TR, BR
    m_vertices[vIdx + 3] = m_vertices[vIdx + 2]; // BL
    m_vertices[vIdx + 4] = m_vertices[vIdx + 1]; // TR

    m_vertices[vIdx + 5].position  = { pos.position.x + pos.size.x,  pos.position.y + pos.size.y  };
    m_vertices[vIdx + 5].texCoords = { tex.position.x + tex.size.x,  tex.position.y + tex.size.y  };
    m_vertices[vIdx + 5].color     = m_color;
}

void NineSliceSprite::updateVertices() {
    m_dirty = false;

    if (m_size.x <= 0 || m_size.y <= 0 || m_textureRect.size.x <= 0) {
        // Degenerate — collapse all vertices to origin so nothing is drawn
        for (std::size_t i = 0; i < m_vertices.getVertexCount(); ++i) {
            m_vertices[i].position = {};
        }
        return;
    }

    const float left   = m_margins[0];
    const float right  = m_margins[1];
    const float top    = m_margins[2];
    const float bottom = m_margins[3];

    const float tx = static_cast<float>(m_textureRect.position.x);
    const float ty = static_cast<float>(m_textureRect.position.y);
    const float tw = static_cast<float>(m_textureRect.size.x);
    const float th = static_cast<float>(m_textureRect.size.y);

    // Destination x-columns: left edge, inner-left, inner-right, right edge
    float xs[4] = {
        m_position.x,
        m_position.x + left,
        m_position.x + m_size.x - right,
        m_position.x + m_size.x
    };
    float ys[4] = {
        m_position.y,
        m_position.y + top,
        m_position.y + m_size.y - bottom,
        m_position.y + m_size.y
    };

    // Source tex-columns matching the 9 regions
    float txs[4] = { tx, tx + left, tx + tw - right, tx + tw };
    float tys[4] = { ty, ty + top,  ty + th - bottom, ty + th };

    // Graceful degradation: if the button is smaller than the margins,
    // collapse the inner region to the midpoint instead of inverting it.
    if (m_size.x < left + right) {
        xs[1] = xs[2] = m_position.x + m_size.x * 0.5f;
    }
    if (m_size.y < top + bottom) {
        ys[1] = ys[2] = m_position.y + m_size.y * 0.5f;
    }

    int quadIdx = 0;
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            const sf::FloatRect posRect({ xs[col], ys[row] }, { xs[col+1] - xs[col], ys[row+1] - ys[row] });
            const sf::FloatRect texRect({ txs[col], tys[row] }, { txs[col+1] - txs[col], tys[row+1] - tys[row] });

            if (posRect.size.x > 0.f && posRect.size.y > 0.f) {
                addQuad(quadIdx, posRect, texRect);
            } else {
                addQuad(quadIdx, sf::FloatRect({}, {}), sf::FloatRect({}, {}));
            }
            ++quadIdx;
        }
    }
}

void NineSliceSprite::render(sf::RenderTarget& target) const {
    if (!m_texture || m_size.x <= 0 || m_size.y <= 0) return;

    // Rebuild geometry lazily — avoids redundant work when multiple setters
    // are called in a single frame before the first draw.
    if (m_dirty) {
        const_cast<NineSliceSprite*>(this)->updateVertices();
    }

    sf::RenderStates states;
    states.texture = m_texture;
    target.draw(m_vertices, states);
}

} // namespace ui
} // namespace view
