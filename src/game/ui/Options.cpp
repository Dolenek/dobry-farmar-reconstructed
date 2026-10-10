#include "Options.h"
#include "ResourceManager.h"
#include "Settings.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include <cmath>
#include <stdexcept>
namespace farmar::recovered {
namespace {
Options* currentOptions=nullptr;
unsigned volumeInteger(float value) noexcept {
#if defined(_MSC_VER) && defined(_M_IX86)
    unsigned short originalControl,truncatedControl;long long result;
    __asm {
        fnstcw originalControl
        mov ax, originalControl
        or ax, 0c00h
        mov truncatedControl, ax
        fldcw truncatedControl
        fld value
        fistp result
        fldcw originalControl
    }
    return static_cast<unsigned>(result);
#else
    return static_cast<unsigned>(static_cast<long long>(std::trunc(value)));
#endif
}
}
Options* Options::instance() noexcept { return currentOptions; }
Options::Options(ResourceManager& resources,Settings& settings,StringManager& strings,InputBindings& input,ButtonInteraction& interaction,OptionsServices services,ViewRegistry& views,ControlRegistry& controls)
    :MenuView(nullptr,resources,settings,strings,views,controls),services_(std::move(services)),settings_(settings) {
    if(currentOptions) throw "Object already exist.";
    if(!services_.soundDisabled || !services_.soundVolume || !services_.musicVolume || !services_.setSoundVolume || !services_.setMusicVolume || !services_.resetGraphics) throw std::invalid_argument("Options requires sound and graphics services");
    currentOptions=this;
    for(auto& picture:pictures_) picture=new Picture(this,resources,views,controls);
    ok_=new TextAnimationButton(this,resources,settings,strings,input,interaction,views,controls);
    fullscreen_=new CheckBox(this,resources,input,interaction,views,controls);
    sound_=new Slider(this,resources,input,interaction,views,controls);music_=new Slider(this,resources,input,interaction,views,controls);
    for(auto& label:labels_) label=new TextLine(this,resources,settings,strings,views,controls);
    ok_->clickEvent=&okClicked;fullscreen_->changed=&fullscreenChanged;sound_->valueChanged=&soundChanged;music_->valueChanged=&musicChanged;
    const auto document=resources.loadSettingsXML(String("UI/Options.xml"));loadXML(*document);
}
Options::~Options() { currentOptions=nullptr; }
void Options::loadXML(const XmlNode& node) {
    MenuView::loadXML(node);
    const std::pair<const char*,View*> bindings[]={{"Back",pictures_[0]},{"Logo",pictures_[1]},{"Podlogka",pictures_[2]},
        {"OkButton",ok_},{"SoundVolume",sound_},{"MusicVolume",music_},{"FullScreen",fullscreen_},
        {"FullScreenText",labels_[0]},{"MusicText",labels_[1]},{"SoundText",labels_[2]},{"Label",labels_[3]}};
    for(const auto& binding:bindings) { XmlNode* entry=nullptr;if(node.section(String(binding.first),entry)) binding.second->loadXML(*entry); }
    bool full=fullscreen_->checked();settings_.get(String("graphics"),String("fullscreen"),full);fullscreen_->setChecked(full);
    if(services_.soundDisabled()) { music_->disable();sound_->disable(); }
    music_->setValue(float(originalUnsigned(services_.musicVolume())));music_->updateThumb();
    sound_->setValue(float(originalUnsigned(services_.soundVolume())));sound_->updateThumb();
}
void Options::okClicked(ButtonPrototype*) { instance()->returnToMenu(); }
void Options::returnToMenu() {
    if(!returnView_) throw std::logic_error("Options return view is not bound");
    returnView_->showTransparently();returnView_->enable();discardTransparently();
}
void Options::fullscreenChanged(CheckBox*,bool full) { auto* self=instance();self->settings_.set(String("graphics"),String("fullscreen"),full);self->services_.resetGraphics(true); }
void Options::soundChanged(Slider*,float value) { instance()->services_.setSoundVolume(volumeInteger(value)); }
void Options::musicChanged(Slider*,float value) { instance()->services_.setMusicVolume(volumeInteger(value)); }
}
