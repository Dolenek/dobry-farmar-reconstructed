#pragma once
#include "MenuView.h"
#include "ButtonText.h"
#include "MultilineText.h"
namespace farmar::recovered {
// cYesNoExit, original 100-byte MenuView, with cursor-dependent Yes movement.
class ExitDialog final : public MenuView {
public:
    using Event=void(*)(ExitDialog*);
    ExitDialog(View* parent,ResourceManager& resources,Settings& settings,StringManager& strings,
        InputBindings& input,ButtonInteraction& interaction,ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 0047b1c0
    void loadXML(const XmlNode& node) override; // 0047a4e0
    void enabledStep() override; // 0047ac90
    Event yesEvent=&hideDialog,noEvent=&hideDialog; // 00479c80
    TextAnimationButton& yesButton() noexcept { return *yes_; }
    TextAnimationButton& noButton() noexcept { return *no_; }
    MultilineText& text() noexcept { return *text_; }
    Picture& bee(unsigned index) { return *bees_.at(index); }
    const Point2& originalYesPosition() const noexcept { return originalYes_; }
private:
    ResourceManager& resources_;ButtonInteraction& interaction_;std::array<Picture*,3> bees_{};
    TextAnimationButton *yes_,*no_;MultilineText* text_;Point2 originalYes_{};
    static void hideDialog(ExitDialog*);static void yesClick(ButtonPrototype*);static void noClick(ButtonPrototype*);
    static void yesHover(ButtonPrototype*);static void noHover(ButtonPrototype*);static void leave(ButtonPrototype*);
};
}
