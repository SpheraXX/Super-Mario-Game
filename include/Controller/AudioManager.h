#ifndef CONTROLLER_AUDIOMANAGER_H
#define CONTROLLER_AUDIOMANAGER_H

#include "Controller/IAudioManager.h"
#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <map>
#include <list>

namespace controller {

class AudioManager : public IAudioManager {
public:
    AudioManager();
    ~AudioManager() override = default;

    void setMasterVolume(float volume) override;
    void setMusicVolume(float volume) override;
    void setSFXVolume(float volume) override;

    void playMusic(const std::string& name) override;
    void stopMusic() override;
    
    void playSound(const std::string& name) override;

    // Call this occasionally (e.g. in game loop) to remove finished sounds
    void update();

private:
    struct MusicMetadata {
        std::string filename;
        bool loop;
    };
    std::unordered_map<std::string, MusicMetadata> musicDB;
    void initDatabase();

    void applyMusicVolume();
    void applySFXVolume();

    float masterVol = 100.f;
    float musicVol = 100.f;
    float sfxVol = 100.f;

    sf::Music currentMusic;
    std::string currentMusicName;

    // Cache of loaded sound buffers
    std::map<std::string, sf::SoundBuffer> soundBuffers;
    
    // List of active playing sounds
    std::list<sf::Sound> activeSounds;
};

}

#endif // CONTROLLER_AUDIOMANAGER_H
