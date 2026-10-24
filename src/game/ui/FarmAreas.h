#pragma once
#include "ActionArea.h"
#include "ProductionPlacement.h"
namespace farmar::recovered {
class FarmField;
// Actual Farm area/field collections, factored from the scene's UI ownership.
// Farm owns these pointers through its View tree and explicit discard requests.
class FarmAreas {
public:
    void registerArea(ActionArea&); // 00431530
    ActionArea* topAt(const Point2&,unsigned mask); // 0042df60
    ActionArea* firstAt(const Point2&,unsigned mask); // 0042e3a0
    void appendAt(const Point2&,unsigned mask,std::vector<ActionArea*>&); // 00431950, does not clear
    std::vector<ActionArea*> wateredFields(const Point2&,float radius); // 004326c0, clears output
    std::vector<ActionArea*> seedPackages(); // 00432260, type32
    bool hasReadyCulture(unsigned); //0042f630, includes inactive fields
    bool hasDryField(); // 0042caa0 / 004381c0
    void clearWaterPreview(); // 0042fa90
    bool canPlaceSprinkle(ActionArea*,const Point2&); // 0042d290
    bool canPlaceSeed(ActionArea*,const Point2&); // 0042ce30
    FarmField* fieldForSeedTutorial(); // 0042e7c0, first active fallback
    void remove(ActionArea&,ViewRegistry&); // 00433420
    std::vector<ActionArea*> areas;std::vector<FarmField*> fields;
    ProductionRectangle bounds{40,80,760,500}; // actual ctor 00435c40
};
}
