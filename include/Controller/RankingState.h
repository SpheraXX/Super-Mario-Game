#ifndef CONTROLLER_RANKINGSTATE_H
#define CONTROLLER_RANKINGSTATE_H

#include "Controller/GameState.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <vector>
#include <memory>

namespace view {
namespace ui {
class UIButton;
}
}

namespace controller {

class RankingState : public GameState {
public:
    RankingState();
    ~RankingState() override = default;

    void onEnter() override;
    void onDisplayModeChanged() override;
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

private:
    void buildUI();

    sf::Sprite bgaSprite;
    sf::Sprite titleSprite;
    sf::RectangleShape overlay;
    
    std::unique_ptr<view::ui::UIButton> backBtn;
    
    std::vector<sf::Text> headerTexts;
    std::vector<sf::Text> rowTexts;
    
    struct RankEntry {
        std::string name;
        int score;
        int passed;
        bool isEmpty;
    };
    std::vector<RankEntry> rankedProfiles;
    
    float m_totalLevels = 0;
};

} // namespace controller

#endif // CONTROLLER_RANKINGSTATE_H
