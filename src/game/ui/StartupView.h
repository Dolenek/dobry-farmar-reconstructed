#pragma once
#include "NewUser.h"
#include "MainMenu.h"
namespace farmar::recovered {
class StartupContext final {
public:
    StartupContext(FileIndex&,ResourceManager&,Settings&,StringManager&,SoundManager&,MusicPlayback&,ProfileSession&,InputBindings&,ButtonInteraction&,MainMenuContext&,ViewRegistry&,ControlRegistry&);
    ~StartupContext();
    StartupContext(const StartupContext&)=delete;StartupContext& operator=(const StartupContext&)=delete;
private:
    friend class StartupView;
    FileIndex& files_;ResourceManager& resources_;Settings& settings_;StringManager& strings_;SoundManager& sound_;MusicPlayback& music_;
    ProfileSession& profiles_;InputBindings& input_;MainMenuContext& menu_;ViewRegistry& views_;ControlRegistry& controls_;NewUserContext newUser_;
    static void firstProfileReady();static void menuReady();static void profileAccepted();
};
// Original60-byte cView with owned Picture +2c, started+30, elapsed+34, stage+38.
class StartupView final:public View {
public:
    explicit StartupView(StartupContext&); //0048b4a0
    ~StartupView() override; //0048b920
    void enabledStep() override; //0048b030
    void nextLogo(); //00489ef0
    void input(); //0048ab10, shared process toggle preserved
    Picture& logo() noexcept {return *logo_;}
    unsigned stage() const noexcept {return stage_;}float elapsed() const noexcept {return elapsed_;}bool started() const noexcept {return started_;}
private:
    StartupContext& context_;Picture* logo_=nullptr;bool started_=false;float elapsed_=0;unsigned stage_=0;
    static void inputEvent(void*);void loadLogo(const String&,bool setFirst);
};
}
