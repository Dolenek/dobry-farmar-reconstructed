#include "UiSound.h"
#include "SoundManager.h"
#include "Sound.h"
#include "ResourceManager.h"
namespace farmar::recovered {
void bindButtonSound(ButtonInteraction& interaction,ResourceManager& resources,SoundManager& sound) {
    interaction.loadSound=[&resources](const String& name) { return resources.getSound(name); };
    interaction.playClick=[&sound](cResource* resource) { sound.clickSound(static_cast<cSound*>(resource)); };
    interaction.playHover=[&sound](cResource* resource,bool loop,unsigned start) {
        // Native Button::hovered (004f3a7e) ignores the returned cChannel.
        // Destroying it here would immediately stop the hover sound. Preserve
        // that confirmed original retained wrapper/reference; record the leak.
        (void)sound.playSound(static_cast<cSound*>(resource),loop,start!=0);
    };
}
OptionsServices bindOptionsSound(SoundManager& sound,std::function<void(bool)> resetGraphics) {
    return {[&sound] { return sound.disabled(); },[&sound] { return sound.soundVolume(); },
        [&sound] { return sound.musicVolume(); },[&sound](unsigned value) { sound.setSoundVolume(value); },
        [&sound](unsigned value) { sound.setMusicVolume(value); },std::move(resetGraphics)};
}
}
