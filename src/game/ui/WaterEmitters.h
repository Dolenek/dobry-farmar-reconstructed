#pragma once
#include "WaterParticles.h"
#include "GameString.h"
#include <list>
namespace farmar::recovered {
class GameRandom;
struct WaterEmissionState {
    float accumulated=0;unsigned maximumCount=0,createdCount=0;
    float interval;float timeLimit=0,timeGenerated=0;
    bool generating=false,infinite=false;unsigned mode=0;
};
class WaterEmitter:public Control {
public:
    WaterEmitter(View*,ResourceManager&,GameRandom&,ViewRegistry&,ControlRegistry&);
    ~WaterEmitter() override;
    void start(float duration); // 0042ccc0, both original template instantiations share this body
    void enabledStep() override {} // derived effects use actual JMP 00445580 -> RET 004bd820
    WaterEmissionState emission;
    const std::list<Picture*>& particles() const noexcept {return particles_;}
protected:
    GameRandom& random_;std::list<Picture*> particles_;
    virtual Picture* emitParticle()=0;
    virtual bool particleFinished(const Picture&) const=0;
    void stepEmitter(const String& discardOrigin);
};
void rotateWaterVector(Point2&,float angle); // 00441810, degree factor 0065d30c
class HoseEmitter final:public WaterEmitter {
public:
    HoseEmitter(View*,ResourceManager&,GameRandom&,const String& path=String("Effects/HoseEffect.xml"),ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 00445420
    void loadXML(const XmlNode&) override; // 00442370
    void absoluteStep() override; // 004440c0
    HoseParticle* createParticle(); // 00442820
    HoseParticle& prototype() {return *prototype_;}
    float angle; // original +264 unspecified until caller sets it
    Point2 speed{0,0};float distortionRadius;Point2 startPoint{0,0},targetPoint{0,0};
private:
    HoseParticle* prototype_;
    Picture* emitParticle() override {return createParticle();}
    bool particleFinished(const Picture&) const override;
}; // native648/288
class SprinkleEmitter final:public WaterEmitter {
public:
    SprinkleEmitter(View*,ResourceManager&,GameRandom&,const String& path=String("Effects/SprinkleEffect.xml"),ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 00464270
    void loadXML(const XmlNode&) override; // 00461370
    void absoluteStep() override; // 00462f10
    SprinkleParticle* createParticle(); // 00461820
    SprinkleParticle& prototype() {return *prototype_;}
    Point2 lifetimeRange{0,0};float randomAngle; // original +270, assigned by actual XML
private:
    SprinkleParticle* prototype_;
    Picture* emitParticle() override {return createParticle();}
    bool particleFinished(const Picture&) const override;
}; // native628/274
}
