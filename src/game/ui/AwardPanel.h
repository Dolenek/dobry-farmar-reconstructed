#pragma once
#include "ButtonText.h"
#include "MultilineText.h"
#include "AwardEffect.h"
namespace farmar::recovered {
class SoundManager;class cSound;
// cAwardPanel, native 596 bytes. The effect is borrowed; Picture owns the
// button and both multiline text controls. Source object layout is modern.
class AwardPanel final : public Picture {
public:
    AwardPanel(AwardEffect& effect,ButtonPrototype::Event accepted,ResourceManager& resources,Settings& settings,
        StringManager& strings,SoundManager& sound,InputBindings& input,ButtonInteraction& interaction,
        ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 004761c0
    ~AwardPanel() override; // 00475300
    void loadXML(const XmlNode& node) override; // 00475b00
    void onShown() override; // 00475700
    TextAnimationButton& okButton() noexcept { return *ok_; }
    MultilineText& text() noexcept { return *text_; }
    MultilineText& congratulation() noexcept { return *congratulation_; }
    const Point2& awardPicturePosition() const noexcept { return awardPicturePosition_; }
    const AwardEffect& effect() const noexcept { return effect_; }
    cSound* sound() const noexcept { return sound_; }
private:
    TextAnimationButton* ok_;
    MultilineText *text_,*congratulation_;
    Point2 awardPicturePosition_{}; // original +244/+248, XML stores this without moving borrowed effect
    AwardEffect& effect_; // original +24c
    SoundManager& soundManager_;
    cSound* sound_=nullptr; // original +250, latest root Sound reference
};
}
