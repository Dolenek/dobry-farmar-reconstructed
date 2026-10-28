#pragma once
#include "View.h"
#include "TooltipFrame.h"
#include <functional>
#include <memory>
namespace farmar::recovered {
class FileIndex;class ResourceManager;class Settings;class StringManager;class SoundManager;class Profile;class GameRandom;
class InputBindings;class ButtonInteraction;class GamePauseContext;class Game;class Farm;class Market;class Manufacture;
class GameMenu;class Gameplay;class SalesHistory;class GameConfiguration;class TutorialManager;class Person;
struct GameContext {
    FileIndex& files;ResourceManager& resources;Settings& settings;StringManager& strings;SoundManager& sound;
    Profile& profile;GameRandom& random;InputBindings& input;ButtonInteraction& interaction;
    TooltipFrame::ScreenSize screenSize;
    // Engine-owned pause context survives deferred game deletion / menu Loading.
    std::function<GamePauseContext&(Game&)> pauseContext;
    ViewRegistry& views;ControlRegistry& controls;
};
class GameLifetime {protected:explicit GameLifetime(Game*);~GameLifetime();};
// Actual cGame factory and concrete cross-system bindings, singleton0065f6ec.
// Source layout differs from the original2716 bytes.
class Game final:public View,private GameLifetime {
public:
    explicit Game(GameContext&); //00413210
    ~Game() override; //00410830
    void loadXML(const XmlNode&) override; //0040e6e0
    static Game* instance() noexcept;
    int money() const;void setMoney(int); //00410ce0 /00410d60
    void refreshUpgrades(); //0040be10
    void cheatMoney();void cheatUpgrades();void cheatSeeds();void cheatProducts(); //00412630/50/b0,0040c360
    void save(); //0040fbb0
    Farm& farm();Market& market();Manufacture& manufacture();GameMenu& hud();Person& person();
    Gameplay& gameplay();SalesHistory& sales();GameConfiguration& configuration();TutorialManager& tutorial();
private:
    struct World;std::unique_ptr<World> world_;
};
}
