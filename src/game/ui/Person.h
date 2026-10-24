#pragma once
#include "FieldObject.h"
#include "PersonVisuals.h"
#include "PersonSounds.h"
#include "PersonTasks.h"
#include "PersonVehicleWalk.h"
#include "GameAction.h"
namespace farmar::recovered {
class Person;class FarmAreas;class HoseEmitter;class SeedPackage;class SoundManager;
struct PersonWorld {
    virtual ~PersonWorld()=default;
    virtual int money()=0;virtual void setMoney(int)=0; // actual game00410ce0 /00410d60, including UI/world refresh
    virtual FarmAreas& areas()=0;
    virtual HoseEmitter& hoseEmitter()=0;
    virtual void removeSeed(SeedPackage&)=0; // actual Farm00433420
};
struct PersonContext {
    ResourceManager& resources;InputBindings& input;ButtonInteraction& interaction;
    GameConfiguration& configuration;ProfileProperties& profile;SoundManager& sound;
    StateView& farm;PersonWorld& world;ViewRegistry& views;ControlRegistry& controls;
};
enum class PersonActionKind {Idle,HoseIdle,Walk,VehicleWalk,Dig,TakeSpade,TakeScythe,TakeBucket,Sow,Harvest,Store,Water,HoseWater};
class PersonLifetime {protected:explicit PersonLifetime(Person*);~PersonLifetime();};
// Actual Person actor. ActionProcessor is constructed before the drawing base
// and destroyed after it, as in0044f340 /0044b3a0. Source layout is separate.
class Person final:private ActionProcessor,public FieldObject,private PersonLifetime,private PersonVisualWorld {
public:
    Person(View*,PersonContext&); //0044f340
    ~Person() override; //0044b3a0
    void loadXML(const XmlNode&) override; //0044b600
    void absoluteStep() override; //00449ad0 calls ButtonPrototype.enabledStep again
    void requestCancel(bool); //00433040
    int heldType();void setHeldType(int); //0044ec00 /0044ec80
    float heldWater();void setHeldWater(float); //0044f120 /0044f0a0
    unsigned productCount(unsigned);void setProductCount(unsigned,unsigned); //0044ed50/80
    bool productCapacity();bool carriesOtherProducts(unsigned); //0044edd0 /0044ee30
    void refreshHeldAppearance(); //00450f90
    void advanceAnimation(double);
    void storePosition(const Point2&,bool move);
    static Person* instance() noexcept;
    PersonVisuals& visual() noexcept {return visual_;}PersonSounds& sounds() noexcept {return sounds_;}
    PersonTasks& tasks() noexcept {return tasks_;}ActionProcessor& actions() noexcept {return *this;}
    PersonContext& context() noexcept {return context_;}
    PersonWalkContext& walkContext() noexcept {return walkContext_;}
    PersonVehicleWalkContext& vehicleContext() noexcept {return vehicleContext_;}
    SeedPackage* carriedSeed=nullptr;bool cancelRequested=false;
    void installIdle(bool hose=false);void installWalk(ActionArea&,bool vehicle);
    void installWork(PersonActionKind,ActionArea&,SeedPackage* seed=nullptr);
    void arrive(ActionArea&); //0045a730, vehicle0045bbe0 delegates here
private:
    PersonContext& context_;PersonVisualContext visualContext_;PersonVisuals visual_;PersonSounds sounds_;
    PersonTasksContext tasksContext_;PersonTasks tasks_;PersonWalkContext walkContext_;PersonVehicleWalkContext vehicleContext_;
    int digLevel() override;int mowLevel() override;
    unsigned carriedSeedKind() override;unsigned carriedSeedCount() override;
    void recordActionStarted(bool);
};
}
