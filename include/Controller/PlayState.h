#ifndef CONTROLLER_PLAYSTATE_H
#define CONTROLLER_PLAYSTATE_H

#include "Controller/GameState.h"
#include "Controller/LevelScene.h"
#include "View/HudData.h"
#include "View/HudRenderer.h"

#include <memory>

namespace controller {

// The play state: owns the LevelScene (the live level) plus the scripted clear play
// and the completion overlay, and handles the state transitions around them — freeze
// behind the overlay, start the clear play on flagpole touch, replace with GameOver on
// run end, and HUD snapshotting + debug keys.
class PlayState : public GameState {
public:
    void onEnter() override;
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& window) override;

private:
    // 3A: the clear cinematic temporarily lives here (driven through scene accessors);
    // it moves into LevelClearSequence in substage 3B.
    enum class ClearPhase {
        None,           // normal play
        SlideToPole,    // penguin + mario slide down the pole
        WalkToCastle,   // mario auto-walks from the pole to the castle
        ReachedCastle,  // mario stands at the door; push the completion overlay
    };

    void beginLevelClear();
    void updateClearSequence(float deltaTime);
    void finishLevelClear();

    std::unique_ptr<LevelScene> scene;  // the live level behind this playthrough
    std::unique_ptr<view::HudRenderer> hudRenderer;
    view::HudData hudData;
    bool levelComplete = false;

    // Level-clear fields (see beginLevelClear/updateClearSequence).
    ClearPhase clearPhase = ClearPhase::None;
    float poleElapsed = 0.0f;
    float poleSlideStartY = 0.0f;
    float poleGroundY = 0.0f;
    bool completionOverlayPushed = false;
};

}

#endif