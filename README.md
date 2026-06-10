# CS202 Super Mario Game

## Overview
Welcome to the CS202 Super Mario Game project! This is a 2D Mario-style platformer developed using C++ and SFML. The game emphasizes Object-Oriented Programming (OOP) principles, featuring strict use of inheritance, encapsulation, polymorphism, abstraction, and various Design Patterns.

**Links:**
* **Demo Video:** [TBD]
* **GitHub Repository:** [TBD]

---

## Requirements
To compile and run this project, you will need the following dependencies installed on your system:
* **C++ Compiler:** Supporting C++17 or higher (e.g., GCC, Clang, or MSVC).
* **CMake:** Version 3.10 or higher.
* **SFML (Simple and Fast Multimedia Library):** Used for rendering graphical components, playing audio, and window management.

---

## Project Layout
The repository is strictly structured using the **Model-View-Controller (MVC)** architectural pattern:

```text
Mario/
├── include/                # Header files (.h)
│   ├── Model/              # Game state, character logic, level data
│   ├── View/               # Graphics, animations, SFML rendering
│   └── Controller/         # Input handling, game loops, game manager
├── src/                    # Implementation files (.cpp)
│   ├── Model/              
│   ├── View/               
│   ├── Controller/         
│   └── main.cpp            # Application entry point
├── assets/                 # Textures, sounds, fonts, and level files
├── CMakeLists.txt          # CMake configuration
└── run.bat / run.command   # Automated build scripts for Windows/Mac
```

---

## How to Build and Run

We use CMake to ensure cross-platform compatibility across Windows and macOS.

### 1. Manual Build (via CMake)
If you prefer to compile manually using a terminal or an IDE (like VS Code or CLion):
```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```
After a successful build, the executable will be generated inside the `build/` directory.

---

## Game Features

* **Characters:** Play as Mario or Luigi, complete with unique abilities. Switchable mid-game!
* **Levels:** 3 full levels with increasing difficulty and engaging level design.
* **Items & Power-ups:** Classic items like Mushrooms (growth), Coins (score), and FireFlowers.
* **Enemies:** Advanced AI behaviors for classic enemies like Goombas and Koopas.
* **Save / Load:** State serialization to allow you to save your progress and continue later.
* **Bonus (Level Editor):** Build your own Mario levels, serialize them, and play them instantly!

## Technical Notes
* **Design Patterns:** The game leverages Factory, Singleton, Observer, State, and Strategy patterns for robust and modular codebase architecture.
