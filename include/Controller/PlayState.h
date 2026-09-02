#ifndef CONTROLLER_PLAYSTATE_H
#define CONTROLLER_PLAYSTATE_H

#include "Controller/GameState.h"
#include "Controller/LevelClearSequence.h"
#include "Controller/LevelScene.h"
#include "Model/Save/SaveData.h"
#include "Model/World/WorldType.h"
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
    // Fires when a state pushed on top of this one (Pause, and Options pushed from
    // Pause) is popped back down to PlayState -- the natural point to notice the player
    // changed the Character option while paused and apply it live (see
    // LevelScene::switchCharacter).
    void onResume() override;
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& window) override;

    model::GameSaveData captureSaveData() const;
    void saveGame() const;

private:
    // Award the clear bonus (time remaining), freeze the level and push the transparent
    // completion overlay.
    void finishClear();
    // Called inside finishClear: updates level_progress, high_scores, and unlocks.
    void updateProgressAndUnlocks(model::GameSaveData& data);
    // Called inside finishClear: syncs ProfileManager stats from the post-clear save.
    void syncProfileStats(const model::GameSaveData& data);
    void playWorldMusic();

    std::unique_ptr<LevelScene> scene;  // the live level behind this playthrough
    LevelClearSequence sequence;        // the flagpole clear cinematic
    std::unique_ptr<view::HudRenderer> hudRenderer;
    view::HudData hudData;
    bool levelComplete = false;

    bool hasSavedState = false;
    std::unique_ptr<model::GameSaveData> savedState;

    // Score and coin count at the start of the level — restored on death.
    // Updated each time a level is cleared so the next level starts from the cleared value.
    int checkpointScore = 0;
    int checkpointCoins = 0;
    
    float deathDelayTimer = -1.0f;
    bool playingStarmanMusic = false;
    model::WorldType m_lastKnownWorldType = model::WorldType::Overworld;

    // Cached save data loaded at level entry — avoids re-reading disk on every
    // captureSaveData() call (e.g. on pause). Updated by finishClear after a clear.
    model::GameSaveData m_cachedSaveBase;
    // Set by updateProgressAndUnlocks; read by syncProfileStats in same finishClear call.
    bool m_isFirstTimeClear = false;
};

}

#endif