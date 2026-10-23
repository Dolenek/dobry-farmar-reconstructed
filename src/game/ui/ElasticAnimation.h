#pragma once
#include "Control.h"
#include "AnimableAppearance.h"
namespace farmar::recovered {
class SoundManager;class cResource;class ElasticAnimation;
class ElasticAnimationLifetime {protected:explicit ElasticAnimationLifetime(ElasticAnimation*);~ElasticAnimationLifetime();};
// Actual Farm/ElasticAnimation.xml instrument transfer used by tool upgrades
// and Person carry transitions; native1028 bytes, modern layout is separate.
class ElasticAnimation final:public Picture,private ElasticAnimationLifetime {
public:
    ElasticAnimation(View*,ResourceManager&,SoundManager&,ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 0042c2d0
    ~ElasticAnimation() override; // 0042b1a0
    void loadXML(const XmlNode&) override; // 0042bce0, retained old sound refs
    void begin(const Point2& from,const Point2& to,int kind,View* reveal); // 0042b7a0
    void absoluteStep() override; // 0042b280, base absolute body is genuine RET
    static ElasticAnimation* instance() noexcept;
    float rotationSpeed=0,rotation=0,speed,acceleration;
    std::array<AnimableAppearance,10> states;
    Point2 velocity{},accelerationVector{},position{};float remaining=0;
    int kind;View* revealView; // native ctor leaves both unspecified until begin
    cResource* moveSound() const noexcept {return moveSound_;}
private:
    SoundManager& sound_;cResource* moveSound_=nullptr;
};
}
