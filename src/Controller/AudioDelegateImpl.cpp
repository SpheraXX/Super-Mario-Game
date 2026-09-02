#include "Controller/AudioDelegateImpl.h"
#include "Controller/IAudioManager.h"

namespace controller {

AudioDelegateImpl::AudioDelegateImpl(IAudioManager* audioManager) 
    : m_audioManager(audioManager) {}

void AudioDelegateImpl::playSound(const std::string& name) {
    if (m_audioManager) {
        m_audioManager->playSound(name);
    }
}

void AudioDelegateImpl::playMusic(const std::string& name) {
    if (m_audioManager) {
        m_audioManager->playMusic(name);
    }
}

void AudioDelegateImpl::stopMusic() {
    if (m_audioManager) {
        m_audioManager->stopMusic();
    }
}

} // namespace controller
