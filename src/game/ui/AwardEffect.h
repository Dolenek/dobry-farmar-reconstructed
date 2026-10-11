#pragma once
#include "Control.h"
#include "GameWString.h"
namespace farmar::recovered {
class SoundManager;class cSound;
struct AwardEffectState {
    unsigned phase=0; // +238
    float originalDeep=0;
    Point2 effectSize{},buttonSize{};
    float scale=0,appearSpeed=1,hoverScale=1,hoverSpeed=1;
    bool hoverAnimation=false;
    WString panelText,congratulation;
    Point2 destination{},panelPosition{},initialPosition{};
    float moveSpeed=0;
};
// Original cAwardEffect, 704 bytes. Modern object layout differs; owns its
// texture through Picture and its root Sound through the original resource API.
class AwardEffect final : public Picture {
public:
    AwardEffect(View* parent,unsigned index,ResourceManager& resources,SoundManager& sound,
        ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 004c6a40
    ~AwardEffect() override; // 004c60e0
    void loadXML(const XmlNode& node) override; // 004c61e0
    void setPhase(unsigned value); // 004c5b70
    void enabledStep() override; // 004c5bc0, Picture tick then phase processing
    unsigned index() const noexcept { return index_; }
    AwardEffectState& effectState() noexcept { return effect_; } // original callers also write destination directly
    const AwardEffectState& effectState() const noexcept { return effect_; }
    cSound* sound() const noexcept { return sound_; }
private:
    unsigned index_;
    AwardEffectState effect_;
    SoundManager& soundManager_;
    cSound* sound_=nullptr;
    void advance(float elapsed);
    void hoverSize();
};
}
