#ifndef CONTROLLER_PLAYSTATE_H
#define CONTROLLER_PLAYSTATE_H

#include "Controller/GameState.h"
#include "Controller/LevelClearSequence.h"
#include "Controller/LevelScene.h"
#include "Model/Save/SaveData.h"
#include "View/HudData.h"
#include "View/HudRenderer.h"

#include <memory>

namespace controller {

// The play state: owns the LevelScene (the live level) and the scripted clear play,
// and handles the state transitions around them — freeze behind the completion
// overlay, replace with GameOver on run end — plus the HUD snapshot and debug keys.
class PlayState : public GameState {
public:
    PlayState();
    explicit PlayState(const model::GameSaveData& save);
    ~PlayState() override;

    void onEnter() override;
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& window) override;

    model::GameSaveData captureSaveData() const;
    void saveGame() const;

private:
    // Freeze the level and push the transparent completion overlay.
    void finishClear();

    std::unique_ptr<LevelScene> scene;  // the live level behind this playthrough
    LevelClearSequence sequence;        // the flagpole clear cinematic
    std::unique_ptr<view::HudRenderer> hudRenderer;
    view::HudData hudData;
    bool levelComplete = false;

    bool hasSavedState = false;
    std::unique_ptr<model::GameSaveData> savedState;
};

}

#endif