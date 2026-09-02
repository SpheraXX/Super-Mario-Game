#ifndef CONTROLLER_AUDIODELEGATEIMPL_H
#define CONTROLLER_AUDIODELEGATEIMPL_H

#include "Model/Core/IAudioDelegate.h"

namespace controller {

class IAudioManager;

class AudioDelegateImpl : public model::IAudioDelegate {
public:
    explicit AudioDelegateImpl(IAudioManager* audioManager);
    ~AudioDelegateImpl() override = default;

    void playSound(const std::string& name) override;
    void playMusic(const std::string& name) override;
    void stopMusic() override;

private:
    IAudioManager* m_audioManager;
};

} // namespace controller

#endif // CONTROLLER_AUDIODELEGATEIMPL_H
