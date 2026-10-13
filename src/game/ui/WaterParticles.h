#pragma once
#include "Control.h"
namespace farmar::recovered {
class GameRandom;
Point2& waterParticleGravity() noexcept; // shared 0065d288/8c, initial {0,10}
struct WaterParticleMotion {
    Point2 velocity{0,0};float maximumScale=3;Point2 originalSize{0,0};
    float lifetime=0,elapsed=0;bool finished=false;
};
class HoseParticle final:public Picture {
public:
    using Picture::Picture; // 004416c0, numeric members initialized below
    HoseParticle(const HoseParticle&); // 00441790
    void loadXML(const XmlNode&) override; // 004418f0
    void enabledStep() override; // 00441a60 -> actual Picture step
    void absoluteStep() override; // 00441e20
    WaterParticleMotion motion;float gravity=0;
}; // native604/25c, destructor common Picture
class SprinkleParticle final:public Picture {
public:
    SprinkleParticle(View*,ResourceManager&,GameRandom&,ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 00460a90
    SprinkleParticle(const SprinkleParticle&); // 00460b20
    void loadXML(const XmlNode&) override; // 00460bc0
    void enabledStep() override; // actual JMP Picture::enabledStep, 00460a70
    void absoluteStep() override; // 00460e30
    WaterParticleMotion motion;
private:GameRandom& random_;
}; // native600/258
}
