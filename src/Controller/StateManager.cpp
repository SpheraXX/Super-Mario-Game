#include "Controller/StateManager.h"

#include <SFML/Graphics/RenderTarget.hpp>

#include <iostream>

#include <stdexcept>

namespace controller {

StateManager::StateManager(std::unique_ptr<GameState> initial, GameContext* ctx) : context(ctx) {
    if (!initial) {
        throw std::invalid_argument("StateManager must start with a valid state");
    }
    initial->manager = this;
    initial->context = context;
    initial->onEnter();
    stack.push_back(std::move(initial));
}

void StateManager::pushState(std::unique_ptr<GameState> state) {
    pending.push_back({Action::Push, std::move(state)});
}

void StateManager::popState() {
    pending.push_back({Action::Pop, nullptr});
}

void StateManager::replaceState(std::unique_ptr<GameState> state) {
    pending.push_back({Action::Replace, std::move(state)});
}

void StateManager::clear() {
    pending.push_back({Action::Clear, nullptr});
}

void StateManager::applyPending() {
    for (auto& change : pending) {
        switch (change.action) {
            case Action::Push:
                change.state->manager = this;
                change.state->context = context;
                stack.push_back(std::move(change.state));
                stack.back()->onEnter();
                break;

            case Action::Pop:
                if (!stack.empty()) {
                    stack.back()->onExit();
                    stack.pop_back();
                }
                break;

            case Action::Replace:
                if (!stack.empty()) {
                    stack.back()->onExit();
                    stack.pop_back();
                }
                change.state->manager = this;
                change.state->context = context;
                stack.push_back(std::move(change.state));
                stack.back()->onEnter();
                break;

            case Action::Clear:
                while (!stack.empty()) {
                    stack.back()->onExit();
                    stack.pop_back();
                }
                break;
        }
    }
    pending.clear();
}

void StateManager::handleEvent(const sf::Event& event) {
    if (GameState* state = activeState()) {
        state->handleEvent(event);
    }
}

void StateManager::update(float deltaTime) {
    if (GameState* state = activeState()) {
        state->update(deltaTime);
    }
}

void StateManager::render(sf::RenderTarget& window) {
    if (stack.empty()) {
        return;
    }

    // std :: cerr << "Rendering state stack of size: " << stack.size() << std :: endl;

    // Find the lowest state that must be drawn: walk down while states are transparent,
    // then render from that state up so overlays composite over what is beneath them.
    std::size_t bottom = stack.size() - 1;
    while (bottom > 0 && stack[bottom]->isTransparent()) {
        --bottom;
    }

    for (std::size_t index = bottom; index < stack.size(); ++index) {
        // std :: cerr << "The type of the state at index " << index << " is: " << typeid(*stack[index]).name() << std :: endl;
        stack[index]->render(window);
        // std :: cerr << "Rendered state at index: " << index << std :: endl;
    }
}

bool StateManager::empty() const {
    return stack.empty();
}

GameState* StateManager::activeState() {
    if (stack.empty()) {
        return nullptr;
    }
    return stack.back().get();
}

}
