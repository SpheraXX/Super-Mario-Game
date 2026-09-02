#ifndef MODEL_CORE_IAUDIODELEGATE_H
#define MODEL_CORE_IAUDIODELEGATE_H

#include <string>

namespace model {

class IAudioDelegate {
public:
    virtual ~IAudioDelegate() = default;

    virtual void playSound(const std::string& name) = 0;
    virtual void playMusic(const std::string& name) = 0;
    virtual void stopMusic() = 0;
};

} // namespace model

#endif // MODEL_CORE_IAUDIODELEGATE_H
