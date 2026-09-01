#ifndef VIEW_EFFECT_METALSHINEEFFECT_H
#define VIEW_EFFECT_METALSHINEEFFECT_H

#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace view::effect {

class MetalShineEffect {
public:
    MetalShineEffect();

    void setInterval(float interval);

    void update(float dt);
    void draw(sf::RenderTarget& target, const sf::Sprite& sprite);

private:
    sf::Shader   m_shader;
    bool         m_isAvailable = false;
    float        m_timer = 0.f;
    float        m_interval = 3.f;   // Lóe sáng mỗi 3 giây
    float        m_progress = -0.2f; // Chạy từ -0.2 đến 1.2
    bool         m_isShining = false;
};

} // namespace view::effect

#endif
