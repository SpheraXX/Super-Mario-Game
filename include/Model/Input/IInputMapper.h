#ifndef MODEL_IINPUTMAPPER_H
#define MODEL_IINPUTMAPPER_H

namespace model {

enum class InputAction {
    MoveLeft,
    MoveRight,
    Jump,
    Run,
    Pause
};

class IInputMapper {
public:
    virtual ~IInputMapper() = default;
    virtual bool isActionPressed(InputAction action) const = 0;
};

} // namespace model

#endif
