#ifndef MODEL_CORE_LEVELTIMER_H
#define MODEL_CORE_LEVELTIMER_H

namespace model {

// SMB-style level countdown: counts down from a starting value at one tick per real
// second. Reaching zero means the player ran out of time (a death in PlayState).
// Pausing freezes the clock so the remaining time can be read off after the level ends.
class LevelTimer {
public:
    explicit LevelTimer(float startSeconds = 400.0f);

    void update(float deltaTime);
    void pause();
    void resume();
    void reset(float startSeconds);

    bool isPaused() const;
    bool isExpired() const;
    // The value shown on the HUD (ceil, so it never flashes 399 in the first frame).
    int getRemainingSeconds() const;
    float getRemaining() const;

private:
    float remaining;
    bool paused;
};

}

#endif
