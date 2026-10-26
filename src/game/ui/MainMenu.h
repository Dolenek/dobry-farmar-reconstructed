#pragma once
#include "MainMenuContent.h"
#include "LoadingFromLog.h"
#include "TooltipFrame.h"
#include <functional>
namespace farmar::recovered {
class FileIndex;class ProfileSession;class SoundManager;class MusicPlayback;
class ExitDialog;class SelectProfile;class Options;class Credits;class Awards;class MainMenu;
// Modern dependency ownership for the original engine globals. This context
// must outlive its screens and Loading completion. Its required game factory
// is the engine's recovered 00413210 operation, not a default/no-op substitute.
class MainMenuContext final {
public:
    MainMenuContext(FileIndex&,ResourceManager&,Settings&,StringManager&,SoundManager&,MusicPlayback&,
        ProfileSession&,InputBindings&,ButtonInteraction&,TooltipFrame::ScreenSize,
        std::function<void(bool)> resetGraphics,LoadingFromLog::Completion constructGame,
        ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry());
    ~MainMenuContext();
    MainMenuContext(const MainMenuContext&)=delete;
    MainMenuContext& operator=(const MainMenuContext&)=delete;
    MainMenu& restore(bool updateName); // 00481860 / 00485490
    MenuView* fixedBack() const noexcept { return fixedBack_; }
    void ensureFixedBack(); //00481990, also used before first NewUser
private:
    friend class MainMenu;
    FileIndex& files_;ResourceManager& resources_;Settings& settings_;StringManager& strings_;
    SoundManager& sound_;MusicPlayback& music_;ProfileSession& profiles_;InputBindings& input_;
    ButtonInteraction& interaction_;TooltipFrame::ScreenSize screenSize_;
    std::function<void(bool)> resetGraphics_;LoadingFromLog::Completion constructGame_;
    ViewRegistry& views_;ControlRegistry& controls_;MenuView* fixedBack_=nullptr;
};
class MainMenuLifetime {
protected:
    explicit MainMenuLifetime(MainMenu*);
    ~MainMenuLifetime();
};
// Original cMainMenu is 96 bytes, singleton base + MenuView at +4. Modern
// dependency/context layout differs; recovered actions use real owned screens.
class MainMenu final : private MainMenuLifetime,public MenuView {
public:
    explicit MainMenu(MainMenuContext&); // 00484da0
    ~MainMenu() override; // 00483e90
    void loadXML(const XmlNode&) override; // 00481e10
    void onEnabled() override; // 004836d0, calls original base onDisabled
    void onDisabled() override; // 00483ab0, calls original base onEnabled
    void enabledStep() override; // 00481770
    LoadingFromLog* startGame(); // 00484700, completion requires actual cGame factory
    ExitDialog* requestExit(); // 004817d0
    SelectProfile* changeProfile(); // 004818f0
    Options* openOptions(); // 004826c0
    Credits* openCredits(); // 00482ef0
    Awards* openAwards(); // 00482ae0
    MainMenuContent& content() noexcept { return content_; }
    static MainMenu* instance() noexcept;
private:
    MainMenuContext& context_;MainMenuContent content_;
    void ensureFixedBack(); // 00481990, ordinary show, only once
    static void startClick(ButtonPrototype*);static void exitClick(ButtonPrototype*);
    static void profileClick(ButtonPrototype*);static void optionsClick(ButtonPrototype*);
    static void creditsClick(ButtonPrototype*);static void awardsClick(ButtonPrototype*);
    static void escape(void*);static void exitNo(ExitDialog*); // 004816d0
    static void exitYes(ExitDialog*); // 004817c0 -> 005306c0 PostQuitMessage(0)
    static void profileAccepted();static void profileCancelled();
    static void awardsReturned(); // 004817a0
    static void loadingCompleted(); // 004832f0, invokes required engine game factory
};
}
