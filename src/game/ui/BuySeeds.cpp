#include "BuySeeds.h"
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
BuySeeds* singleton=nullptr;
void child(View& view,const XmlNode& xml,const char* key) {XmlNode* node=nullptr;if(xml.section(String(key),node)) view.loadXML(*node);}
bool enabled(const View& view) {return view.state().enabled && view.state().parentEnabled;}
bool active(ButtonState state) {return state==ButtonState::Hovered || state==ButtonState::Pressed;}
bool inRectangle(const Control& control,const Point2& p) { // 00417910, original float bounds spills
    const auto& g=control.geometry();const float x=float(originalSubtract(g.x,originalMultiply(g.width,0.5))),y=float(originalSubtract(g.y,originalMultiply(g.height,0.5)));
    const auto right=originalAdd(x,g.width);const float bottom=float(originalAdd(y,g.height));return x<=p[0] && p[0]<=right && y<=p[1] && p[1]<=bottom;
}
int signedBits(unsigned value) {int output;std::memcpy(&output,&value,4);return output;}
}
BuySeedsLifetime::BuySeedsLifetime(BuySeeds* value) {if(singleton) throw "Object already exist.";singleton=value;}
BuySeedsLifetime::~BuySeedsLifetime() {singleton=nullptr;}
BuySeeds* BuySeeds::instance() noexcept {return singleton;}
void SeedPurchaseDefinition::loadXML(const XmlNode& node,ResourceManager& resources) {name=node.attribute(WString(L"name"));XmlNode* state=nullptr;if(node.section(String("State"),state)) appearance.loadXML(*state,resources);}
BuySeeds::BuySeeds(View* parent,BuySeedsContext& context):BuySeedsLifetime(this),MovableUnion(parent,context.resources,context.views,context.controls),context_(context) {
    auto& c=context;cancel_=new TextAnimationButton(this,c.resources,c.settings,c.strings,c.input,c.interaction,c.views,c.controls);
    for(auto& cell:cells_) {cell.button=new SeedPurchaseButton(this,c.resources,c.input,c.interaction,c.views,c.controls);cell.price=new TextMoney(cell.button,c.resources,c.settings,c.strings,c.views,c.controls);cell.name=new MultilineText(this,c.resources,c.settings,c.strings,c.views,c.controls);}
    loadXML(*c.resources.loadSettingsXML(String("UI/gameUI/BuySeeds.xml")));cancel_->clickEvent=&cancelled;for(auto& cell:cells_) cell.button->clickEvent=&chosen;
}
BuySeeds::~BuySeeds() {if(buySound_) buySound_->releaseReference();context_.input.unbind(0x208,&outsidePressed,this);singleton=nullptr;}
void BuySeeds::loadXML(const XmlNode& node) {
    MovableUnion::loadXML(node);child(*cancel_,node,"CancelButton");XmlNode* entries=nullptr;
    if(node.section(String("ItemPlace"),entries)) for(unsigned i=0;i<std::min<std::size_t>(8,entries->children().size());++i) {XmlNode* item=nullptr;if(entries->section(i,item)) {if(i>=places.size()) throw std::runtime_error("Native eighth seed place overlaps its first definition; unsupported inconsistent XML");const auto text=item->attribute(WString(L"place"));if(!text.empty()) {std::wistringstream in(text.c_str());in>>places[i][0];in.ignore(1);in>>places[i][1];}}}
    if(node.section(String("ItemConfig"),entries)) for(unsigned i=1;i<std::min<std::size_t>(8,entries->children().size());++i) {XmlNode* item=nullptr;if(entries->section(i,item)) definitions[i].loadXML(*item,context_.resources);}
    const auto sound=toNarrow(node.attribute(WString(L"BuySound")));if(!sound.empty()) buySound_=static_cast<cSound*>(context_.resources.getSound(sound)); // native reload retains old reference
    XmlNode* proto=nullptr;if(!node.section(String("ItemProto"),proto)) throw std::runtime_error("BuySeeds ItemProto is required by original valid XML");
    for(unsigned i=0;i<cells_.size();++i) {auto& cell=cells_[i];child(*cell.button,*proto,"BuyButton");child(*cell.price,*proto,"PriceText");child(*cell.name,*proto,"Text");
        cell.button->setPosition({float(originalAdd(places[i][0],cell.button->geometry().x)),float(originalAdd(places[i][1],cell.button->geometry().y))});
        cell.name->setPosition({float(originalAdd(cell.name->geometry().x,places[i][0])),float(originalAdd(places[i][1],cell.name->geometry().y))});}
    setPosition(hiddenPlace);
}
void BuySeeds::enabledStep() {Picture::enabledStep();if(movement) return;auto& t=context_.world.tutorial();if(t.canShow(14)) {t.showDisabledAnchor(14,this);return;}if(t.canShow(15)) t.showRepeatedAction(15,cells_[0].button);}
void BuySeeds::open() {
    enable();unsigned count=0;for(unsigned seed=1;seed<8;++seed) if(context_.profile.uints["SeedAvail"+std::to_string(seed)]) {
        auto& cell=cells_[count++];const auto& definition=definitions[seed];cell.name->setString(definition.name,false);cell.name->show();cell.button->copyAppearance(definition.appearance);cell.button->deepProcessing=true;cell.button->show();
        const auto price=context_.configuration.seeds[seed].price;cell.price->setString(WString(std::to_wstring(price)));cell.button->seed=seed;cell.price->show();
        const bool unavailable=context_.world.money()<signedBits(price);if(unavailable) cell.button->disable();else cell.button->enable();cell.price->setFrame(unavailable ? 1:0);
    }
    for(unsigned i=count;i<7;++i) if(i) {cells_[i].name->hide();cells_[i].button->hide();cells_[i].price->hide();}
    context_.input.bind(0x208,&outsidePressed,this);move(2,{geometry().x,geometry().y},originalPosition);
}
void BuySeeds::close() {context_.input.unbind(0x208,&outsidePressed,this);disable();move(1,{geometry().x,geometry().y},hiddenPlace);}
void BuySeeds::cancel() {auto& h=context_.world.hud();h.enable();h.absoluteEnable();close();}
void BuySeeds::choose(SeedPurchaseButton& button) {const auto seed=button.seed;auto& h=context_.world.hud();h.enable();h.absoluteEnable();close();if(buySound_) context_.sound.clickSound(buySound_);context_.world.chooseSeed(seed);}
bool BuySeeds::cursorInside() {
    if(!enabled(*this)) return false;if(!context_.interaction.cursor) throw std::logic_error("BuySeeds cursor is not bound");const auto at=context_.interaction.cursor();
    if(inRectangle(*this,at)) return true;for(auto* picture:backgrounds()) if(inRectangle(*picture,at)) return true;if(active(cancel_->buttonState())) return true;
    for(unsigned i=1;i<7;++i) if(active(cells_[i].button->buttonState())) return true;
    // Original 0041e140 iterates one slot past its seven cells and interprets
    // the sound pointer as a Button, reading +238 beyond the 40-byte cSound.
    // Preserve the actual instruction-level read, rather than inventing a
    // substitute slot or hover result. This original undefined-memory branch
    // requires separate heap-state diagnosis and full-game verification.
    unsigned state;std::memcpy(&state,reinterpret_cast<const unsigned char*>(buySound_)+0x238,4);return state==2 || state==3;
}
void BuySeeds::cancelled(ButtonPrototype*) {instance()->cancel();}
void BuySeeds::chosen(ButtonPrototype* button) {instance()->choose(*static_cast<SeedPurchaseButton*>(button));}
void BuySeeds::outsidePressed(void*) {auto& self=*instance();if(enabled(self) && !self.context_.world.tutorial().active() && !self.cursorInside()) self.cancel();}
}
