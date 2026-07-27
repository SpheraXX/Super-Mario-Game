#include "Model/TileMap.h"
#include "Model/Mario.h"
#include "Model/Luigi.h"
#include "View/TileMapRenderer.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/Keyboard.hpp>
#include "Controller/AppEngine.h"

#include <exception>
#include <iostream>
#include <memory>
#include <cstdio>

int main() {
    try {
        
        controller::AppEngine engine;
        engine.run();
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << '\n';
        return 1;
    }

    return 0;
}
