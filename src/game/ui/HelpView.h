#pragma once
#include "MenuView.h"
#include "ButtonText.h"
#include "MultilineText.h"
#include <vector>
namespace farmar::recovered {
struct HelpViewContext {
    ResourceManager& resources;Settings& settings;StringManager& strings;
    InputBindings& input;ButtonInteraction& interaction;ViewRegistry& views;ControlRegistry& controls;
};
// Native cHelpPage (68 bytes). The two lists only remember View-owned controls.
class HelpPage final:public View {
public:
    HelpPage(View* parent,HelpViewContext& context):View(parent,context.views) {} //0047dbe0
    void readContent(const XmlNode&,HelpViewContext&); //0047ed50, deliberately does not call View::loadXML
    const std::vector<Picture*>& pictures() const {return pictures_;}
    const std::vector<MultilineText*>& texts() const {return texts_;}
private:
    std::vector<Picture*> pictures_;std::vector<MultilineText*> texts_;
};
// Native cHelpView (92 bytes), singleton0065f778 and MenuView base at0.
class HelpView final:public MenuView {
public:
    HelpView(View*,HelpViewContext&); //0047fa60
    ~HelpView() override; //0047dde0
    static HelpView* instance() noexcept;
    void loadXML(const XmlNode&) override; //0047f370
    void nextPage();void previousPage();void close(); //0047e250/0047e690/0047de80
    unsigned currentPage() const {return page_;}
    const std::vector<HelpPage*>& pages() const {return pages_;}
    TextAnimationButton& okButton() {return *ok_;}
    TextAnimationButton& nextButton() {return *next_;}
    TextAnimationButton& previousButton() {return *previous_;}
private:
    HelpViewContext& context_;TextAnimationButton *ok_,*next_,*previous_;
    unsigned page_=0;std::vector<HelpPage*> pages_;
    static void okClicked(ButtonPrototype*);static void nextClicked(ButtonPrototype*);static void previousClicked(ButtonPrototype*);
};
}
