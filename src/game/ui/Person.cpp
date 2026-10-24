#include "Person.h"
#include "FarmAreas.h"
#include "SeedPackage.h"
#include "DigBuilding.h"
#include "MowBuilding.h"
#include "WaterSource.h"
#include "StateView.h"
#include "ResourceManager.h"
#include "OriginalArithmetic.h"
#include <stdexcept>
#include <string>
namespace farmar::recovered {
namespace {Person* currentPerson=nullptr;}
PersonLifetime::PersonLifetime(Person* value) {if(currentPerson) throw "Object already exist.";currentPerson=value;}
PersonLifetime::~PersonLifetime() {currentPerson=nullptr;}
Person* Person::instance() noexcept {return currentPerson;}
Person::Person(View* parent,PersonContext& c):FieldObject(parent,c.resources,c.input,c.interaction,c.views,c.controls),PersonLifetime(this),context_(c),
    visualContext_{c.resources,c.interaction,c.profile,c.configuration,*this,c.views,c.controls},visual_(*this,visualContext_),sounds_(visual_.sounds,c.sound),
    tasksContext_{c.resources,c.views,c.controls,visual_.markerAppearance,visual_.markerSize},tasks_(tasksContext_),
    walkContext_{*this,visual_,c.profile,c.farm,cancelRequested},vehicleContext_{walkContext_,*this} {
    loadXML(*c.resources.loadSettingsXML(String("Person/Person.xml")));
    storePosition({c.profile.floats["XPersPos"],c.profile.floats["YPersPos"]},true);
    for(auto* area:c.world.areas().seedPackages()) {auto* seed=dynamic_cast<SeedPackage*>(area);if(!seed) throw std::logic_error("Person requires actual SeedPackage");if(seed->status()==3) {carriedSeed=seed;setHeldType(10);break;}}
    installIdle(heldType()==11);
}
Person::~Person() {sounds_.stop(6);visual_.releaseSounds();sounds_.clearWorkChannels();}
void Person::loadXML(const XmlNode& node) {visual_.loadXML(node);}
void Person::absoluteStep() {ButtonPrototype::enabledStep();ActionProcessor::step();}
void Person::requestCancel(bool value) {cancelRequested=value;if(value) tasks_.abandonCurrent();}
int Person::heldType() {return static_cast<int>(context_.profile.uints["PCarry"]);}
void Person::setHeldType(int value) {auto& stored=context_.profile.uints["PCarry"];if(stored!=unsigned(value)) stored=unsigned(value);}
float Person::heldWater() {return context_.profile.floats["WaterLevel"];}
void Person::setHeldWater(float value) {context_.profile.floats["WaterLevel"]=value;}
unsigned Person::productCount(unsigned index) {return context_.profile.uints["PCC"+std::to_string(index)];}
void Person::setProductCount(unsigned index,unsigned value) {auto& stored=context_.profile.uints["PCC"+std::to_string(index)];if(stored!=value) stored=value;}
bool Person::productCapacity() {auto* mow=MowBuilding::instance();if(!mow) throw std::logic_error("Person requires actual MowBuilding");return visual_.carriedProducts()<mow->levels.at(unsigned(mow->level())).productionNum;}
bool Person::carriesOtherProducts(unsigned kind) {for(unsigned i=0;i<8;++i) if(i!=kind && productCount(i)) return true;return false;}
void Person::refreshHeldAppearance() {visual_.refreshHeldAppearance();}
void Person::advanceAnimation(double elapsed) {animation_.time=float(originalAdd(elapsed,animation_.time));markTransformDirty();}
void Person::storePosition(const Point2& position,bool move) {storePersonPosition(*this,context_.profile,position,move);}
int Person::digLevel() {auto* dig=DigBuilding::instance();if(!dig) throw std::logic_error("Person requires actual DigBuilding");return dig->level();}
int Person::mowLevel() {auto* mow=MowBuilding::instance();if(!mow) throw std::logic_error("Person requires actual MowBuilding");return mow->level();}
unsigned Person::carriedSeedKind() {if(!carriedSeed) throw std::logic_error("Person carry10 requires actual SeedPackage");return carriedSeed->kind();}
unsigned Person::carriedSeedCount() {if(!carriedSeed) throw std::logic_error("Person carry10 requires actual SeedPackage");return carriedSeed->count();}
}
