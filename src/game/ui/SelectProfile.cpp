#include "SelectProfile.h"
#include "ResourceManager.h"
namespace farmar::recovered {
namespace { SelectProfile* currentSelector=nullptr; }
SelectProfile* SelectProfile::instance() noexcept { return currentSelector; }
SelectProfile::SelectProfile(bool hideCancel,bool loadDefault,ProfileSession& profiles,ResourceManager& resources,Settings& settings,
    StringManager& strings,InputBindings& input,ButtonInteraction& interaction,ViewRegistry& views,ControlRegistry& controls)
    :MenuView(nullptr,resources,settings,strings,views,controls),profiles_(profiles),resources_(resources),settings_(settings),strings_(strings),input_(input),interaction_(interaction),controls_(controls),hideCancel_(hideCancel) {
    if(currentSelector) throw "Object already exist.";currentSelector=this;
    ok_=new TextAnimationButton(this,resources,settings,strings,input,interaction,views,controls);
    exit_=new TextAnimationButton(this,resources,settings,strings,input,interaction,views,controls);
    delete_=new TextAnimationButton(this,resources,settings,strings,input,interaction,views,controls);
    nameEdit_=new EditBox(this,resources,settings,strings,input,views,controls);
    background_=new Picture(this,resources,views,controls);logo_=new Picture(this,resources,views,controls);nameBacking_=new Picture(this,resources,views,controls);
    enterName_=new TextLine(this,resources,settings,strings,views,controls);list_=new ListBox(this,resources,settings,strings,input,interaction,views,controls);
    delete_->clickEvent=&deleteClick;nameEdit_->changed=&changed;nameEdit_->confirm=&confirm;
    ok_->clickEvent=&acceptClick;exit_->clickEvent=&cancelClick;list_->selected=&selected;
    if(loadDefault) loadDefaultXML();
}
SelectProfile::~SelectProfile() { onDisabled();currentSelector=nullptr; }
void SelectProfile::loadDefaultXML() { const auto document=resources_.loadSettingsXML(String("UI/SelectUser.xml"));loadXML(*document); }
void SelectProfile::loadXML(const XmlNode& node) {
    MenuView::loadXML(node);stringOK_=node.attribute(WString(L"StringOK"));stringNew_=node.attribute(WString(L"StringNew"));stringConfirm_=node.attribute(WString(L"StringConfirm"));
    const auto load=[&](const char* name,View* target) { XmlNode* child=nullptr;if(node.section(String(name),child)) { target->loadXML(*child);return true; }return false; };
    load("Background",background_);load("BigLogo",logo_);load("PodlogkaPodImia",nameBacking_);load("EnterNameText",enterName_);load("NameEdit",nameEdit_);load("OkButton",ok_);
    if(load("ExitButton",exit_) && hideCancel_) exit_->hide();load("DeleteButton",delete_);delete_->hide();
    if(load("UserList",list_)) updateList();
}
void SelectProfile::updateList() {
    list_->clear();if(auto* profile=profiles_.current()) { std::vector<WString> names;profile->headers().appendNames(names);for(const auto& name:names) list_->append(name); }
}
void SelectProfile::onEnabled() { View::onEnabled();input_.bind(0x1c,&acceptInput,this);input_.bind(1,&cancelInput,this); }
void SelectProfile::onDisabled() { View::onDisabled();input_.unbind(0x1c,&acceptInput,this);input_.unbind(1,&cancelInput,this); }
void SelectProfile::enabledStep() {
    View::enabledStep();const bool enabled=ok_->state().enabled && ok_->state().parentEnabled;
    if(nameEdit_->string().empty()) { if(enabled) ok_->disable(); }else if(!enabled) ok_->enable();
}
void SelectProfile::updateName() {
    const auto& name=nameEdit_->string();if(name.empty()) { ok_->hide();return; }ok_->show();
    for(auto* row:list_->rows()) if(row->text().string().view()==name.view()) {
        ok_->setText(stringOK_);if(!delete_->state().shown || !delete_->state().parentShown) delete_->show();return;
    }
    ok_->setText(stringNew_);delete_->hide();
}
void SelectProfile::changed(EditBox*) { instance()->updateName(); }
bool SelectProfile::confirm(wchar_t* character) { return *character==8 || instance()->nameEdit_->isCharAvailable(*character); }
void SelectProfile::selected(unsigned index,ListBox*) { instance()->select(index); }
void SelectProfile::select(unsigned index) {
    const auto& name=list_->rows().at(index)->text().string();nameEdit_->setString(name);selectedIndex_=index;
    profiles_.close();if(!name.empty()) profiles_.open(name);delete_->show();delete_->enable();ok_->setText(stringOK_);
}
void SelectProfile::accept() {
    if(nameEdit_->string().empty()) return;profiles_.open(nameEdit_->string());disable();discardTransparently();if(accepted) accepted();
}
void SelectProfile::cancel() {
    const auto& button=exit_->state();if(list_->rows().empty() || !button.enabled || !button.parentEnabled || !button.shown || !button.parentShown) return;
    disable();discardTransparently();if(cancelled) cancelled();
}
void SelectProfile::acceptInput(void*) { instance()->accept(); }
void SelectProfile::cancelInput(void*) { instance()->cancel(); }
void SelectProfile::acceptClick(ButtonPrototype*) { instance()->accept(); }
void SelectProfile::cancelClick(ButtonPrototype*) { instance()->cancel(); }
void SelectProfile::deleteClick(ButtonPrototype*) { instance()->requestDelete(); }
YesNoDialog* SelectProfile::requestDelete() {
    disable();hide();auto* dialog=new YesNoDialog(nullptr,resources_,settings_,strings_,input_,interaction_,registry_,controls_);
    dialog->setYesEvent(&confirmDelete);dialog->setNoEvent(&cancelDelete);dialog->text().setString(stringConfirm_,false);dialog->show();dialog->enable();return dialog;
}
void SelectProfile::confirmDelete(YesNoDialog* dialog) {
    auto* self=instance();auto* profile=self->profiles_.current();if(!self->nameEdit_->string().empty()) {
        profile->remove(self->nameEdit_->string());self->updateList();self->delete_->hide();self->nameEdit_->setString(WString());self->exit_->hide();
        const WString name(profile->headers().currentName());self->profiles_.close();if(!name.empty()) self->profiles_.open(name);
    }
    self->registry_.requestDiscard(dialog,String(".\\Ui\\SelectProfile.cpp400"));self->enable();self->show();
}
void SelectProfile::cancelDelete(YesNoDialog* dialog) {
    auto* self=instance();self->registry_.requestDiscard(dialog,String(".\\Ui\\SelectProfile.cpp410"));self->enable();self->show();
}
}
