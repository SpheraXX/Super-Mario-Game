cmake_minimum_required(VERSION 3.15)
project(SuperMario CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile_commands.json for Clangd and VS Code Intellisense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------
# 1. FETCH DEPENDENCIES (SFML 3.0.2)
# ---------------------------------------------------------
include(FetchContent)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        3.0.2
)
FetchContent_MakeAvailable(SFML)

# Include directory path
include_directories(include)

# Collect all implementation source files
file(GLOB_RECURSE SOURCES 
    src/*.cpp
    src/Model/*.cpp
    src/View/*.cpp
    src/Controller/*.cpp
)

# Output directories configuration
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Create executable target
add_executable(SuperMario ${SOURCES})

# Link SFML libraries
target_link_libraries(SuperMario PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
)

# Copy assets to build directory for easy execution
add_custom_target(copy-assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/bin/assets
    COMMENT "Copying assets to bin directory..."
)
add_dependencies(SuperMario copy-assets)
