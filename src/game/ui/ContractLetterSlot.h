#pragma once
#include "ExtendedTextButton.h"
#include "ContractProvide.h"
#include "ProgressBar.h"
#include <string>
namespace farmar::recovered {
class TutorialManager;class StateView;class ProfileProperties;class SoundManager;class cSound;
class ContractLetterSlot;
struct ContractLetterWorld {
    virtual ~ContractLetterWorld()=default;
    virtual TutorialManager& tutorial()=0;
    virtual StateView& farmScene()=0;
    virtual void removeLetter(ContractLetterSlot&)=0; // actual Farm004341b0
};
struct ContractLetterContext {
    ResourceManager& resources;Settings& settings;StringManager& strings;InputBindings& input;
    ButtonInteraction& interaction;ProfileProperties& profile;SoundManager& sound;
    ContractLetterWorld& world;ViewRegistry& views;ControlRegistry& controls;
};
// Original Farm's owned incoming contract offer, native size0x3fc. Modern
// ownership/layout is separate; its borrowed event also feeds ContractProvide.
class ContractLetterSlot final:public ExtendedAnimationButton,public ContractOfferLetter {
public:
    ContractLetterSlot(View*,unsigned,ContractLetterContext&); //00424ba0
    ~ContractLetterSlot() override; //00421760
    void loadXML(const XmlNode&) override; //00424320
    void absoluteStep() override; //004250d0
    void clicked() override; //00425d90
    void assign(GameplayEvent&,unsigned state,bool restored); //00425770
    GameplayEvent& contractEvent() const override;
    AnimationButton& image() noexcept {return *image_;}
    ProgressBar& progress() noexcept {return *progress_;}
    const AnimableAppearance& readAppearance() const noexcept {return read_;}
    const AnimableAppearance& unreadAppearance() const noexcept {return unread_;}
    cSound* nearSound() const noexcept {return nearSound_;}
    cSound* expiredSound() const noexcept {return expiredSound_;}
    unsigned id,letterState=2;
    float warningTime,blinkPeriod,blinkRemaining;
    GameplayEvent* event=nullptr;
    float timeRemaining,fullDuration;
    bool warning;
    Point2 appear{0,0},place{0,0};float moveRemaining=0,moveSpeed=0;
private:
    ContractLetterContext& context_;ProgressBar* progress_;AnimationButton* image_;
    AnimableAppearance read_,unread_;cSound* nearSound_=nullptr; cSound* expiredSound_=nullptr;
    std::string key(const char*) const;
    void updateProgress();
};
}
