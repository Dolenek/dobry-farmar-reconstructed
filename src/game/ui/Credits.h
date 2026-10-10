#pragma once
#include "MenuView.h"
#include "ButtonText.h"
namespace farmar::recovered {
// Original cCredits: singleton interface + MenuView subobject at +4, 64 bytes.
class Credits final : public MenuView {
public:
    Credits(View& mainMenu,ResourceManager& resources,Settings& settings,StringManager& strings,
        InputBindings& input,ButtonInteraction& interaction,ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 00479b30
    ~Credits() override; // 00478e90
    void loadXML(const XmlNode& node) override; // 004795b0
    void returnToMenu(); // 00479110
    static Credits* instance() noexcept;
    TextAnimationButton& returnButton() noexcept { return *return_; }
private:
    View& mainMenu_;ResourceManager& resources_;Settings& settings_;StringManager& strings_;ControlRegistry& controls_;
    TextAnimationButton* return_=nullptr;
    static void returnClick(ButtonPrototype*);
};
}
