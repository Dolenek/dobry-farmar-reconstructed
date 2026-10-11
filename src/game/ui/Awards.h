#pragma once
#include "MenuView.h"
#include "ButtonText.h"
#include "AwardButton.h"
namespace farmar::recovered {
class Awards;
class AwardPanel;
// Mirrors the original first singleton base: initialize before MenuView, clear
// after its owned-child destruction, rather than clearing early in ~Awards body.
class AwardsLifetime {
protected:
    explicit AwardsLifetime(Awards* owner);
    ~AwardsLifetime();
};
class Awards final : private AwardsLifetime,public MenuView {
public:
    Awards(View* parent,ResourceManager& resources,Settings& settings,StringManager& strings,SoundManager& sound,
        InputBindings& input,ButtonInteraction& interaction,TooltipFrame::ScreenSize screenSize,
        ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 004787e0
    ~Awards() override; // 00478dc0
    void loadXML(const XmlNode& node) override; // 004763a0
    void onShown() override; // 00477ab0
    void refresh(); // 004776b0, real typed profile booleans awardShow0..20
    void markOwned(unsigned index); // 00476ec0, force this button's ownership without writing a profile property
    AwardPanel* showAwardPanel(AwardEffect& effect,void(*unusedReturnEvent)()=nullptr); // 004783d0, second argument unused in original
    void returnFromScreen(); // 00476ae0
    static void onAwardPanelOk(ButtonPrototype* button); // 00477e80, generic parent discard then award condition commit
    static const String& conditionKey(unsigned index); // 0040ab50, original indexed condition names
    static Awards* instance() noexcept;
    AwardButton& button(unsigned index) { return *buttons_.at(index); }
    TextAnimationButton& returnButton() noexcept { return *return_; }
    TextLine& caption() noexcept { return *caption_; }
    const WString& description(unsigned index) const { return descriptions_.at(index); }
    std::function<void()> returnEvent; // native +2e8 assigned by MainMenu/award manager after construction
    AwardEffect* pendingEffect=nullptr; // native +2e4, nonowned effect assigned by the gameplay award manager
private:
    ResourceManager& resources_;
    Settings& settings_;StringManager& strings_;SoundManager& sound_;InputBindings& input_;
    ButtonInteraction& interaction_;ControlRegistry& controls_;
    std::array<WString,21> descriptions_;
    std::array<AwardButton*,21> buttons_{};
    TextAnimationButton* return_;
    TextLine* caption_;
    static void returnClick(ButtonPrototype*);
};
}
