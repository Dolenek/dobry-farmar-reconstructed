#pragma once
#include "ExtendedTextButton.h"
#include <array>
#include <vector>
namespace farmar::recovered {
class ActionArea;class TutorialManager;
// Mandatory original Farm/Person dependencies. Production has no default
// provider: registration and queued actions must be supplied by the real world.
struct ActionAreaWorld {
    virtual ~ActionAreaWorld()=default;
    virtual TutorialManager& tutorial()=0;
    virtual void registerArea(ActionArea&)=0; // Farm 00431530
    virtual void queueAreaAction(ActionArea&,const Point2&)=0; // Person 0044d2f0, marker + task
    virtual int warehouseLevel()=0;
    virtual int digBuildingLevel()=0;
    virtual int mowBuildingLevel()=0;
    virtual int waterSourceLevel()=0;
};
struct ActionAreaContext {
    ResourceManager& resources;Settings& settings;StringManager& strings;
    InputBindings& input;ButtonInteraction& interaction;ActionAreaWorld& world;
    ViewRegistry& views;ControlRegistry& controls;
};
// Original common 1284-byte cActionArea. Native +368 is left unspecified by
// its ctor; getAreaExtent returns it verbatim. Derived initialization and its
// whole-game uses remain under audit, not replaced with a guessed dimension.
class ActionArea:public ExtendedFieldButton {
public:
    ActionArea(unsigned type,View*,ActionAreaContext&); // 0041c010
    ~ActionArea() override; // 004187a0
    void loadXML(const XmlNode&) override; // 0041cd10
    void clicked() override; // 00418c60
    void onStateChanged(ButtonState) override; // 00417e10
    Point2 goal(unsigned index); // 00418800: actual ABI is output + one index
    Point2 areaExtent() const noexcept; // 00417f60
    void enqueueClick(); // 00418110
    Picture& overlay() noexcept {return *overlay_;}
    unsigned areaType;float areaExtentHeight; // +364/+368; second word not initialized by ctor
    std::array<Point2,48> parkingOffsets{}; // +36c; four rows, twelve points
    std::vector<Point2> clickOffsets; // +4f0..4f8
    unsigned clickIndex=0; // +4fc, keeps incrementing after the last point
protected:
    ActionAreaContext& areaContext_;
private:
    Picture* overlay_;
};
}
