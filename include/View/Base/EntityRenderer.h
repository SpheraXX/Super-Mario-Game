#ifndef VIEW_ENTITYRENDERER_H
#define VIEW_ENTITYRENDERER_H

#include "View/Base/RenderContext.h"

#include <SFML/Graphics/RenderTarget.hpp>

namespace model {
class Entity;
}

namespace view {

// Strategy interface: knows how to draw one concrete model entity type. Instances are
// owned by an EntityRendererRegistry, which dispatches each entity to the renderer
// registered for its exact runtime type. Every draw receives a RenderContext with the
// world type so themed renderers can pick the right atlas row.
class EntityRenderer {
public:
    virtual ~EntityRenderer() = default;

    virtual void render(sf::RenderTarget& window, const model::Entity& entity,
                        const RenderContext& ctx) const = 0;
};

// Intermediate base that performs the single, guaranteed-safe downcast from the generic
// Entity reference to the renderer's concrete type T (the registry key guarantees the
// entity really is a T), then delegates to a typed, overridable renderTyped(). Concrete
// renderers therefore never see a dynamic_cast or a type check.
template <typename T>
class TypedEntityRenderer : public EntityRenderer {
public:
    void render(sf::RenderTarget& window, const model::Entity& entity,
                const RenderContext& ctx) const final {
        renderTyped(window, static_cast<const T&>(entity), ctx);
    }

protected:
    virtual void renderTyped(sf::RenderTarget& window, const T& entity,
                             const RenderContext& ctx) const = 0;
};

}

#endif
