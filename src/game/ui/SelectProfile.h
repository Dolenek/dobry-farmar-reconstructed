#pragma once
#include "MenuView.h"
#include "ListBox.h"
#include "EditBox.h"
#include "YesNoDialog.h"
#include "ProfileSession.h"
namespace farmar::recovered {
// cSelectProfile: original 196-byte singleton with MenuView subobject at +4.
// This class uses modern ownership/layout and the recovered profile pipeline.
class SelectProfile final : public MenuView {
public:
    using Event=void(*)();
    SelectProfile(bool hideCancel,bool loadDefault,ProfileSession& profiles,ResourceManager& resources,Settings& settings,
        StringManager& strings,InputBindings& input,ButtonInteraction& interaction,
        ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 004fffb0
    ~SelectProfile() override; // 004fe870, always unbinds its own two keys
    void loadXML(const XmlNode& node) override; // 004fcea0
    void loadDefaultXML(); // 004fca60
    void updateList(); // 004fefa0
    void onEnabled() override; // 004fdfd0
    void onDisabled() override; // 004fe020
    void enabledStep() override; // 004fe070
    void accept(); // 004fd780
    void cancel(); // 004fdbb0
    YesNoDialog* requestDelete(); // 004ffb70, owned by its own root registry
    void select(unsigned index); // 004feaa0
    Event accepted=nullptr,cancelled=nullptr; // original +68/+6c, no arguments
    static SelectProfile* instance() noexcept;
    ListBox& list() noexcept { return *list_; }
    EditBox& nameEdit() noexcept { return *nameEdit_; }
    TextAnimationButton& okButton() noexcept { return *ok_; }
    TextAnimationButton& exitButton() noexcept { return *exit_; }
    TextAnimationButton& deleteButton() noexcept { return *delete_; }
    TextLine& enterNameText() noexcept { return *enterName_; }
    Picture& background() noexcept { return *background_; }
    Picture& logo() noexcept { return *logo_; }
    Picture& nameBacking() noexcept { return *nameBacking_; }
    unsigned selectedIndex() const noexcept { return selectedIndex_; }
private:
    ProfileSession& profiles_;ResourceManager& resources_;Settings& settings_;StringManager& strings_;
    InputBindings& input_;ButtonInteraction& interaction_;ControlRegistry& controls_;
    TextAnimationButton *ok_,*exit_,*delete_;EditBox* nameEdit_;Picture *background_,*logo_,*nameBacking_;TextLine* enterName_;ListBox* list_;
    bool hideCancel_;unsigned selectedIndex_=0xffffffffU;WString stringOK_,stringNew_,stringConfirm_;
    static void changed(EditBox* edit); // 004fe9b0..004fea9b
    static bool confirm(wchar_t* character); // 004fc680
    static void selected(unsigned index,ListBox* list);
    static void acceptInput(void*);static void cancelInput(void*);
    static void acceptClick(ButtonPrototype*);static void cancelClick(ButtonPrototype*);static void deleteClick(ButtonPrototype*);
    static void confirmDelete(YesNoDialog* dialog); // 004ff410
    static void cancelDelete(YesNoDialog* dialog); // 004ffa00
    void updateName();
};
}
