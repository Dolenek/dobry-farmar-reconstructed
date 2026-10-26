#include "MainMenu.h"
#include "ResourceManager.h"
#include "ProfileSession.h"
#include "Settings.h"
#include "SoundManager.h"
#include "MusicPlayback.h"
#include "ExitDialog.h"
#include "SelectProfile.h"
#include "Options.h"
#include "Credits.h"
#include "Awards.h"
#include <stdexcept>
namespace farmar::recovered {
namespace { MainMenu* currentMenu=nullptr;MainMenuContext* currentContext=nullptr; }
MainMenuContext::MainMenuContext(FileIndex& files,ResourceManager& resources,Settings& settings,StringManager& strings,
    SoundManager& sound,MusicPlayback& music,ProfileSession& profiles,InputBindings& input,ButtonInteraction& interaction,
    TooltipFrame::ScreenSize screenSize,std::function<void(bool)> resetGraphics,LoadingFromLog::Completion constructGame,
    ViewRegistry& views,ControlRegistry& controls)
    :files_(files),resources_(resources),settings_(settings),strings_(strings),sound_(sound),music_(music),profiles_(profiles),
     input_(input),interaction_(interaction),screenSize_(std::move(screenSize)),resetGraphics_(std::move(resetGraphics)),
     constructGame_(constructGame),views_(views),controls_(controls) {
    if(currentContext) throw "Object already exist.";
    if(!screenSize_ || !resetGraphics_ || !constructGame_) throw std::invalid_argument("MainMenu requires graphics and actual game construction services");
    currentContext=this;
}
MainMenuContext::~MainMenuContext() { currentContext=nullptr; }
MainMenu& MainMenuContext::restore(bool updateName) {
    auto* menu=MainMenu::instance();if(!menu) menu=new MainMenu(*this);
    menu->showTransparently();menu->enable();
    if(updateName) menu->content().userName->setString(profiles_.current()->headers().currentName());
    return *menu;
}
MainMenuLifetime::MainMenuLifetime(MainMenu* menu) { if(currentMenu) throw "Object already exist.";currentMenu=menu; }
MainMenuLifetime::~MainMenuLifetime() { currentMenu=nullptr; }
MainMenu* MainMenu::instance() noexcept { return currentMenu; }
MainMenu::MainMenu(MainMenuContext& context)
    :MainMenuLifetime(this),MenuView(nullptr,context.resources_,context.settings_,context.strings_,context.views_,context.controls_),
     context_(context),content_(*this,context.resources_,context.settings_,context.strings_,context.input_,context.interaction_,context.views_,context.controls_) {
    { const auto xml=context.resources_.loadSettingsXML(String("UI/MainMenu.xml"));loadXML(*xml); }
    content_.start->clickEvent=&startClick;content_.exit->clickEvent=&exitClick;
    content_.changeUser->clickEvent=&profileClick;content_.options->clickEvent=&optionsClick;
    // MainMenuContent preserves native allocation order: its first final
    // button is native +54 (Credits action), second +50 (Awards action).
    content_.awards->clickEvent=&creditsClick;content_.credits->clickEvent=&awardsClick;
}
MainMenu::~MainMenu() { onDisabled();delete context_.fixedBack_;context_.fixedBack_=nullptr; }
void MainMenuContext::ensureFixedBack() {
    if(fixedBack_) return;
    auto* back=new MenuView(nullptr,resources_,settings_,strings_,views_,controls_);fixedBack_=back;
    const auto xml=resources_.loadSettingsXML(String("Ui/FixedBack.xml"));back->loadXML(*xml);back->show();
}
void MainMenu::ensureFixedBack() {context_.ensureFixedBack();}
void MainMenu::loadXML(const XmlNode& node) {
    const auto* profile=context_.profiles_.current();
    content_.loadXML(node,[this] { ensureFixedBack(); },profile ? &profile->headers().currentName():nullptr);
}
void MainMenu::onEnabled() { View::onDisabled();context_.input_.bind(1,&escape,static_cast<View*>(this)); }
void MainMenu::onDisabled() { View::onEnabled();context_.input_.unbind(1,&escape,static_cast<View*>(this)); }
void MainMenu::enabledStep() {
    const auto* profile=context_.profiles_.current();content_.updateProfileName(profile ? &profile->headers().currentName():nullptr);
}
ExitDialog* MainMenu::requestExit() {
    disable();hideTransparently();
    auto* dialog=new ExitDialog(nullptr,context_.resources_,context_.settings_,context_.strings_,context_.input_,context_.interaction_,context_.views_,context_.controls_);
    dialog->yesEvent=&exitYes;dialog->noEvent=&exitNo;dialog->hide();dialog->showTransparently();dialog->enable();return dialog;
}
void MainMenu::exitNo(ExitDialog* dialog) { dialog->discardTransparently();instance()->showTransparently();instance()->enable(); }
void MainMenu::exitYes(ExitDialog*) { PostQuitMessage(0); }
SelectProfile* MainMenu::changeProfile() {
    disable();hideTransparently();
    auto* selector=new SelectProfile(false,true,context_.profiles_,context_.resources_,context_.settings_,context_.strings_,context_.input_,context_.interaction_,context_.views_,context_.controls_);
    selector->accepted=&profileAccepted;selector->cancelled=&profileCancelled;
    selector->hide();selector->showTransparently();selector->enable();return selector;
}
void MainMenu::profileAccepted() { currentContext->restore(true); }
void MainMenu::profileCancelled() { currentContext->restore(false); }
Options* MainMenu::openOptions() {
    if(Options::instance()) return nullptr;
    hideTransparently();OptionsServices services;
    auto* sound=&context_.sound_;
    services.soundDisabled=[sound] { return sound->disabled(); };services.soundVolume=[sound] { return sound->soundVolume(); };
    services.musicVolume=[sound] { return sound->musicVolume(); };services.setSoundVolume=[sound](unsigned value) { sound->setSoundVolume(value); };
    services.setMusicVolume=[sound](unsigned value) { sound->setMusicVolume(value); };services.resetGraphics=context_.resetGraphics_;
    auto* options=new Options(context_.resources_,context_.settings_,context_.strings_,context_.input_,context_.interaction_,std::move(services),context_.views_,context_.controls_);
    options->hide();options->showTransparently();options->setReturnView(this);return options;
}
Credits* MainMenu::openCredits() {
    if(Credits::instance()) return nullptr;
    hideTransparently();auto* credits=new Credits(*this,context_.resources_,context_.settings_,context_.strings_,context_.input_,context_.interaction_,context_.views_,context_.controls_);
    credits->showTransparently();return credits;
}
Awards* MainMenu::openAwards() {
    if(Awards::instance()) return nullptr;
    hideTransparently();auto* awards=new Awards(nullptr,context_.resources_,context_.settings_,context_.strings_,context_.sound_,context_.input_,context_.interaction_,context_.screenSize_,context_.views_,context_.controls_);
    awards->returnEvent=&awardsReturned;awards->hide();awards->showTransparently();return awards;
}
void MainMenu::awardsReturned() { instance()->showTransparently();instance()->enable(); }
LoadingFromLog* MainMenu::startGame() {
    registry_.requestDiscard(this,String(".\\Game\\Ui\\MainMenu.cpp153"));hide();
    bool log=false;context_.settings_.get(String("game"),String("LogResources"),log);
    if(log) context_.resources_.logResources(String("MenuResLog.xml"));
    return LoadingFromLog::create(context_.files_,context_.resources_,context_.settings_,context_.strings_,context_.sound_,context_.music_,
        String("GameResLog.xml"),&loadingCompleted,30,false,context_.views_,context_.controls_);
}
void MainMenu::loadingCompleted() { currentContext->constructGame_(); }
void MainMenu::startClick(ButtonPrototype*) { instance()->startGame(); }
void MainMenu::exitClick(ButtonPrototype*) { instance()->requestExit(); }
void MainMenu::profileClick(ButtonPrototype*) { instance()->changeProfile(); }
void MainMenu::optionsClick(ButtonPrototype*) { instance()->openOptions(); }
void MainMenu::creditsClick(ButtonPrototype*) { instance()->openCredits(); }
void MainMenu::awardsClick(ButtonPrototype*) { instance()->openAwards(); }
void MainMenu::escape(void*) { instance()->requestExit(); }
}
