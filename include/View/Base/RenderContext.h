#ifndef VIEW_RENDERCONTEXT_H
#define VIEW_RENDERCONTEXT_H

#include "Model/World/WorldType.h"

namespace view {

// Read-only per-frame rendering context handed to every entity renderer. It carries the
// view-side state sprites depend on that lives outside the entity model — today only the
// world type, which picks themed atlas rows (e.g. teal CoinBlocks underwater, gray bricks
// in a castle). Renderers read it but never store it, so they stay stateless.
struct RenderContext {
    model::WorldType worldType;
};

}

#endif