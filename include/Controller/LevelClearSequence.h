#ifndef CONTROLLER_LEVELCLEARSEQUENCE_H
#define CONTROLLER_LEVELCLEARSEQUENCE_H

namespace controller {
class LevelScene;
}

namespace controller {

// The scripted clear play that runs the moment Mario touches the flagpole: he hugs the
// pole and slides down with the pennant, auto-walks to the painted castle's door and
// stops — the frozen tableau the completion overlay is pushed over.
//
// begin() also awards the clear bonus (flag height + remaining time) and pauses the
// scene's timer, snapshotting the geometry the animation drives through the scene's
// accessors. The sequence never touches the StateManager: when isFinished() turns true
// the owning state pushes the completion overlay and unfreezes nothing.
class LevelClearSequence {
public:
    enum class Phase {
        SlideToPole,   // penguin + mario slide down the pole
        WalkToCastle,  // mario auto-walks from the pole to the castle
        ReachedCastle, // mario stands at the door; the run is finished
    };

    // Start the cinematic against `scene`: pause its timer, award the flag + time bonus
    // through GameManager, and snapshot the geometry the animation drives. Requires the
    // scene to have a live player and flagpole (the owner checks before calling).
    void begin(LevelScene& scene);

    // Advance the cinematic. No-op unless active and not yet finished; when the player
    // or the pole unexpectedly vanish, the sequence marks itself finished so the owner
    // falls through to the overlay.
    void update(float deltaTime);

    bool isActive() const;
    bool isFinished() const;

private:
    LevelScene* scenePtr = nullptr;
    Phase phase = Phase::SlideToPole;
    bool active = false;
    bool finished = false;
    float poleElapsed = 0.0f;
    float poleSlideStartY = 0.0f;
    float poleGroundY = 0.0f;
};

}

#endif