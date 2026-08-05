#ifndef VIEW_ENTITYRENDERER_H
#define VIEW_ENTITYRENDERER_H

#include <SFML/Graphics/RenderTarget.hpp>

namespace model {
class Entity;
}

namespace view {

// Strategy interface: knows how to draw one concrete model entity type. Instances are
// owned by an EntityRendererRegistry, which dispatches each entity to the renderer
// registered for its exact runtime type.
class EntityRenderer {
public:
    virtual ~EntityRenderer() = default;

    virtual void render(sf::RenderTarget& window, const model::Entity& entity) const = 0;
};

// Intermediate base that performs the single, guaranteed-safe downcast from the generic
// Entity reference to the renderer's concrete type T (the registry key guarantees the
// entity really is a T), then delegates to a typed, overridable renderTyped(). Concrete
// renderers therefore never see a dynamic_cast or a type check.
template <typename T>
class TypedEntityRenderer : public EntityRenderer {
public:
    void render(sf::RenderTarget& window, const model::Entity& entity) const final {
        renderTyped(window, static_cast<const T&>(entity));
    }

protected:
    virtual void renderTyped(sf::RenderTarget& window, const T& entity) const = 0;
};

}

#endif
