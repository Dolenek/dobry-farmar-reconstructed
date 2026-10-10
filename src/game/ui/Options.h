#pragma once
#include "MenuView.h"
#include "ButtonText.h"
#include "CheckBox.h"
#include "Slider.h"
namespace farmar::recovered {
// Required engine services. No default/no-op audio or reset implementation:
// an engine must supply its recovered SoundManager and graphics reset operations.
struct OptionsServices {
    std::function<bool()> soundDisabled;
    std::function<unsigned()> soundVolume,musicVolume;
    std::function<void(unsigned)> setSoundVolume,setMusicVolume;
    std::function<void(bool)> resetGraphics;
};
// cOptions: 108 native bytes, singleton interface + MenuView at +4.
class Options final : public MenuView {
public:
    Options(ResourceManager& resources,Settings& settings,StringManager& strings,InputBindings& input,
        ButtonInteraction& interaction,OptionsServices services,ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 00489770
    ~Options() override; // 00487a30
    void loadXML(const XmlNode& node) override; // 00487ae0
    void returnToMenu(); // 00488f10
    void setReturnView(View* view) noexcept { returnView_=view; } // 00487aa0
    static Options* instance() noexcept;
    Slider& soundSlider() noexcept { return *sound_; }
    Slider& musicSlider() noexcept { return *music_; }
    CheckBox& fullscreen() noexcept { return *fullscreen_; }
    TextAnimationButton& okButton() noexcept { return *ok_; }
    Picture& picture(unsigned index) { return *pictures_.at(index); }
    TextLine& label(unsigned index) { return *labels_.at(index); }
private:
    OptionsServices services_;Settings& settings_;
    Slider *sound_=nullptr,*music_=nullptr;
    std::array<Picture*,3> pictures_{};
    TextAnimationButton* ok_=nullptr;CheckBox* fullscreen_=nullptr;
    std::array<TextLine*,4> labels_{};View* returnView_=nullptr;
    static void okClicked(ButtonPrototype*);
    static void fullscreenChanged(CheckBox*,bool); // 004892f0
    static void soundChanged(Slider*,float); // 00488770
    static void musicChanged(Slider*,float); // 00488b40
};
}
