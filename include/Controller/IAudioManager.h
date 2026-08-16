#ifndef CONTROLLER_IAUDIOMANAGER_H
#define CONTROLLER_IAUDIOMANAGER_H

#include <string>

namespace controller {

class IAudioManager {
public:
    virtual ~IAudioManager() = default;

    virtual void setMasterVolume(float volume) = 0;
    virtual void setMusicVolume(float volume) = 0;
    virtual void setSFXVolume(float volume) = 0;

    virtual void playMusic(const std::string& name) = 0;
    virtual void stopMusic() = 0;
    
    virtual void playSound(const std::string& name) = 0;
};

}

#endif // CONTROLLER_IAUDIOMANAGER_H
