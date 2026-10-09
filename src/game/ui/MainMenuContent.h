#pragma once
#include "ButtonText.h"
#include "MenuView.h"
namespace farmar::recovered {
// Owned control construction and XML binding portions of 00484da0/00481e10.
// MenuView owns all control pointers. This is not the main-menu controller:
// navigation callbacks, singleton/escape hooks and profile persistence are
// separate recovery work, not replaced by nonfunctional menu actions.
class MainMenuContent {
public:
    MainMenuContent(MenuView& owner,ResourceManager& resources,Settings& settings,StringManager& strings,
        InputBindings& input,ButtonInteraction& interaction,ViewRegistry& views,ControlRegistry& controls);
    void loadXML(const XmlNode& node,const std::function<void()>& ensureFixedBack,const WString* profileName);
    void updateProfileName(const WString* profileName); // 00481770
    TextAnimationButton *start=nullptr,*exit=nullptr,*changeUser=nullptr,*options=nullptr,*awards=nullptr,*credits=nullptr,*unused=nullptr;
    TextLine *userName=nullptr,*userText=nullptr;
private:
    MenuView& owner_;
};
}
