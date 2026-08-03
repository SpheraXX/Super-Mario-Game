#ifndef VIEW_ENTITYRENDERERREGISTRY_H
#define VIEW_ENTITYRENDERERREGISTRY_H

#include "View/EntityRenderer.h"
#include "Model/Entity.h"

#include <memory>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>

namespace view {

// Owns one EntityRenderer per concrete entity type and dispatches every entity to the
// right one by its exact runtime type. This centralises the type -> renderer mapping so
// game code can draw any entity without explicit type checks.
class EntityRendererRegistry {
public:
    // Register a renderer class R for entities of type T. R must derive from
    // EntityRenderer (typically TypedEntityRenderer<T>).
    template <typename T, typename R, typename... Args>
    void registerRenderer(Args&&... args) {
        renderers[typeid(T)] = std::make_unique<R>(std::forward<Args>(args)...);
    }

    // Draw entity through the renderer registered for its dynamic type. Entities without
    // a registered renderer are simply not drawn (e.g. unspawned future types).
    void render(sf::RenderWindow& window, const model::Entity& entity) const {
        const auto it = renderers.find(typeid(entity));
        if (it != renderers.end()) {
            it->second->render(window, entity);
        }
    }

private:
    std::unordered_map<std::type_index, std::unique_ptr<EntityRenderer>> renderers;
};

}

#endif
