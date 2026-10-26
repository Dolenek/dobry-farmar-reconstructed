#pragma once
#include "MenuView.h"
#include "EditBox.h"
#include "ButtonText.h"
#include "ProfileSession.h"
#include <vector>
namespace farmar::recovered {
struct NewUserContext {
    ResourceManager& resources;Settings& settings;StringManager& strings;ProfileSession& profiles;
    InputBindings& input;ButtonInteraction& interaction;ViewRegistry& views;ControlRegistry& controls;
};
class NewUser;
class NewUserLifetime {protected:explicit NewUserLifetime(NewUser*);~NewUserLifetime();};
// Actual first-profile screen, native76 bytes, singleton006604f4.
// This is independent of the later SelectProfile controller.
class NewUser final:private NewUserLifetime,public MenuView {
public:
    explicit NewUser(NewUserContext&); //00506780
    ~NewUser() override; //00506920, MenuView before singleton cleanup
    void loadXML(const XmlNode&) override; //00505bf0
    void enabledStep() override; //005053a0
    void onEnabled() override;void onDisabled() override; //00506720 /00506750
    void accept(); //005062d0
    static NewUser* instance() noexcept;
    using Completion=void(*)();Completion completed=nullptr; //raw+48
    EditBox& nameEdit() {return *name_;}TextAnimationButton& okButton() {return *ok_;}
    const std::vector<TextLine*>& labels() const {return labels_;}
private:
    NewUserContext& context_;TextAnimationButton* ok_;EditBox* name_;
    Picture* nameBacking_=nullptr; //raw+44; native ctor leaves this null
    std::vector<TextLine*> labels_; //native loader keeps a temporary, View owns all
    static bool confirm(wchar_t*);static void changed(EditBox*);
    static void accepted(ButtonPrototype*);static void enter(void*);
};
}
