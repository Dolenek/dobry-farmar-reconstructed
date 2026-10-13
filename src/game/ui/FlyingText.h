#pragma once
#include "TextLine.h"
#include <vector>
namespace farmar::recovered {
struct FlyingPosition {Point2 position{};float distance=0;Point2 velocity{};}; // native 20 bytes
struct FlyingAlpha {float alpha=0,distance=0,delta=0;}; // native 12 bytes
struct FlyingTextContext {ResourceManager& resources;Settings& settings;StringManager& strings;ViewRegistry& views;ControlRegistry& controls;};
// Actual cFlyText (760 native bytes), used by product and Money effects.
class FlyingText final:public TextLine {
public:
    FlyingText(View*,FlyingTextContext&,const String& xml=String()); // 0047cc50
    ~FlyingText() override=default; // 0047ba00: alpha/position vectors then TextLine
    void loadXML(const XmlNode&) override; // 0047c250
    void enabledStep() override; // 0047d590, actual timer
    void finish(); // 0047d4d0, completion then deferred discard
    void setIconAppearance(const AnimableAppearance&); // 0047b8e0
    Picture* icon() const noexcept {return icon_;}
    const std::vector<FlyingPosition>& positions() const noexcept {return positions_;}
    const std::vector<FlyingAlpha>& alphas() const noexcept {return alphas_;}
    float speed=50,totalDistance=0,distance=0;
    unsigned positionIndex=1,alphaIndex=1;
    using Completion=void(*)(void*);
    Completion completed=nullptr;void* completionArgument=nullptr;
private:
    Picture* icon_=nullptr; // View owns each XML-created icon, including old ones
    std::vector<FlyingPosition> positions_;
    std::vector<FlyingAlpha> alphas_;
};
}
