#pragma once
#include "MenuView.h"
#include "ButtonText.h"
#include "TooltipFrame.h"
#include <functional>
namespace farmar::recovered {
class FileIndex;class SoundManager;class MusicPlayback;class MainMenuContext;class Options;class Awards;class YesNoDialog;class GamePauseMenu;
// Engine-owned context must outlive pause screens and menu Loading completion.
struct GamePauseContext {
    FileIndex& files;ResourceManager& resources;Settings& settings;StringManager& strings;
    SoundManager& sound;MusicPlayback& music;InputBindings& input;ButtonInteraction& interaction;
    View& game;
    //00413210 constructs GameMenu/its pause menu before HelpView. Resolve the
    // actual late-created object only when00485630 opens it.
    std::function<View&()> help;
    MainMenuContext& mainMenu;TooltipFrame::ScreenSize screenSize;
    std::function<void(bool)> resetGraphics;
    // Required recovered 00486970 / 00432b20 save-and-discard operation.
    std::function<void(const String&)> discardGame;
    ViewRegistry& views;ControlRegistry& controls;
};
class GamePauseLifetime {
protected:
    explicit GamePauseLifetime(GamePauseMenu*);~GamePauseLifetime();
};
// Original cMenu singleton 0065ffd8, native 80 bytes; MenuView is at +4.
// Owned by GameMenu+0xa0; separate from the gameplay HUD GameMenu.
class GamePauseMenu final:private GamePauseLifetime,public MenuView {
public:
    GamePauseMenu(View*,GamePauseContext&); // 00487330
    ~GamePauseMenu() override; // 00487980
    void loadXML(const XmlNode&) override; // 00485680
    void onEnabled() override; // 00485d90
    void onDisabled() override; // 00486170
    void resume(); // 00485560
    Options* openOptions(); // 00485590
    Awards* openAwards(); // 00486550
    void openHelp(); // 00485630
    YesNoDialog* requestMainMenu(); // 00487210
    TextAnimationButton& continueButton() noexcept {return *continue_;}
    TextAnimationButton& optionsButton() noexcept {return *options_;}
    TextAnimationButton& awardsButton() noexcept {return *awards_;}
    TextAnimationButton& mainMenuButton() noexcept {return *main_;}
    TextAnimationButton& helpButton() noexcept {return *help_;}
    static GamePauseMenu* instance() noexcept;
private:
    GamePauseContext& context_;
    TextAnimationButton *continue_,*options_,*awards_,*main_,*help_;
    static void continueClick(ButtonPrototype*);static void optionsClick(ButtonPrototype*);
    static void awardsClick(ButtonPrototype*);static void mainClick(ButtonPrototype*);static void helpClick(ButtonPrototype*);
    static void resumeInput(void*);static void awardsReturned(); // 00485660
    static void mainYes(YesNoDialog*);static void mainNo(YesNoDialog*); // 00486d90 / 004870a0
    static void mainLoaded(); // 00489e80
};
}
