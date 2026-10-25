#include "GameAwardManager.h"
#include "ContractProvide.h"
#include "ContractDialogs.h"
#include "Warehouse.h"
#include "WaterSource.h"
#include "DigBuilding.h"
#include "MowBuilding.h"
#include "Manufacture.h"
#include "ProductionBuilding.h"
#include "Market.h"
#include "ProfileProperties.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include <stdexcept>
#include <string>
namespace farmar::recovered {
namespace {
GameAwardManager* currentAwardManager=nullptr;
bool shown(const View* view) {return view && view->state().shown && view->state().parentShown;}
}
GameAwardManagerLifetime::GameAwardManagerLifetime(GameAwardManager* value) {if(currentAwardManager) throw "Object already exist.";currentAwardManager=value;}
GameAwardManagerLifetime::~GameAwardManagerLifetime() {currentAwardManager=nullptr;}
GameAwardManager* GameAwardManager::instance() noexcept {return currentAwardManager;}
GameAwardManager::GameAwardManager(GameAwardContext& c):View(nullptr,c.views),GameAwardManagerLifetime(this),context_(c) {
    for(unsigned i=0;i<21;++i) attained[i]=c.profile.booleans[std::string(Awards::conditionKey(i).view())];
    loadXML(*c.resources.loadSettingsXML(String("UI/Awards.xml")));
    for(unsigned i=0;i<21;++i) c.profile.booleans["awardShow"+std::to_string(i)]=c.profile.booleans[std::string(Awards::conditionKey(i).view())];
}
GameAwardManager::~GameAwardManager()=default; // Native does not delete an independent pending effect here.
void GameAwardManager::loadXML(const XmlNode& n) {View::loadXML(n);}
void GameAwardManager::returned() {auto& game=instance()->context_.game;game.enable();game.absoluteEnable();}
bool GameAwardManager::condition(unsigned i) {
    auto& p=context_.profile;
    switch(i) {
    case 0:return p.uints["awQCollect"]>99;
    case 1:return p.uints["awQProcess"]>99;
    case 2:return p.uints["awQCollect"]>999;
    case 3:return p.uints["awQProcess"]>999;
    case 4:return p.uints["awQTask"]>19;
    case 5:return p.uints["awQTask"]>39;
    case 6:return p.booleans["awEarn"];
    case 7: {bool result=Warehouse::instance()->level()==3;result&=WaterSource::instance()->level()==2;result&=DigBuilding::instance()->level()==3;result&=MowBuilding::instance()->level()==3;return result;}
    case 8:for(unsigned n=0;n<11;++n) if(!p.booleans["awManDepotBuild"+std::to_string(n)]) return false;return true;
    case 9: {unsigned built=0;for(auto* building:Manufacture::instance()->slots()) if(building && building->built) ++built;return built==7;} //00493af0 counts actual built flags, despite award key's upgrade wording
    case 10:return p.uints["awQDestroy"]>19;
    case 11:return p.ints["awGrowType"]==0xff;
    case 12:return p.floats["awNoDryTime"]>=1200;
    case 13:return p.floats["RunTime"]>=1800;
    case 14:return p.floats["RunTime"]>=10800;
    case 15:return p.booleans["awHighSold"];
    case 16:return context_.world.money()>=1000;
    case 17:return context_.world.money()>=10000;
    case 18:for(unsigned n=0;n<19;++n) if(Market::instance()->goods(n)==0) return false;return true;
    case 19:return p.uints["awAdecvClick"]>=30;
    case 20:for(unsigned n=0;n<20;++n) if(!p.booleans[std::string(Awards::conditionKey(n).view())]) return false;return true;
    default:throw std::out_of_range("Award predicate index");
    }
}
void GameAwardManager::enabledStep() {
    auto& c=context_;auto* screen=Awards::instance();
    if(pending) {
        if(pending->effectState().phase==1) {
            if(!shown(screen)) screen->showTransparently();
            else if(screen->state().transition==0) {pending->setPhase(3);screen->showAwardPanel(*pending,&returned);}
        } else if(pending->effectState().phase==5) {
            const unsigned index=pending->index();c.profile.booleans["awardShow"+std::to_string(index)]=true;screen->markOwned(index);screen->enable();delete pending;pending=nullptr;
        }
        return;
    }
    auto& world=c.world;const auto farmMove=world.farm().movement,wareMove=world.wareDetails().movement,marketMove=world.manufacture().movement;if(farmMove || wareMove || marketMove) return;
    bool clear=!shown(ContractProvide::instance());clear&=!shown(ContractFail::instance());clear&=!shown(ContractPassConfirm::instance());
    clear&=!shown(&world.selectionOverlay());clear&=!shown(&world.sceneOverlay());clear&=!shown(ContractAim::instance());if(!clear) return;
    for(unsigned i=0;i<21;++i) if(!attained[i]) {
        attained[i]=condition(i);if(i!=unsigned(requested) && !attained[i]) continue;requested=-1;
        screen=Awards::instance();if(!screen) {screen=new Awards(nullptr,c.resources,c.settings,c.strings,c.sound,c.input,c.interaction,c.screenSize,c.views,c.controls);screen->hide();screen->returnEvent=&returned;screen->disable();}
        pending=new AwardEffect(nullptr,i,c.resources,c.sound,c.views,c.controls);pending->show();pending->enable();const auto& target=screen->button(i).geometry();pending->effectState().destination={target.x,target.y};
        c.game.disable();c.game.absoluteDisable();return;
    }
}
}
