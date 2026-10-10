#pragma once
#include "Options.h"
namespace farmar::recovered {
class SoundManager;
void bindButtonSound(ButtonInteraction& interaction,ResourceManager& resources,SoundManager& sound);
OptionsServices bindOptionsSound(SoundManager& sound,std::function<void(bool)> resetGraphics);
}
