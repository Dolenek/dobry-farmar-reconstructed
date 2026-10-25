#include "Farm.h"
#include "GameMenu.h"
#include "BuySeeds.h"
#include "Warehouse.h"
#include "ElasticAnimation.h"
#include "TutorialManager.h"
#include "ResourceManager.h"
#include "SoundManager.h"
#include "Sound.h"
#include "XmlDocument.h"
#include "StringConversion.h"
#include "OriginalArithmetic.h"
#include "GameRandom.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
namespace {
Farm* currentFarm=nullptr;
template<class T> T& actual(T* value) {if(!value) throw std::logic_error("Farm requires actual constructed game object");return *value;}
template<class T> void attribute(const XmlNode& n,const wchar_t* key,T& v) {const auto text=n.attribute(WString(key));if(!text.empty()) {std::wistringstream in(text.c_str());in>>v;}}
void point(const XmlNode& n,const wchar_t* key,Point2& v) {const auto text=n.attribute(WString(key));if(!text.empty()) {std::wistringstream in(text.c_str());in>>v[0]>>v[1];}}
Point2 position(const Control& control) {return {control.geometry().x,control.geometry().y};}
Person& person() {return actual(Person::instance());}
}
FarmLifetime::FarmLifetime(Farm* value) {if(currentFarm) throw "Object already exist.";currentFarm=value;}
void FarmLifetime::releaseSingleton() {currentFarm=nullptr;}
FarmLifetime::~FarmLifetime() {releaseSingleton();}
Farm* Farm::instance() noexcept {return currentFarm;}
Farm::Farm(View* parent,FarmContext& c):StateView(parent,c.resources,c.settings,c.strings,c.views,c.controls),FarmLifetime(this),context_(c),
    fieldContext_{{c.resources,c.settings,c.strings,c.input,c.interaction,static_cast<FarmFieldWorld&>(*this),c.views,c.controls},c.profile,c.configuration,c.random,*this},
    seedContext_{{c.resources,c.settings,c.strings,c.input,c.interaction,static_cast<SeedPackageWorld&>(*this),c.views,c.controls},c.configuration,c.profile,*this},
    sprinkleContext_{{c.resources,c.settings,c.strings,c.input,c.interaction,static_cast<SprinkleWorld&>(*this),c.views,c.controls},c.profile,c.random,*this},
    letterContext_{c.resources,c.settings,c.strings,c.input,c.interaction,c.profile,c.sound,*this,c.views,c.controls} {
    const auto active=[this] {return context_.tips.active();};
    seeds_=new ExtendedAnimationButton(&origin(),c.resources,c.settings,c.strings,c.input,c.interaction,active,c.views,c.controls);
    fields_=new ExtendedTextAnimationButton(&movingPicture(),c.resources,c.settings,c.strings,c.input,c.interaction,active,c.views,c.controls);
    price_=new TextMoney(fields_,c.resources,c.settings,c.strings,c.views,c.controls);
    lighter_=new Picture(&movingPicture(),c.resources,c.views,c.controls);
    seedText_=new TextLine(&origin(),c.resources,c.settings,c.strings,c.views,c.controls);
    mailText_=new TextLine(&origin(),c.resources,c.settings,c.strings,c.views,c.controls);
    postBox_=new Picture(&origin(),c.resources,c.views,c.controls);
    for(unsigned i=0;i<3;++i) letters_[i]=new ContractLetterSlot(&origin(),i,letterContext_);
    loadXML(*c.resources.loadSettingsXML(String("UI/gameUI/farm.xml")));
    for(unsigned i=0;i<3;++i) {const auto saved=c.profile.ints["letter"+std::to_string(i)];if(saved!=-1) {letters_[i]->assign(c.game.savedContract(saved),0,true);++letterCount;}}
    for(unsigned i=0;i<3;++i) if(c.profile.booleans["SPExist"+std::to_string(i)]) {new Sprinkle(&movingPicture(),0,i,sprinkleContext_);++sprinklers;}
    hose_=new HoseEmitter(&movingPicture(),c.resources,c.random,String("Effects/HoseEffect.xml"),c.views,c.controls);
    hose_->start(0);hose_->setPosition({0,0});hose_->targetPoint={float(originalSubtract(250,hose_->geometry().x)),float(originalSubtract(250,hose_->geometry().y))};hose_->emission.generating=false;
    seeds_->clickEvent=&openSeeds;fields_->clickEvent=&purchasedFields;refreshUpgradeButton();refreshSeedButton();
}
Farm::~Farm() {
    if(letterSound_) letterSound_->releaseReference();unbindInput();graph.fields.clear();graph.areas.clear();releaseSingleton();
    // Fields and actors borrow the bindings above. Destroy the original View
    // ownership tree now, while Farm virtual services and contexts still live.
    // Native also clears its singleton before entering MenuView/View teardown.
    while(!children().empty()) delete *children().begin();
}
void Farm::loadXML(const XmlNode& n) {
    StateView::loadXML(n);XmlNode* child=nullptr;
    for(auto item:std::initializer_list<std::pair<const char*,View*>>{{"PostBox",postBox_},{"SeedsBuyButton",seeds_},{"FieldsBuyButton",fields_},{"FieldsBuyCostText",price_},{"FieldLighter",lighter_},{"SeedText",seedText_},{"MailText",mailText_}})
        if(n.section(String(item.first),child)) item.second->loadXML(*child);
    upgradePrices.fill(0);
    if(n.section(String("FieldsBuyPlaces"),child)) for(unsigned i=0;i<(std::min)(std::size_t(4),child->children().size());++i) {XmlNode* p=nullptr;if(child->section(i,p)) point(*p,L"Pos",buyPlaces[i]);}
    if(n.section(String("UpgradePrices"),child)) for(unsigned i=0;i<(std::min)(std::size_t(4),child->children().size());++i) {XmlNode* p=nullptr;if(child->section(i,p)) attribute(*p,L"Price",upgradePrices[i]);}
    if(n.section(String("FieldArray"),child)) for(unsigned i=0;i<child->children().size();++i) {XmlNode* p=nullptr;if(!child->section(i,p)) continue;Point2 at{};unsigned upgrade;point(*p,L"Center",at);attribute(*p,L"UpgradeIndex",upgrade);
        auto* plot=new FarmField(&movingPicture(),0,i,int(upgrade),fieldContext_);plot->setPosition(at);plot->show();plot->enable();graph.fields.push_back(plot);}
    if(n.section(String("LetterPlace"),child)) for(unsigned i=0;i<(std::min)(std::size_t(3),child->children().size());++i) {XmlNode* p=nullptr;if(child->section(i,p)) {Point2 at{};point(*p,L"Pos",at);letters_[i]->place=at;}}
    const auto sound=toNarrow(n.attribute(WString(L"LetterSound")));if(!sound.empty()) letterSound_=context_.resources.getSound(sound); // native retained replacement
}
TutorialManager& Farm::tutorial() {return context_.tips;}
int Farm::money() {return context_.game.money();}void Farm::setMoney(int v) {context_.game.setMoney(v);}
int Farm::money() const {return context_.game.money();}
int Farm::activeContracts() const {return int(actual(GameMenu::instance()).contractCount);}
void Farm::consumeLetter(ContractOfferLetter& letter) {removeLetter(dynamic_cast<ContractLetterSlot&>(letter));}
void Farm::activateContract(GameplayEvent& e) {actual(GameMenu::instance()).activateContract(e);}
bool Farm::tutorialCanShow(unsigned n) const {return context_.tips.canShow(n);}
void Farm::showTutorial(unsigned n,View& anchor) {context_.tips.showAction(n,&dynamic_cast<Control&>(anchor));}
void Farm::registerArea(ActionArea& a) {graph.registerArea(a);}
void Farm::queueAreaAction(ActionArea& a,const Point2& p) {person().tasks().add(a,p);}
int Farm::warehouseLevel() {return actual(Warehouse::instance()).level();}int Farm::digBuildingLevel() {return actual(DigBuilding::instance()).level();}
int Farm::mowBuildingLevel() {return actual(MowBuilding::instance()).level();}int Farm::waterSourceLevel() {return actual(WaterSource::instance()).level();}
bool Farm::hasDryField() {return graph.hasDryField();}
bool Farm::cursorOverAreaType(unsigned type,const ActionArea* excluded) {
    const auto p=context_.interaction.cursor();for(auto* a:graph.areas) if(a!=excluded && a->areaType==type) {
        const auto& g=a->geometry();const float x=float(originalSubtract(g.x,originalMultiply(g.width,.5))),y=float(originalSubtract(g.y,originalMultiply(g.height,.5)));
        //00417910 spills left/top/bottom, while right stays in x87.
        if(x<=p[0] && p[0]<=originalAdd(x,g.width) && y<=p[1] && p[1]<=float(originalAdd(y,g.height))) return true;
    }return false;
}
bool Farm::areaGate(unsigned type) {return cursorOverAreaType(type,nullptr);}
ActionArea* Farm::firstAreaAt(const Point2& p,unsigned mask) {return graph.firstAt(p,mask);}
void Farm::indicateField(FarmField& f) {indicated=&f;lighter_->setPosition(position(f));lighter_->show();}
void Farm::clearFieldIndication(FarmField& f) {if(indicated==&f) {indicated=nullptr;lighter_->hide();}}
std::vector<ActionArea*> Farm::seedPackages() {return graph.seedPackages();}
bool Farm::canPlaceSeed(SeedPackage& s,const Point2& p) {return graph.canPlaceSeed(&s,p);}
ActionArea* Farm::topAt(const Point2& p,unsigned mask) {return graph.topAt(p,mask);}
ActionArea* Farm::areaAt(const Point2& p,unsigned mask) {return graph.topAt(p,mask);}
std::vector<ActionArea*> Farm::areasAt(const Point2& p,unsigned mask) {std::vector<ActionArea*> out;graph.appendAt(p,mask,out);return out;}
void Farm::setFarmActionButtonEnabled(bool v) {if(v) seeds_->enable();else seeds_->disable();}
void Farm::removeSeed(SeedPackage& s) {graph.remove(s,context_.views);}
FarmField* Farm::fieldForSeedTutorial() {return graph.fieldForSeedTutorial();}
Control& Farm::digBuilding() {return actual(DigBuilding::instance());}
bool Farm::buildingAt(const Point2& p) {return graph.firstAt(p,1)!=nullptr;}
bool Farm::canPlaceSprinkle(Sprinkle& s,const Point2& p) {return graph.canPlaceSprinkle(&s,p);}
std::vector<ActionArea*> Farm::wateredFields(const Point2& p,float r) {return graph.wateredFields(p,r);}
void Farm::clearWaterPreview() {graph.clearWaterPreview();}
void Farm::previewWater(ActionArea& a,bool v) {dynamic_cast<FarmField&>(a).preview(v);}
double Farm::addWater(ActionArea& a,float v) {return dynamic_cast<FarmField&>(a).addWater(v);}
void Farm::removeSprinkle(Sprinkle& s) {graph.remove(s,context_.views);--sprinklers;}
Sprinkle* Farm::createSprinkle(unsigned cost) {return createFarmSprinkle(&movingPicture(),cost,sprinklers,sprinkleContext_);}
SeedPackage* Farm::createSeed(unsigned kind) {return createFarmSeed(kind,seedContext_);}
SeedPackage* Farm::restoreSeed(unsigned kind,unsigned count,const Point2& position,unsigned status,bool unpaid) {return new SeedPackage(&movingPicture(),kind,count,position,status,unpaid,seedContext_);}
void Farm::addLetter(GameplayEvent& e) {for(auto* slot:letters_) if(slot->letterState==2) {slot->assign(e,0,false);++letterCount;if(letterSound_) context_.sound.clickSound(letterSound_);return;}}
void Farm::removeLetter(ContractLetterSlot& slot) {for(auto* s:letters_) if(s==&slot) {--letterCount;slot.letterState=2;context_.profile.ints["letter"+std::to_string(slot.id)]=-1;slot.hideTips();return;}}
int Farm::heldType() {return int(context_.profile.uints["PCarry"]);}void Farm::setHeldType(int v) {context_.profile.uints["PCarry"]=unsigned(v);}
bool Farm::personPresent() {return Person::instance()!=nullptr;}
unsigned Farm::heldProductCount() {unsigned total=0;for(unsigned i=0;i<8;++i) total+=context_.profile.uints["PCC"+std::to_string(i)];return total;}
void Farm::hidePersonHeldIndicator() {person().visual().waterUse().hide();}
void Farm::updatePersonHeldIndicator(bool on,unsigned count,unsigned capacity) {person().visual().showDiscrete(on,count,capacity);}
Point2 Farm::characterPosition() {return position(person());}
void Farm::updatePersonAppearance() {person().visual().playIdle();}
void Farm::refreshHeldAppearance() {person().refreshHeldAppearance();}
void Farm::cancelPersonAction(const String&) {person().requestCancel(true);} // all recovered callers pass Person.h157
void Farm::animateHeldDigUpgrade(const Point2& from,const Point2& to,int kind,bool) {actual(ElasticAnimation::instance()).begin(from,to,kind,nullptr);}
void Farm::animateHeldUpgrade(const Point2& from,const Point2& to,int kind,bool) {actual(ElasticAnimation::instance()).begin(from,to,kind,nullptr);}
void Farm::configureHose(const Point2& from,const Point2& to,float angle) {hose_->angle=angle;hose_->start(0);hose_->setPosition(from);const auto p=position(*hose_);hose_->startPoint={float(originalSubtract(from[0],p[0])),float(originalSubtract(from[1],p[1]))};hose_->targetPoint={float(originalSubtract(to[0],p[0])),float(originalSubtract(to[1],p[1]))};hose_->emission.generating=true;}
void Farm::stopHose() {hose_->emission.generating=false;}
void Farm::refreshSeedButton() {for(unsigned i=1;i<8;++i) if(context_.profile.uints["SeedAvail"+std::to_string(i)]) {seeds_->enable();return;}seeds_->disable();}
void Farm::refreshUpgradeButton() {
    const int current=context_.profile.ints["fcu"],available=context_.profile.ints["fua"];if(current==available) {fields_->hideTips();return;}
    const auto at=position(movingPicture());fields_->show();const auto& place=buyPlaces.at(unsigned(current+1));fields_->setPosition({float(originalAdd(at[0],place[0])),float(originalAdd(at[1],place[1]))});
    const auto have=money();const auto cost=int(upgradePrices.at(unsigned(current+1)));price_->setString(WString(std::to_wstring(upgradePrices.at(unsigned(current+1)))));if(have<cost) fields_->disable();else fields_->enable();
}
void Farm::setAvailableUpgrade(int value) {const int current=context_.profile.ints["fcu"],previous=context_.profile.ints["fua"];if(int(unsigned(previous)+1)!=value) throw "upgrade_avail+1!=_upgrade";context_.profile.ints["fua"]=value;for(auto* f:graph.fields) f->refreshPurchase(current,value);refreshUpgradeButton();}
void Farm::setCurrentUpgrade(int value) {context_.profile.ints["fcu"]=value;const int available=context_.profile.ints["fua"];for(auto* f:graph.fields) f->refreshPurchase(value,available);refreshUpgradeButton();for(auto* area:graph.areas) if(area->areaType==64) {auto& sprinkle=dynamic_cast<Sprinkle&>(*area);if(sprinkle.status()==0) sprinkle.setStatus(0);}}
void Farm::buyFields() {const int have=money(),current=context_.profile.ints["fcu"];const int cost=int(upgradeCost(unsigned(current+1)));if(cost<=have) {setMoney(int(unsigned(have)-unsigned(cost)));setCurrentUpgrade(int(unsigned(current)+1));context_.game.playUpgradeSound();}}
void Farm::absoluteStep() {
    StateView::absoluteStep();auto& p=context_.profile;if(p.booleans["awNoDry"]) {const float before=p.floats["awNoDryTime"],dt=uiElapsedTime()();p.floats["awNoDryTime"]=float(originalAdd(dt,before));}
    const bool visible=state().shown && state().parentShown;
    if(seeds_->state().enabled && seeds_->state().parentEnabled && movement==0 && visible && tutorial().canShow(13)) {
        unsigned minimum=100000;for(unsigned i=1;i<8;++i) if(p.uints["SeedAvail"+std::to_string(i)]) {const unsigned price=context_.configuration.seeds[i].price;if(price<=minimum) minimum=price;}
        if(minimum==100000) minimum=0;const int have=money();setMoney(int(unsigned(have)+minimum));tutorial().showAction(13,seeds_);
    }
    if(fields_->state().shown && fields_->state().parentShown && fields_->state().enabled && fields_->state().parentEnabled && movement==0 && visible && tutorial().canShow(19)) tutorial().showArrow(19,fields_);
}
void Farm::onShown() {context_.input.bind(0x209,&rightClick,this);context_.input.bind(0x208,&leftPress,this);context_.input.bind(0x200,&leftRelease,this);View::onShown();refreshSeedButton();}
void Farm::unbindInput() {context_.input.unbind(0x209,&rightClick,this);context_.input.unbind(0x208,&leftPress,this);context_.input.unbind(0x200,&leftRelease,this);}
void Farm::onHidden() {unbindInput();View::onHidden();}
void Farm::onDisabled() {View::onDisabled();lighter_->hide();clickHeld=false;}
void Farm::openSeeds(ButtonPrototype*) {auto& f=actual(instance());if(!f.tutorial().active() || f.tutorial().activeStep(13)) {auto& hud=actual(GameMenu::instance());hud.disable();hud.absoluteDisable();actual(BuySeeds::instance()).open();}}
void Farm::purchasedFields(ButtonPrototype*) {actual(instance()).buyFields();}
void Farm::rightClick(void*) {person().tasks().clear();person().requestCancel(true);}
void Farm::leftPress(void*) {actual(instance()).inputHeld=true;}
void Farm::leftRelease(void*) {auto& f=actual(instance());f.inputHeld=false;f.clickHeld=false;}
void Farm::destroyFields(int count,float probability) { //0042f080
    if(count==0 || !(originalMultiply(context_.random.next(),3.0518509447574615e-05f)<probability)) return;
    int eligible=0;for(auto* f:graph.fields) if(f->culture()!=0) {
        if(f->culture()!=0) {if(!f->harvested) ++eligible;}else {++eligible;f->harvested=false;}
    }
    if(eligible<count) count=eligible;
    while(count!=0) {
        // Native decrements the upper bound BEFORE multiplication. With N
        // eligible fields and N>1, the last one cannot be selected this round.
        // Probability uses1/32767 above; selection uses1/32768 here.
        const int upper=int(unsigned(eligible)-1);
        int index=int(originalMultiply(originalMultiply(context_.random.next(),upper),3.0517578125e-05f));
        for(auto* f:graph.fields) if(f->culture()!=0) {
            if(f->culture()==0) f->harvested=false;else if(f->harvested) continue;
            if(index!=0) {index=int(unsigned(index)-1);continue;}f->harvested=true;break;
        }
        count=int(unsigned(count)-1);eligible=int(unsigned(eligible)-1);
    }
}
}
