#ifndef MODEL_SETTINGS_H
#define MODEL_SETTINGS_H

#include <string>

namespace model {

enum class Language { English, Vietnamese };
enum class GraphicsQuality { Low, Medium, High };

struct Settings {
    bool             fullscreen    = false;
    int              logicalWidth  = 384;   
    GraphicsQuality  quality       = GraphicsQuality::Low;

    int  masterVolume  = 100;  
    int  musicVolume   = 80;
    int  sfxVolume     = 100;

    Language language  = Language::English;

    int keyMoveLeft   = -1;  
    int keyMoveRight  = -1;  
    int keyJump       = -1;  
    int keyRun        = -1;  
    int keyPause      = -1;  

    int controlSlot   = 0;

    static Settings defaults();
};

}  // namespace model

#endif
