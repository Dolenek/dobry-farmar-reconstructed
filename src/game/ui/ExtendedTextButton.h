#pragma once
#include "ButtonText.h"
#include "PictureButton.h"
#include "FieldObject.h"
#include "AnimableAppearance.h"
#include <vector>
namespace farmar::recovered {
// Original extended animation/text-animation specializations: 864/868 bytes. Tooltip
// controls are owned View children, with independent resource appearances.
template<class Base> class BasicExtendedTooltipButton:public Base {
public:
    BasicExtendedTooltipButton(View*,ResourceManager&,Settings&,StringManager&,InputBindings&,
        ButtonInteraction&,std::function<bool()> tutorialActive,
        ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 00428370
    void loadXML(const XmlNode&) override; // 0042a1b0
    void enabledStep() override; // 00428590
    void onEnabled() override; // 00429bb0
    void onDisabled() override; // 004297c0
    void hovered() override; // 00428ff0
    void unhovered() override; // 004293e0
    void clicked() override; // 00499ca0
    void unclicked() override; // 00499cc0
    void hideTips(); // 0049a340, hide self and every tooltip
    void positionTips(); // 00428a90, original fixed 400/300 screen split
    bool tipsDisabled=false;
    float waitTime=2,waitRemaining=0,alphaChangeTime=0.1f,tipsAlpha=0;
    std::int32_t tipsDirection=1;
    Point2 tipOffset{100,100};
    const std::vector<Control*>& tips() const noexcept {return tips_;}
    struct TipLayout {Point2 position;float deep;};
    const std::vector<TipLayout>& tipLayouts() const noexcept {return layouts_;}
    const std::array<AnimableAppearance,4>& tipAppearances() const noexcept {return appearances_;}
    Picture& tipImage() noexcept {return *image_;}
private:
    using Base::resources_;using Base::registry_;using Base::controls_;using Base::geometry_;using Base::interaction_;
    std::function<bool()> tutorialActive_;
    Settings& settings_;StringManager& strings_;
    std::vector<Control*> tips_;
    std::vector<TipLayout> layouts_;
    Picture* image_;
    std::array<AnimableAppearance,4> appearances_;
};
using ExtendedTextAnimationButton=BasicExtendedTooltipButton<TextAnimationButton>;
using ExtendedAnimationButton=BasicExtendedTooltipButton<AnimationButton>;
using ExtendedButton=BasicExtendedTooltipButton<Button>; // 0048fa00, native 864 bytes
using ExtendedPictureButton=BasicExtendedTooltipButton<PictureButton>; // 0049dc20, native 868 bytes, base of production houses
using ExtendedFieldButton=BasicExtendedTooltipButton<FieldObject>; // 0041be20, native 868 bytes, ActionArea base
}
