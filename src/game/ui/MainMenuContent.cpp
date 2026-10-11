#include "MainMenuContent.h"
#include "XmlDocument.h"
namespace farmar::recovered {
MainMenuContent::MainMenuContent(MenuView& owner,ResourceManager& resources,Settings& settings,StringManager& strings,
    InputBindings& input,ButtonInteraction& interaction,ViewRegistry& views,ControlRegistry& controls):owner_(owner) {
    const auto button=[&] { return new TextAnimationButton(&owner,resources,settings,strings,input,interaction,views,controls); };
    // Exact native allocation order: four buttons, two text lines, three more
    // buttons. The final button is owned but never configured by 00481e10.
    start=button();exit=button();changeUser=button();options=button();
    userName=new TextLine(&owner,resources,settings,strings,views,controls);
    userText=new TextLine(&owner,resources,settings,strings,views,controls);
    awards=button();credits=button();unused=button();
}
void MainMenuContent::loadXML(const XmlNode& node,const std::function<void()>& ensureFixedBack,const WString* profileName) {
    owner_.MenuView::loadXML(node);ensureFixedBack();
    const std::pair<const char*,TextAnimationButton*> buttons[]={{"StartButton",start},{"Options",options},{"ExitButton",exit},
        {"ChangeUserButton",changeUser},{"AwardsButton",credits},{"CreditsButton",awards}};
    // The native XML loader addresses the embedded View, four bytes past
    // the singleton. Its Awards/Credits bindings are opposite to the action
    // bindings installed by 00484da0. Preserve that observable ordering.
    for(const auto& binding:buttons) {
        XmlNode* entry=nullptr;if(binding.second && node.section(String(binding.first),entry)) binding.second->loadXML(*entry);
    }
    XmlNode* entry=nullptr;
    if(node.section(String("UserName"),entry)) { userName->loadXML(*entry);if(profileName) userName->setString(*profileName); }
    if(node.section(String("UserText"),entry)) userText->loadXML(*entry);
}
void MainMenuContent::updateProfileName(const WString* profileName) {
    if(profileName && userName->string().empty()) userName->setString(*profileName);
}
}
