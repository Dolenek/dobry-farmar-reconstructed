#include "FarmAreas.h"
#include "FarmField.h"
#include "OriginalArithmetic.h"
#include <algorithm>
namespace farmar::recovered {
namespace {
ProductionRectangle rectangle(const Control& c) {const auto& g=c.geometry();const float x=float(originalSubtract(g.x,originalMultiply(g.width,.5))),y=float(originalSubtract(g.y,originalMultiply(g.height,.5)));return {x,y,float(originalAdd(x,g.width)),float(originalAdd(y,g.height))};}
}
void FarmAreas::registerArea(ActionArea& a) {areas.push_back(&a);}
ActionArea* FarmAreas::topAt(const Point2& p,unsigned mask) {ActionArea* found=nullptr;float depth=-99999;for(auto* a:areas) if(a->areaType==(a->areaType&mask) && a->pointInRect(p) && depth<a->deep()) {depth=a->deep();found=a;}return found;}
ActionArea* FarmAreas::firstAt(const Point2& p,unsigned mask) {for(auto* a:areas) if(a->areaType==(a->areaType&mask) && a->pointInRect(p)) return a;return nullptr;}
void FarmAreas::appendAt(const Point2& p,unsigned mask,std::vector<ActionArea*>& out) {for(auto* a:areas) if(a->areaType==(a->areaType&mask) && a->pointInRect(p)) out.push_back(a);}
std::vector<ActionArea*> FarmAreas::wateredFields(const Point2& p,float radius) {
    std::vector<ActionArea*> out;const float squared=float(originalMultiply(radius,radius));for(auto* f:fields) if(f && f->active) {
        const auto dx=originalSubtract(p[0],f->geometry().x);const float dy=float(originalSubtract(p[1],f->geometry().y));
        if(originalAdd(originalMultiply(dx,dx),originalMultiply(dy,dy))<=squared) out.push_back(f);
    }return out;
}
std::vector<ActionArea*> FarmAreas::seedPackages() {std::vector<ActionArea*> out;for(auto* a:areas) if(a->areaType==32) out.push_back(a);return out;}
bool FarmAreas::hasReadyCulture(unsigned kind) {for(auto* field:fields) if(field->culture()==kind && field->actionState()==1) return true;return false;}
bool FarmAreas::hasDryField() {for(auto* f:fields) if(f->water()<=f->waterSignLevel) return true;return false;}
void FarmAreas::clearWaterPreview() {for(auto* a:areas) if(a->areaType==1) static_cast<FarmField*>(a)->preview(false);}
bool FarmAreas::canPlaceSprinkle(ActionArea* candidate,const Point2& p) {
    if(!candidate || !productionPointInside(bounds,p)) return false;
    for(auto* a:areas) {const auto& s=a->state();if(a!=candidate && s.shown && s.parentShown && a->areaType!=32 && a->areaType!=64 && a->areaType!=1)
        if(productionRectangleTouches(rectangle(*a),candidate) && productionMasksOverlap(*a,*candidate,&a->clickMask(),&candidate->clickMask())) return false;
    }return true;
}
void FarmAreas::remove(ActionArea& a,ViewRegistry& views) {const auto found=std::find(areas.begin(),areas.end(),&a);if(found!=areas.end()) {areas.erase(found);views.requestDiscard(&a,String(".\\Game\\Farm\\Farm.cpp382"));}}
bool FarmAreas::canPlaceSeed(ActionArea* candidate,const Point2& p) {
    if(!candidate || !productionPointInside(bounds,p)) return false;
    for(auto* a:areas) {const auto& s=a->state();if(a!=candidate && s.shown && s.parentShown && a->areaType!=32 && a->areaType!=64)
        if(productionRectangleTouches(rectangle(*a),candidate) && productionMasksOverlap(*a,*candidate,&a->clickMask(),&candidate->clickMask())) return false;
    }return true;
}
FarmField* FarmAreas::fieldForSeedTutorial() {
    FarmField* first=nullptr;for(auto* a:areas) if(a->areaType==1) {auto* f=static_cast<FarmField*>(a);if(!f->active) continue;
        if(!first) first=f;if(f->culture()==0 && f->water()==0) return f;
    }return first;
}
}
