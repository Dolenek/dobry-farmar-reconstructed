#include "GamePauseMenu.h"
#include "MainMenu.h"
#include "Options.h"
#include "Awards.h"
#include "YesNoDialog.h"
#include "SoundManager.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include <stdexcept>
namespace farmar::recovered {
namespace {GamePauseMenu* currentPause=nullptr;GamePauseContext* loadingPauseContext=nullptr;}
GamePauseLifetime::GamePauseLifetime(GamePauseMenu* value) {if(currentPause) throw "Object already exist.";currentPause=value;}
GamePauseLifetime::~GamePauseLifetime() {currentPause=nullptr;}
GamePauseMenu* GamePauseMenu::instance() noexcept {return currentPause;}
GamePauseMenu::GamePauseMenu(View* parent,GamePauseContext& context)
    :GamePauseLifetime(this),MenuView(parent,context.resources,context.settings,context.strings,context.views,context.controls),context_(context) {
    if(!context.discardGame || !context.resetGraphics || !context.screenSize) throw std::invalid_argument("GamePauseMenu requires engine services");
    const auto button=[&] {return new TextAnimationButton(this,context.resources,context.settings,context.strings,context.input,context.interaction,context.views,context.controls);};
    continue_=button();awards_=button();options_=button();main_=button();help_=button();
    auto xml=context.resources.loadSettingsXML(String("UI/Menu.xml"));loadXML(*xml);
    continue_->clickEvent=&continueClick;awards_->clickEvent=&awardsClick;options_->clickEvent=&optionsClick;main_->clickEvent=&mainClick;help_->clickEvent=&helpClick;
}
GamePauseMenu::~GamePauseMenu()=default;
void GamePauseMenu::loadXML(const XmlNode& node) {
    MenuView::loadXML(node);
    const std::pair<const char*,View*> children[]={{"ContinueButton",continue_},{"AwardsButton",awards_},{"OptionsButton",options_},{"MainMenuButton",main_},{"HelpButton",help_}};
    for(const auto& item:children) {XmlNode* child=nullptr;if(node.section(String(item.first),child)) item.second->loadXML(*child);}
}
void GamePauseMenu::onEnabled() {View::onEnabled();context_.input.bind(1,&resumeInput,nullptr);context_.input.bind(0x1c,&resumeInput,nullptr);}
void GamePauseMenu::onDisabled() {View::onDisabled();context_.input.unbind(1,&resumeInput,nullptr);context_.input.unbind(0x1c,&resumeInput,nullptr);}
void GamePauseMenu::resume() {context_.game.enable();context_.game.absoluteEnable();hideTransparently();}
Options* GamePauseMenu::openOptions() {
    auto* options=Options::instance();auto& c=context_;
    if(!options) {
        OptionsServices services;auto* sound=&c.sound;
        services.soundDisabled=[sound]{return sound->disabled();};services.soundVolume=[sound]{return sound->soundVolume();};services.musicVolume=[sound]{return sound->musicVolume();};
        services.setSoundVolume=[sound](unsigned value){sound->setSoundVolume(value);};services.setMusicVolume=[sound](unsigned value){sound->setMusicVolume(value);};services.resetGraphics=c.resetGraphics;
        options=new Options(c.resources,c.settings,c.strings,c.input,c.interaction,std::move(services),c.views,c.controls);options->hide();
    }
    hideTransparently();options->setReturnView(this);options->showTransparently();return options;
}
Awards* GamePauseMenu::openAwards() {
    if(Awards::instance()) return nullptr;
    hideTransparently();context_.game.hideTransparently();auto& c=context_;
    auto* awards=new Awards(nullptr,c.resources,c.settings,c.strings,c.sound,c.input,c.interaction,c.screenSize,c.views,c.controls);
    awards->returnEvent=&awardsReturned;awards->hide();awards->showTransparently();return awards;
}
void GamePauseMenu::awardsReturned() {auto* self=instance();self->showTransparently();self->context_.game.showTransparently();}
void GamePauseMenu::openHelp() {hideTransparently();context_.help.showTransparently();context_.help.enable();}
YesNoDialog* GamePauseMenu::requestMainMenu() {
    disable();hide();auto& c=context_;
    auto* dialog=new YesNoDialog(nullptr,c.resources,c.settings,c.strings,c.input,c.interaction,c.views,c.controls);
    dialog->setYesEvent(&mainYes);dialog->setNoEvent(&mainNo);
    dialog->text().setString(WString(L"\u0414\u0435\u0439\u0441\u0442\u0432\u0438\u0442\u0435\u043b\u044c\u043d\u043e \u0445\u043e\u0442\u0438\u0442\u0435\n \u0432\u044b\u0439\u0442\u0438?"),false);
    dialog->show();dialog->enable();return dialog;
}
void GamePauseMenu::mainNo(YesNoDialog* dialog) {auto* self=instance();self->registry_.requestDiscard(dialog,String(".\\Game\\Ui\\Menu.cpp194"));self->showTransparently();self->enable();}
void GamePauseMenu::mainYes(YesNoDialog* dialog) {
    auto* self=instance();auto& c=self->context_;loadingPauseContext=&c;
    c.views.requestDiscard(dialog,String(".\\Game\\Ui\\Menu.cpp181"));c.game.hide();c.discardGame(String(".\\Game\\Ui\\Menu.cpp185"));
    LoadingFromLog::create(c.files,c.resources,c.settings,c.strings,c.sound,c.music,String("menureslog.xml"),&mainLoaded,3,true,c.views,c.controls);
}
void GamePauseMenu::mainLoaded() {auto* menu=new MainMenu(loadingPauseContext->mainMenu);menu->hide();menu->showTransparently();}
void GamePauseMenu::continueClick(ButtonPrototype*) {instance()->resume();}
void GamePauseMenu::optionsClick(ButtonPrototype*) {instance()->openOptions();}
void GamePauseMenu::awardsClick(ButtonPrototype*) {instance()->openAwards();}
void GamePauseMenu::mainClick(ButtonPrototype*) {instance()->requestMainMenu();}
void GamePauseMenu::helpClick(ButtonPrototype*) {instance()->openHelp();}
void GamePauseMenu::resumeInput(void*) {instance()->resume();}
}
