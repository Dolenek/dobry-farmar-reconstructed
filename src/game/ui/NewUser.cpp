#include "NewUser.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
namespace farmar::recovered {
namespace {NewUser* currentNewUser=nullptr;}
NewUserLifetime::NewUserLifetime(NewUser* value) {if(currentNewUser) throw "Object already exist.";currentNewUser=value;}
NewUserLifetime::~NewUserLifetime() {currentNewUser=nullptr;}
NewUser* NewUser::instance() noexcept {return currentNewUser;}
NewUser::NewUser(NewUserContext& c):NewUserLifetime(this),MenuView(nullptr,c.resources,c.settings,c.strings,c.views,c.controls),context_(c) {
    ok_=new TextAnimationButton(this,c.resources,c.settings,c.strings,c.input,c.interaction,c.views,c.controls);
    name_=new EditBox(this,c.resources,c.settings,c.strings,c.input,c.views,c.controls);
    ok_->clickEvent=&accepted;
    loadXML(*c.resources.loadSettingsXML(String("UI/NewUser.xml")));
    name_->confirm=&confirm;name_->changed=&changed;
}
NewUser::~NewUser() {onDisabled();}
void NewUser::loadXML(const XmlNode& node) {
    MenuView::loadXML(node);XmlNode* child=nullptr;
    if(nameBacking_ && node.section(String("PodlogkaPodImia"),child)) nameBacking_->loadXML(*child);
    if(node.section(String("NameEdit"),child)) name_->loadXML(*child);
    if(node.section(String("OkButton"),child)) ok_->loadXML(*child);
    XmlNode* texts=nullptr;if(node.section(String("Texts"),texts)) for(unsigned i=0;i<texts->children().size();++i) if(texts->section(i,child)) {
        auto& c=context_;auto* text=new TextLine(this,c.resources,c.settings,c.strings,c.views,c.controls);text->loadXML(*child);addDecoration(text);labels_.push_back(text); //00505f3a inserts into MenuView+2c
    }
}
void NewUser::enabledStep() {
    const bool enabled=ok_->state().enabled && ok_->state().parentEnabled;
    if(name_->string().empty()) {if(enabled) ok_->disable();}else if(!enabled) ok_->enable();
}
void NewUser::onEnabled() {View::onEnabled();context_.input.bind(0x1c,&enter,this);}
void NewUser::onDisabled() {View::onDisabled();context_.input.unbind(0x1c,&enter,this);}
bool NewUser::confirm(wchar_t* value) {return *value==8 || instance()->name_->isCharAvailable(*value);}
void NewUser::changed(EditBox*) {auto& self=*instance();if(self.name_->string().empty()) self.ok_->hide();else self.ok_->show();}
void NewUser::accepted(ButtonPrototype*) {instance()->accept();}void NewUser::enter(void*) {instance()->accept();}
void NewUser::accept() {
    if(name_->string().empty()) return;
    context_.profiles.open(name_->string());discardTransparently();if(completed) completed();
}
}
