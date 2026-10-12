#include "BuyBuilding.h"
#include "GameMenu.h"
#include "TutorialManager.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include "SoundManager.h"
#include "Sound.h"
#include "StringConversion.h"
#include <algorithm>
#include <cstring>
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
namespace {
BuyBuilding* singleton=nullptr;
void child(View& view,const XmlNode& xml,const char* key) {XmlNode* node=nullptr;if(xml.section(String(key),node)) view.loadXML(*node);}
bool enabled(const View& view) {return view.state().enabled && view.state().parentEnabled;}
bool inRectangle(const Control& control,const Point2& p) {
    const auto& g=control.geometry();const float x=float(originalSubtract(g.x,originalMultiply(g.width,0.5))),y=float(originalSubtract(g.y,originalMultiply(g.height,0.5)));
    const auto right=originalAdd(x,g.width);const float bottom=float(originalAdd(y,g.height));return x<=p[0] && p[0]<=right && y<=p[1] && p[1]<=bottom;
}
int signedBits(unsigned value) {int output;std::memcpy(&output,&value,4);return output;}
}
BuyBuildingLifetime::BuyBuildingLifetime(BuyBuilding* value) {if(singleton) throw "Object already exist.";singleton=value;}
BuyBuildingLifetime::~BuyBuildingLifetime() {singleton=nullptr;}
BuyBuilding* BuyBuilding::instance() noexcept {return singleton;}
void BuildingPurchaseDefinition::loadXML(const XmlNode& node,ResourceManager& resources) {name=node.attribute(WString(L"name"));XmlNode* state=nullptr;if(node.section(String("State"),state)) appearance.loadXML(*state,resources);}
BuyBuilding::BuyBuilding(View* parent,BuyBuildingContext& c):BuyBuildingLifetime(this),MovableUnion(parent,c.resources,c.views,c.controls),context_(c) {
    for(unsigned i=0;i<cells_.size();++i) {auto& cell=cells_[i];cell.button=new BuildingPurchaseButton(this,c.resources,c.input,c.interaction,c.views,c.controls);cell.button->building=i;
        cell.mask=new ExtendedButton(this,c.resources,c.settings,c.strings,c.input,c.interaction,[this]{return context_.world.tutorial().active();},c.views,c.controls);
        cell.name=new TextLine(this,c.resources,c.settings,c.strings,c.views,c.controls);cell.price=new TextMoney(cell.button,c.resources,c.settings,c.strings,c.views,c.controls);}
    loadXML(*c.resources.loadSettingsXML(String("UI/gameUI/BuyBuilding.xml")));for(auto& cell:cells_) cell.button->clickEvent=&chosen;
}
BuyBuilding::~BuyBuilding() {if(buySound_) buySound_->releaseReference();context_.input.unbind(0x208,&outsidePressed,this);singleton=nullptr;}
void BuyBuilding::loadXML(const XmlNode& node) {
    MovableUnion::loadXML(node);XmlNode* entries=nullptr;
    if(node.section(String("ItemPlace"),entries)) for(unsigned i=0;i<std::min<std::size_t>(11,entries->children().size());++i) {XmlNode* item=nullptr;if(entries->section(i,item)) {const auto text=item->attribute(WString(L"place"));if(!text.empty()) {std::wistringstream in(text.c_str());in>>places[i][0];in.ignore(1);in>>places[i][1];}}}
    if(node.section(String("ItemConfig"),entries)) for(unsigned i=0;i<std::min<std::size_t>(11,entries->children().size());++i) {XmlNode* item=nullptr;if(entries->section(i,item)) definitions[i].loadXML(*item,context_.resources);}
    const auto sound=toNarrow(node.attribute(WString(L"BuySound")));if(!sound.empty()) buySound_=static_cast<cSound*>(context_.resources.getSound(sound)); // native reload overwrites without releasing old reference
    XmlNode* proto=nullptr;if(!node.section(String("ItemProto"),proto)) throw std::runtime_error("BuyBuilding ItemProto is required by original valid XML");
    for(unsigned i=0;i<cells_.size();++i) {auto& cell=cells_[i];child(*cell.button,*proto,"BuyButton");child(*cell.mask,*proto,"BuyMask");child(*cell.price,*proto,"PriceText");child(*cell.name,*proto,"Text");
        cell.button->setPosition({float(originalAdd(places[i][0],cell.button->geometry().x)),float(originalAdd(places[i][1],cell.button->geometry().y))});
        cell.mask->setPosition({float(originalAdd(cell.mask->geometry().x,places[i][0])),float(originalAdd(places[i][1],cell.mask->geometry().y))});
        cell.name->setPosition({float(originalAdd(places[i][0],cell.name->geometry().x)),float(originalAdd(places[i][1],cell.name->geometry().y))});}
    setPosition(hiddenPlace);
}
void BuyBuilding::enabledStep() {Picture::enabledStep();if(movement) return;auto& t=context_.world.tutorial();if(t.canShow(35)) {t.showDisabledAnchor(35,this);return;}if(t.canShow(36)) t.showRepeatedAction(36,cells_[0].button);}
void BuyBuilding::open() {
    enable();unsigned count=0;auto& t=context_.world.tutorial();
    for(unsigned building=0;building<11;++building) if(context_.profile.uints["UpAvail"+std::to_string(building)]!=0xffffffffU) {
        auto& cell=cells_[count++];const auto& definition=definitions[building];cell.name->setString(definition.name);cell.name->show();cell.button->copyAppearance(definition.appearance);cell.button->markTransformDirty();cell.button->show();
        int price=signedBits(context_.configuration.upgradeCost(building,0));const bool tutorialUnfinished=!t.isProcessed(35) || !t.isProcessed(36);if(t.tutorialEnabled && tutorialUnfinished) price=0;
        context_.world.money(); // actual discarded Money query before formatting
        cell.price->setString(WString(std::to_wstring(static_cast<unsigned>(price))));cell.button->building=building;cell.price->show();cell.mask->show();
        if(context_.world.money()<price) {cell.button->disable();cell.price->disable();}else {cell.button->enable();cell.price->enable();}
        for(unsigned j=0;j<2;++j) {auto* picture=dynamic_cast<Picture*>(cell.mask->tips().at(j+1));if(!picture) throw std::runtime_error("Original BuyBuilding tooltip requires two product Pictures");picture->copyAppearance(context_.configuration.products[tooltipProducts[building][j]].icon);}
    }
    for(unsigned i=count;i<11;++i) {auto& cell=cells_[i];cell.name->hide();cell.button->hide();cell.price->hide();cell.mask->hideTips();}
    context_.input.bind(0x208,&outsidePressed,this);move(2,{geometry().x,geometry().y},originalPosition);
}
void BuyBuilding::close() {context_.input.unbind(0x208,&outsidePressed,this);disable();move(1,{geometry().x,geometry().y},hiddenPlace);}
void BuyBuilding::cancel() {auto& h=context_.world.hud();h.enable();h.absoluteEnable();close();}
void BuyBuilding::choose(BuildingPurchaseButton& button) {auto& h=context_.world.hud();h.enable();h.absoluteEnable();context_.world.chooseBuilding(button.building,-1);close();if(buySound_) context_.sound.clickSound(buySound_);}
bool BuyBuilding::cursorInside() {if(!enabled(*this)) return false;if(!context_.interaction.cursor) throw std::logic_error("BuyBuilding cursor is not bound");const auto at=context_.interaction.cursor();if(inRectangle(*this,at)) return true;for(auto* picture:backgrounds()) if(inRectangle(*picture,at)) return true;return false;}
void BuyBuilding::chosen(ButtonPrototype* button) {instance()->choose(*static_cast<BuildingPurchaseButton*>(button));}
void BuyBuilding::outsidePressed(void*) {auto& self=*instance();if(enabled(self) && !self.context_.world.tutorial().active() && !self.cursorInside()) self.cancel();}
}
