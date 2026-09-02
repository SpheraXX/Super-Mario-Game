#ifndef MODEL_SETTINGS_H
#define MODEL_SETTINGS_H

#include <string>

namespace model {

enum class Language { English, Vietnamese };
enum class GraphicsQuality { Low, Medium, High };
enum class AspectRatio { Ratio4x3, Ratio16x9 };

struct Settings {
    bool             fullscreen       = false;
    AspectRatio      ratio            = AspectRatio::Ratio4x3;
    int              resolutionIndex  = 0;
    GraphicsQuality  quality          = GraphicsQuality::Medium;
    bool             vsync            = true;

    int  masterVolume  = 100;  
    int  musicVolume   = 80;
    int  sfxVolume     = 100;

    Language language  = Language::English;

    int keyMoveLeft   = -1;  
    int keyMoveRight  = -1;  
    int keyJump       = -1;  
    int keyRun        = -1;  
    int keyPause      = -1;  
    int keyDash       = -1;
    int keyAttack     = -1;
    int keyCrouch     = -1;
    int keyInteract   = -1;
    int keyInventory  = -1;
    int keyCycleDisplay = -1;

    int controlSlot   = 0;

    bool operator==(const Settings& o) const {
        return fullscreen == o.fullscreen && ratio == o.ratio &&
               resolutionIndex == o.resolutionIndex &&
               quality == o.quality && vsync == o.vsync && masterVolume == o.masterVolume &&
               musicVolume == o.musicVolume && sfxVolume == o.sfxVolume &&
               language == o.language && keyMoveLeft == o.keyMoveLeft &&
               keyMoveRight == o.keyMoveRight && keyJump == o.keyJump &&
               keyRun == o.keyRun && keyPause == o.keyPause &&
               keyDash == o.keyDash && keyAttack == o.keyAttack &&
               keyCrouch == o.keyCrouch && keyInteract == o.keyInteract &&
               keyInventory == o.keyInventory && keyCycleDisplay == o.keyCycleDisplay &&
               controlSlot == o.controlSlot;
    }
    bool operator!=(const Settings& o) const {
        return !(*this == o);
    }

    static Settings defaults();
};

}  // namespace model

#endif
