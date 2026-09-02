#include "View/Effect/MetalShineEffect.h"
#include <iostream>

namespace view::effect {

MetalShineEffect::MetalShineEffect() {
    m_isAvailable = sf::Shader::isAvailable();
    if (m_isAvailable) {
        if (!m_shader.loadFromFile("assets/shaders/metal_shine.frag", sf::Shader::Type::Fragment)) {
            std::cerr << "[MetalShineEffect] Failed to load shader\n";
            m_isAvailable = false;
        }
    }
}

void MetalShineEffect::setInterval(float interval) {
    m_interval = interval;
}

void MetalShineEffect::update(float dt) {
    if (!m_isAvailable) return;

    if (m_isShining) {
        // Tốc độ quét sáng (0.8 unit mỗi giây)
        m_progress += 0.8f * dt;
        
        // Cập nhật giá trị vào shader
        m_shader.setUniform("progress", m_progress);

        // Kết thúc quét
        if (m_progress > 1.2f) {
            m_isShining = false;
            m_timer = 0.f;
        }
    } else {
        m_timer += dt;
        if (m_timer >= m_interval) {
            m_isShining = true;
            m_progress = -0.2f;
        }
    }
}

void MetalShineEffect::draw(sf::RenderTarget& target, const sf::Sprite& sprite) {
    if (m_isAvailable && m_isShining) {
        target.draw(sprite, &m_shader);
    } else {
        target.draw(sprite);
    }
}

} // namespace view::effect
