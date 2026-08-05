#include "Model/Map/TileMap.h"
#include "Model/Player/Mario.h"
#include "Model/Player/Luigi.h"
#include "View/Map/TileMapRenderer.h"

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
    freopen("log.txt", "w", stderr);

    std :: cerr << "START" << std :: endl;

    try {    
        controller::AppEngine engine;
        engine.run();
    } catch (const std::exception& exception) {
        std :: cerr << exception.what() << std :: endl;
        return 1;
    }

    std :: cerr << "END" << std :: endl;

    return 0;
}
