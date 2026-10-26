#include "StartupView.h"
#include "FileIndex.h"
#include "ResourceManager.h"
#include "Texture.h"
#include "OriginalArithmetic.h"
#include <stdexcept>
namespace farmar::recovered {
namespace {StartupContext* activeStartupContext=nullptr;bool startupInputToggle=true;} //initial byte at0065d290 is1
StartupContext::StartupContext(FileIndex& f,ResourceManager& r,Settings& s,StringManager& strings,SoundManager& sound,MusicPlayback& music,
    ProfileSession& profiles,InputBindings& input,ButtonInteraction& interaction,MainMenuContext& menu,ViewRegistry& views,ControlRegistry& controls)
    :files_(f),resources_(r),settings_(s),strings_(strings),sound_(sound),music_(music),profiles_(profiles),input_(input),menu_(menu),views_(views),controls_(controls),
     newUser_{r,s,strings,profiles,input,interaction,views,controls} {
    if(activeStartupContext) throw std::logic_error("Startup context already exists");activeStartupContext=this;
}
StartupContext::~StartupContext() {activeStartupContext=nullptr;}
void StartupContext::firstProfileReady() {
    auto& c=*activeStartupContext;c.menu_.ensureFixedBack();auto* user=new NewUser(c.newUser_);user->completed=&profileAccepted;
    user->hide();user->showTransparently();user->enable();
}
void StartupContext::menuReady() {auto* menu=new MainMenu(activeStartupContext->menu_);menu->hide();menu->showTransparently();}
void StartupContext::profileAccepted() {activeStartupContext->menu_.restore(true);}
StartupView::StartupView(StartupContext& c):View(nullptr,c.views_),context_(c) {
    c.input_.bind(InputBindings::AllControls,&inputEvent,this);
    logo_=new Picture(this,c.resources_,c.views_,c.controls_);logo_->setPosition({400,300});logo_->setDeep(10000);
    logo_->show();logo_->enable();show();enable();
}
StartupView::~StartupView() {context_.input_.unbind(InputBindings::AllControls,&inputEvent,this);}
void StartupView::loadLogo(const String& name,bool setFirst) {
    if(setFirst) logo_->setTextureName(name);
    auto* texture=context_.resources_.getTexture(name);
    if(!setFirst) logo_->setTextureName(name);
    logo_->setSize({static_cast<float>(originalMultiply(static_cast<float>(texture->allocatedWidth()),texture->scaleU())),
                    static_cast<float>(originalMultiply(static_cast<float>(texture->allocatedHeight()),texture->scaleV()))});
    texture->releaseReference();elapsed_=0;
}
void StartupView::nextLogo() {
    switch(stage_) {
    case 0:stage_=1;if(context_.files_.isExist(String("Logo.jpg"))) loadLogo(String("Logo.jpg"),false);else nextLogo();break;
    case 1:loadLogo(String("Logo/Logo2.jpg"),true);stage_=2;break;
    case 2:loadLogo(String("Logo/Logo3.jpg"),false);stage_=3;break;
    }
}
void StartupView::enabledStep() {
    const float dt=uiElapsedTime()();
    switch(stage_) {
    case 0:nextLogo();break;
    case 1:case 2:if(!(elapsed_<4.0f)) nextLogo();break;
    case 3:if(!(elapsed_<4.0f)) input();break;
    }
    const auto alpha=originalMultiply(elapsed_,0.5);logo_->setAlpha(alpha<1.0 ? static_cast<float>(alpha):1.0f);
    elapsed_=static_cast<float>(originalAdd(dt,elapsed_));
}
void StartupView::inputEvent(void* value) {static_cast<StartupView*>(value)->input();}
void StartupView::input() {
    startupInputToggle=!startupInputToggle;if(startupInputToggle) return;
    if(stage_!=3) {nextLogo();return;}
    if(started_) return;
    auto& c=context_;const bool exists=Profile::headersExist(c.files_,c.settings_);
    if(exists) c.profiles_.open(Profile::storedName(c.files_,c.settings_,[&](const String& name){return c.resources_.loadSettingsXML(name);}));
    LoadingFromLog::create(c.files_,c.resources_,c.settings_,c.strings_,c.sound_,c.music_,String("menureslog.xml"),
        exists ? &StartupContext::menuReady:&StartupContext::firstProfileReady,3,true,c.views_,c.controls_);
    started_=true;discardTransparently();
}
}
