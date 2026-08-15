#ifndef VIEW_HUDDATA_H
#define VIEW_HUDDATA_H

#include <string>

namespace view {

// Snapshot of everything the HUD draws, rebuilt by the controller every frame. Keeping
// the data in one plain struct (instead of the view reaching into the model) means the
// renderer has no dependency on game state.
struct HudData {
    int score = 0;
    int coins = 0;
    std::string levelName;
    int time = 0;
};

}

#endif
