#pragma once
#include "MenuView.h"
#include "ButtonText.h"
#include "MultilineText.h"
#include <vector>
namespace farmar::recovered {
struct JournalContext {
    ResourceManager& resources;Settings& settings;StringManager& strings;
    InputBindings& input;ButtonInteraction& interaction;ViewRegistry& views;ControlRegistry& controls;
};
// Actual cJournal (156 native bytes), singleton0065f764, MenuView base at0.
class Journal final:public MenuView {
public:
    Journal(View*,JournalContext&); //00446aa0
    ~Journal() override; //00446d10
    static Journal* instance() noexcept;
    void loadXML(const XmlNode&) override; //00445880
    void open();void close(); //00446d50/004457e0
    void setFirst(int);void setLast(int); //00446420/00446740
    AnimationButton& upButton() {return *up_;}AnimationButton& downButton() {return *down_;}
    TextAnimationButton& okButton() {return *ok_;}MultilineText& prototype() {return *prototype_;}
    const std::vector<MultilineText*>& visibleMessages() const {return displayed_;}
    std::array<float,4> fit{};float minDistance;Color readColor{},unreadColor{};
    int count,first,last; // Native ctor does not initialize these; open establishes them.
private:
    JournalContext& context_;AnimationButton *up_,*down_;TextAnimationButton* ok_;MultilineText* prototype_;
    std::vector<MultilineText*> displayed_;
    void layout(int,bool);void refreshArrows();
    static void upClicked(ButtonPrototype*);static void downClicked(ButtonPrototype*);static void okClicked(ButtonPrototype*);
};
}
