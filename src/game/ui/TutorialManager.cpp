#include "TutorialManager.h"
#include "GameMenu.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
namespace {
TutorialManager* singleton=nullptr;
bool visible(const View& value) {return value.state().shown && value.state().parentShown;}
bool enabled(const View& value) {return value.state().enabled && value.state().parentEnabled;}
void setEnabled(View& value,bool state) {if(state) value.enable();else value.disable();}
template<unsigned Step> bool predicate(Control* anchor) {return TutorialManager::instance()->evaluate(Step,anchor);}
template<class T> void attribute(const XmlNode& node,const wchar_t* key,T& value) {const auto text=node.attribute(WString(key));if(!text.empty()) {std::wistringstream in(text.c_str());in>>value;}}
void pointAttribute(const XmlNode& node,const wchar_t* key,Point2& value) {const auto text=node.attribute(WString(key));if(!text.empty()) {std::wistringstream in(text.c_str());in>>value[0];in.ignore(1);in>>value[1];}}
Control& first(const std::vector<Control*>& values) {if(values.empty() || !values.front()) throw std::logic_error("Tutorial predicate requires a nonempty actual world list");return *values.front();}
Control& required(Control* value) {if(!value) throw std::logic_error("Tutorial predicate requires an actual anchor");return *value;}
}
TutorialManagerLifetime::TutorialManagerLifetime(TutorialManager* value) {if(singleton) throw "Object already exist.";singleton=value;}
TutorialManagerLifetime::~TutorialManagerLifetime() {singleton=nullptr;}
TutorialManager* TutorialManager::instance() noexcept {return singleton;}
std::string TutorialManager::profileKey(unsigned step) {return "tip"+std::to_string(step);}
void TutorialStep::loadXML(unsigned value,const XmlNode& node) {
    index=int(value);XmlNode* section=nullptr;if(node.section(String("Text"),section)) text=section->text();
    pointAttribute(node,L"Offset",offset);pointAttribute(node,L"ArrowOffset",arrowOffset);
    attribute(node,L"TextMult",multiplier);attribute(node,L"TextInterlinear",interlinear);
}
TutorialManager::TutorialManager(TutorialManagerContext& context):TutorialManagerLifetime(this),View(nullptr,context.views),context_(context),
    tipContext_{context.resources,context.settings,context.strings,context.input,context.interaction,*this,context.views,context.controls} {
    auto& r=context.resources;
    common_=new TutorialCommonTip(nullptr,tipContext_);common_->loadXML(*r.loadSettingsXML(String("Tutorial/CommonHelpTip.xml")));
    arrow_=new TutorialArrowTip(nullptr,tipContext_);arrow_->loadXML(*r.loadSettingsXML(String("Tutorial/ArrowHelpTip.xml")));
    action_=new TutorialActionTip(nullptr,tipContext_);action_->loadXML(*r.loadSettingsXML(String("Tutorial/ActionTip.xml")));
    disabled_=new TutorialDisabledAnchorTip(nullptr,tipContext_);disabled_->loadXML(*r.loadSettingsXML(String("Tutorial/ArrowHelpTip.xml")));
    repeated_=new TutorialRepeatedActionTip(nullptr,tipContext_);repeated_->loadXML(*r.loadSettingsXML(String("Tutorial/ActionTip.xml")));
    loadXML(*r.loadSettingsXML(String("Tutorial/Tutorial.xml")));
    steps[1].predicate=&predicate<1>;steps[2].predicate=&predicate<2>;steps[3].predicate=&predicate<3>;steps[4].predicate=&predicate<4>;
    steps[5].predicate=&predicate<5>;steps[6].predicate=&predicate<6>;steps[7].predicate=&predicate<7>;
    steps[15].predicate=&predicate<15>;steps[16].predicate=&predicate<16>;steps[13].predicate=&predicate<13>;
    steps[17].predicate=&predicate<17>;steps[18].predicate=&predicate<18>;steps[20].predicate=&predicate<20>;steps[21].predicate=&predicate<21>;
    steps[22].predicate=&predicate<22>;steps[23].predicate=&predicate<23>;steps[30].predicate=&predicate<30>;steps[32].predicate=&predicate<32>;
    steps[33].predicate=&predicate<33>;steps[34].predicate=&predicate<34>;steps[36].predicate=&predicate<36>;steps[37].predicate=&predicate<37>;
    steps[39].predicate=&predicate<39>;steps[40].predicate=&predicate<40>;steps[41].predicate=&predicate<41>;steps[42].predicate=&predicate<42>;
    steps[44].predicate=&predicate<44>;steps[45].predicate=&predicate<45>;steps[48].predicate=&predicate<48>;
}
TutorialManager::~TutorialManager() {
    common_->disable();arrow_->disable();disabled_->disable();action_->disable();
    // Native 004d0490 queues only these four objects. The repeated-action tip
    // is neither disabled nor discarded and remains in the global View set.
    TutorialCommonTip* tips[]={common_,arrow_,disabled_,action_};
    for(unsigned i=0;i<4;++i) context_.views.requestDiscard(tips[i],String(".\\Game\\Tutorial\\TutorialManager.cpp"+std::to_string(122+i)));
    singleton=nullptr;
}
void TutorialManager::loadXML(const XmlNode& node) {
    View::loadXML(node);attribute(node,L"TutorialEnable",tutorialEnabled);XmlNode* entries=nullptr;
    if(node.section(String("Tips"),entries)) for(unsigned i=0;i<std::min<std::size_t>(entries->children().size(),steps.size());++i) {XmlNode* item=nullptr;if(entries->section(i,item)) steps[i].loadXML(i,*item);}
}
bool TutorialManager::isProcessed(unsigned step) {return tutorialEnabled && context_.profile.booleans[profileKey(step)];}
bool TutorialManager::canShow(unsigned step) {return tutorialEnabled && currentStep==-1 && !context_.profile.booleans[profileKey(step)];}
void TutorialManager::complete(unsigned step) {
    context_.profile.booleans[profileKey(step)]=true;justProcessed=int(step);context_.profile.ints["tutJustProc"]=int(step);
    if(step==2) context_.profile.booleans["TFastSpeed"]=true;if(step==4) context_.profile.booleans["TFastSpeed"]=false;
}
void TutorialManager::captureHUD() {auto& h=context_.world.hud();resumeHUD=enabled(h);resumeHUDabsolute=h.state().absoluteEnabled && h.state().parentAbsoluteEnabled;}
void TutorialManager::configure(TutorialCommonTip& tip,const TutorialStep& step) {tip.setTextMultiplier(step.multiplier);tip.setTextInterlinear(step.interlinear);}
void TutorialManager::showCommon(unsigned index) {
    auto& w=context_.world;captureHUD();resumeSelection=enabled(w.selectionOverlay());resumeOverlay=enabled(w.sceneOverlay());const auto& s=steps.at(index);
    configure(*common_,s);common_->start(s.offset,s.text);w.hud().absoluteDisable();w.hud().disable();w.selectionOverlay().disable();w.sceneOverlay().disable();currentStep=int(index);
}
void TutorialManager::showArrow(unsigned index,Control* anchor) {auto& w=context_.world;captureHUD();const auto& s=steps.at(index);configure(*arrow_,s);arrow_->start(s.offset,s.arrowOffset,s.text,anchor);w.hud().absoluteDisable();w.hud().disable();currentStep=int(index);}
void TutorialManager::showDisabledAnchor(unsigned index,Control* anchor) {auto& w=context_.world;captureHUD();const auto& s=steps.at(index);configure(*disabled_,s);disabled_->start(s.offset,s.arrowOffset,s.text,anchor);w.hud().absoluteDisable();w.hud().disable();currentStep=int(index);}
void TutorialManager::showAction(unsigned index,Control* anchor) {const auto& s=steps.at(index);configure(*action_,s);action_->start(s.offset,s.arrowOffset,s.text,anchor,s.predicate);currentStep=int(index);}
void TutorialManager::showRepeatedAction(unsigned index,Control* anchor) {const auto& s=steps.at(index);configure(*repeated_,s);repeated_->start(s.offset,s.arrowOffset,s.text,anchor,s.predicate);currentStep=int(index);}
void TutorialManager::restoreHUD() {auto& h=context_.world.hud();setEnabled(h,resumeHUD);if(resumeHUDabsolute) h.absoluteEnable();else h.absoluteDisable();}
void TutorialManager::finishTip(TutorialCommonTip& tip) {
    if(&tip!=common_ && &tip!=arrow_ && &tip!=disabled_ && &tip!=action_ && &tip!=repeated_) throw "cTutorialManager::hideTips";
    complete(unsigned(currentStep));currentStep=-1;
    if(&tip==common_) {setEnabled(context_.world.selectionOverlay(),resumeSelection);setEnabled(context_.world.sceneOverlay(),resumeOverlay);restoreHUD();}
    if(&tip==arrow_) restoreHUD();if(&tip==disabled_) restoreHUD();
}
bool TutorialManager::takeCancelRequest() {const bool result=cancelRequested;cancelRequested=false;return result;}
void TutorialManager::setGameplayPaused(bool value) {context_.world.hud().gameplayPaused=value;}
bool TutorialManager::evaluate(unsigned index,Control* anchor) {
    auto& w=context_.world;
    switch(index) {
    case 1:if(w.wellUpgradeLevel()!=0 || w.characterHeldType()!=1) return false;{const float water=w.characterHeldWater();return water==w.wellCapacity();}
    case 2:if(w.characterActivity()==10) return true;{const float before=w.plotWaterLimit(required(anchor));if(before==w.plotWater(required(anchor))) return true;}
        for(auto* plot:w.farmPlots()) {const float before=w.plotWaterLimit(*plot);if(before==w.plotWater(*plot)) return true;}return false;
    case 3:return w.mowerUpgradeLevel()==0 && w.characterHeldType()==6;
    case 4:return w.characterActivity()>=4 && w.characterActivity()<=7;
    case 5:return w.warehouseProductCount(0)!=0;
    case 6:case 7:return w.manufacture().movement!=0 && w.farm().movement!=0;
    case 13:return w.selectionPresent();case 15:return w.seedMoveActive();
    case 16:return w.plantState(first(w.farmPlants()))==0;case 17:return w.anchorState(required(anchor))==1;
    case 18:return visible(w.contractConfirm()) || visible(w.contractAim());
    case 20:return w.characterHeldType()==2;case 21:return w.characterActivity()<=3 && w.characterActivity()>=0;
    case 22:return w.characterHeldType()==10;case 23:return w.plotKind(required(anchor))==0 || w.characterActivity()==8;
    case 30:return w.wareDetails().movement!=0;case 32:return w.sceneSelectionPresent();case 33:return w.wareItems().empty();
    case 34:return w.wareItemUpgrade(first(w.wareItems()))!=0;case 36:return w.wareSelectionPresent();case 37:return !(w.wareItemFlags(first(w.wareItems()))&1);
    case 39:return visible(w.contractDialog764());case 40:return w.characterHeldType()!=6 || w.characterToolFlag();
    case 41:return w.characterHeldType()!=6 || w.warehouseFree()!=0 || w.farm().movement!=0;
    case 42:return !visible(w.contractProvide());
    case 44:if(w.characterHeldType()!=6) return true;w.characterActivity();if(w.characterActivity()<=7 && w.characterActivity()>=4) return true;return w.farm().movement!=0;
    case 45:return w.manufactureProductCount(0)!=0;case 48:return w.parentProductCount(required(anchor))!=0;
    default:throw std::logic_error("No original tutorial predicate installed for this step");
    }
}
void TutorialManager::enabledStep() {
    if(!tutorialEnabled) return;if(!uiElapsedTime()) throw std::logic_error("UI timer is not bound");elapsed=float(originalAdd(uiElapsedTime()(),elapsed));auto& w=context_.world;
    if(canShow(1) && w.farm().movement==0 && visible(w.farm()) && context_.profile.floats["GameTime"]>6) {
        const auto plots=w.farmPlots();bool progressed=false;for(auto* plot:plots) if(w.plotWater(*plot)!=0) {progressed=true;break;}
        if(progressed || w.characterHeldType()==1) complete(1);else showAction(1,&w.well());
    }
    if(canShow(2) && w.farm().movement==0 && visible(w.farm()) && !canShow(1)) {
        const auto plots=w.farmPlots();bool progressed=false;for(auto* plot:plots) if(w.plotWater(*plot)!=0) {progressed=true;break;}
        if(progressed) complete(2);else showAction(2,&first(plots));
    }
    if(justProcessed==4 && canShow(5) && w.farm().movement==0 && visible(w.farm())) showAction(5,&w.warehouse());
    if(context_.profile.booleans["TProdCompl"] && w.farm().movement==0 && w.wareDetails().movement==0 && w.manufacture().movement==0 && canShow(43)) showCommon(43);
}
}
