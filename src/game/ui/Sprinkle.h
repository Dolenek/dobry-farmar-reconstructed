#pragma once
#include "ActionArea.h"
#include "WaterEmitters.h"
#include "ProfileProperties.h"
namespace farmar::recovered {
class Sprinkle;
// Actual Farm and cField callers. Every operation is mandatory until the
// corresponding world implementation is reconstructed; no substitute behavior.
struct SprinkleWorld:ActionAreaWorld {
    virtual bool sprinkleMoveActive()=0;
    virtual void setSprinkleMoveActive(bool)=0;
    virtual ActionArea* areaAt(const Point2&,unsigned mask)=0; // 0042df60
    virtual bool buildingAt(const Point2&)=0; // 0042e3a0(point,1)
    virtual std::vector<ActionArea*> areasAt(const Point2&,unsigned mask)=0; // 00431950
    virtual bool canPlaceSprinkle(Sprinkle&,const Point2&)=0; // 0042d290
    virtual std::vector<ActionArea*> wateredFields(const Point2&,float radius)=0; // 004326c0
    virtual void clearWaterPreview()=0; // 0042fa90
    virtual void previewWater(ActionArea&,bool)=0; // 00436880
    virtual double addWater(ActionArea&,float)=0; // 00451680
    virtual void setFarmActionButtonEnabled(bool)=0; // 0042c7e0 disables Farm+9c, 0042c7f0 enables it
    virtual int money()=0;virtual void setMoney(int)=0;
    virtual void removeSprinkle(Sprinkle&)=0; // 00433de0
};
struct SprinkleContext {ActionAreaContext area;ProfileProperties& profile;GameRandom& random;SprinkleWorld& world;};
class Sprinkle final:public ActionArea {
public:
    Sprinkle(View*,unsigned mode,unsigned id,SprinkleContext&); // 00460180
    ~Sprinkle() override; // 0045e9d0
    void loadXML(const XmlNode&) override; // 0045eb40
    void setStoredPosition(const Point2&); // 0045f340; not a virtual slot
    void setStatus(unsigned); // 0045f3d0, integer despite Ghidra float signature
    unsigned status();
    void clicked() override;void unclicked() override; // 0045f580 / 0045fb40
    void hovered() override;void unhovered() override; // 0045f230 / 0045f210
    void enabledStep() override; // 00460450
    void absoluteStep() override {} // original Sprinkle slot +0c is RET 004bd820
    void onStateChanged(ButtonState state) override {AnimationButton::onStateChanged(state);} // 0045ceb0 -> 004f1ce0, not ActionArea overlay
    SprinkleEmitter& effect() {return *effect_;}
    unsigned id;float radius;bool installed;
    Point2 pressPosition{0,0},dragOffset{0,0};
    Color normalColor{0,0,0,0},unfitColor{0,0,0,0};
    const std::vector<ActionArea*>& fields() const {return fields_;}
private:
    SprinkleContext& sprinkleContext_;SprinkleEmitter* effect_=nullptr;
    std::vector<ActionArea*> fields_;
    cResource* installSound_=nullptr; cResource* takeSound_=nullptr;cResource* dropSound_=nullptr;
    std::string key(const char*) const;
    Point2 dragPosition(const Point2&) const;
    void play(cResource*);void positionEffect();
}; // native 1380 / 0x564; editable layout intentionally separate
// Actual Farm factory 00433980. The Farm owns count and the moving-picture
// parent; payment happens on placement in Sprinkle::clicked, not here.
Sprinkle* createFarmSprinkle(View* parent,unsigned cost,unsigned& count,SprinkleContext&);
}
