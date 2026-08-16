cmake_minimum_required(VERSION 3.12)
project(SuperMario CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile_commands.json for Clangd and VS Code Intellisense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Write includes directly in compile_commands.json (not in .rsp files)
# so that Clangd / IntelliSense can resolve SFML headers
set(CMAKE_USE_RESPONSE_FILE_FOR_INCLUDES OFF)

# Find SFML package
find_package(SFML 3.0 COMPONENTS Graphics Window System Audio REQUIRED)

# Include directory path
include_directories(include)

# Collect all implementation source files.
#
# CONFIGURE_DEPENDS makes the build re-check this glob every time it runs. Without it the
# file list is captured once, when CMake configures, and never revisited — so pulling a
# teammate's branch that adds a .cpp leaves that file out of the build. It still compiles
# (the headers are found), then fails at link with "undefined reference" to everything the
# new file defines, which looks nothing like the missing-file problem it actually is.
# The cost is a directory stat per build; the alternative is every teammate hitting that
# error and having to know to re-run cmake.
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
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
