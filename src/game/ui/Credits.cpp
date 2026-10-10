#include "Credits.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
namespace farmar::recovered {
namespace { Credits* currentCredits=nullptr; }
Credits* Credits::instance() noexcept { return currentCredits; }
Credits::Credits(View& mainMenu,ResourceManager& resources,Settings& settings,StringManager& strings,
    InputBindings& input,ButtonInteraction& interaction,ViewRegistry& views,ControlRegistry& controls)
    :MenuView(nullptr,resources,settings,strings,views,controls),mainMenu_(mainMenu),resources_(resources),settings_(settings),strings_(strings),controls_(controls) {
    if(currentCredits) throw "Object already exist.";currentCredits=this;
    return_=new TextAnimationButton(this,resources,settings,strings,input,interaction,views,controls);
    const auto document=resources.loadSettingsXML(String("UI/Credits.xml"));loadXML(*document);return_->clickEvent=&returnClick;
}
Credits::~Credits() { currentCredits=nullptr; }
void Credits::loadXML(const XmlNode& node) {
    MenuView::loadXML(node);XmlNode* entry=nullptr;
    if(node.section(String("ReturnButton"),entry)) return_->loadXML(*entry);
    if(node.section(String("Texts"),entry)) for(unsigned i=0;i<entry->children().size();++i) {
        XmlNode* textNode=nullptr;if(!entry->section(i,textNode)) continue;
        auto* text=new TextLine(this,resources_,settings_,strings_,registry_,controls_);text->loadXML(*textNode);addDecoration(text);
    }
}
void Credits::returnClick(ButtonPrototype*) { instance()->returnToMenu(); }
void Credits::returnToMenu() { discardTransparently();mainMenu_.showTransparently();mainMenu_.enable(); }
}
