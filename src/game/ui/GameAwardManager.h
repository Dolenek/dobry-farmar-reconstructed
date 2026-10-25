#pragma once
#include "GameMenu.h"
#include "Awards.h"
#include "AwardEffect.h"
namespace farmar::recovered {
struct GameAwardContext {
    ResourceManager& resources;Settings& settings;StringManager& strings;SoundManager& sound;
    InputBindings& input;ButtonInteraction& interaction;ProfileProperties& profile;
    View& game;GameMenuWorld& world;TooltipFrame::ScreenSize screenSize;ViewRegistry& views;ControlRegistry& controls;
};
class GameAwardManager;
class GameAwardManagerLifetime {protected:explicit GameAwardManagerLifetime(GameAwardManager*);~GameAwardManagerLifetime();};
// Native cAwardsWindow224 bytes, singleton0065f6e8. Distinct from Awards screen.
class GameAwardManager final:public View,private GameAwardManagerLifetime {
public:
    explicit GameAwardManager(GameAwardContext&); //004c7d00
    ~GameAwardManager() override; //004c8630
    static GameAwardManager* instance() noexcept;
    void loadXML(const XmlNode&) override; //004c6d20, View XML only
    void enabledStep() override; //004c8650
    bool condition(unsigned);
    std::array<bool,21> attained{};AwardEffect* pending=nullptr;int requested=-1;
private:
    GameAwardContext& context_;
    static void returned(); //004c70e0, resume whole cGame
};
}
