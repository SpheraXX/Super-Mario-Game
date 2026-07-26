#include "Controller/AppEngine.h"

#include <exception>
#include <iostream>

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
