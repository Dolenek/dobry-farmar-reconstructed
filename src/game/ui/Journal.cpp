#include "Journal.h"
#include "GameMenu.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include <sstream>
#include <utility>
namespace farmar::recovered {
namespace {Journal* currentJournal=nullptr;}
Journal* Journal::instance() noexcept {return currentJournal;}
Journal::Journal(View* parent,JournalContext& c):MenuView(parent,c.resources,c.settings,c.strings,c.views,c.controls),context_(c) {
    if(currentJournal) throw "Object already exist.";currentJournal=this;
    up_=new AnimationButton(this,c.resources,c.input,c.interaction,c.views,c.controls);
    down_=new AnimationButton(this,c.resources,c.input,c.interaction,c.views,c.controls);
    ok_=new TextAnimationButton(this,c.resources,c.settings,c.strings,c.input,c.interaction,c.views,c.controls);
    prototype_=new MultilineText(this,c.resources,c.settings,c.strings,c.views,c.controls);
    loadXML(*c.resources.loadSettingsXML(String("UI/gameUI/Journal.xml")));
    up_->clickEvent=&upClicked;down_->clickEvent=&downClicked;ok_->clickEvent=&okClicked;
}
Journal::~Journal() {currentJournal=nullptr;} // Owned text controls are deleted by View.
void Journal::loadXML(const XmlNode& node) {
    MenuView::loadXML(node);XmlNode* child=nullptr;
    for(const auto& entry:{std::pair<const char*,View*>{"UpButton",up_},{"DownButton",down_},{"OkButton",ok_},{"MultiProto",prototype_}})
        if(node.section(String(entry.first),child)) entry.second->loadXML(*child);
    const auto color=[&](const wchar_t* name,Color& value) {const auto text=node.attribute(WString(name));if(!text.empty()) {unsigned raw;std::wistringstream input(text.c_str());if(input>>std::hex>>raw) value=Color::fromARGB(raw);}};
    color(L"ReadColor",readColor);color(L"UnreadColor",unreadColor);
    auto text=node.attribute(WString(L"FitRect"));if(!text.empty()) {std::wistringstream input(text.c_str());input>>fit[0]>>fit[1]>>fit[2]>>fit[3];}
    text=node.attribute(WString(L"MinDist"));if(!text.empty()) {std::wistringstream input(text.c_str());input>>minDistance;}
}
void Journal::open() {
    auto& hud=*GameMenu::instance();hud.disable();hud.absoluteDisable();count=int(hud.messages().size());setLast(int(unsigned(count)-1));showTransparently();
}
void Journal::close() {auto& hud=*GameMenu::instance();hud.enable();hud.absoluteEnable();hud.setMessagesRead(true);hideTransparently();}
void Journal::refreshArrows() {if(first==0) up_->hide();else up_->show();if(last<int(unsigned(count)-1)) down_->show();else down_->hide();}
void Journal::setFirst(int index) {layout(index,false);}
void Journal::setLast(int index) {layout(index,true);}
void Journal::layout(int index,bool backward) {
    for(auto* text:displayed_) delete text;displayed_.clear();
    if(index<0) index=0;if(count<=index) index=int(unsigned(count)-1);first=last=index;
    const auto& messages=GameMenu::instance()->messages();std::vector<Point2> sizes;float total=0;
    for(;;) {
        const int current=backward ? first:last;auto* text=new MultilineText(*prototype_);
        text->setSize({float(originalSubtract(fit[2],fit[0])),100});text->setString(messages.at(unsigned(current)).text,true);
        const auto size=text->measuredSize();total=float(originalAdd(size[1],total));
        if(current!=index && !(originalSubtract(fit[3],fit[1])>total)) {if(!backward) text->hide();delete text;break;}
        sizes.push_back(size);displayed_.push_back(text);text->setPackedColor((messages.at(unsigned(current)).read ? readColor:unreadColor).toARGB());
        if(backward) {first=int(unsigned(first)-1);if(first<0) break;}
        else {last=int(unsigned(last)+1);if(count<=last) break;}
        total=float(originalAdd(total,minDistance));
    }
    const float x=float(originalMultiply(originalAdd(fit[2],fit[0]),.5));float y=fit[1];
    if(backward) {
        first=int(unsigned(first)+1);
        for(std::size_t i=displayed_.size();i--;) {auto* text=displayed_[i];text->setPosition({x,float(originalAdd(originalMultiply(text->geometry().height,.5),y))});y=float(originalAdd(originalAdd(y,minDistance),sizes[i][1]));}
    } else {
        last=int(unsigned(last)-1);
        for(unsigned i=0;i<displayed_.size();++i) {auto* text=displayed_[i];text->setPosition({x,float(originalAdd(originalMultiply(text->geometry().height,.5),y))});y=float(originalAdd(originalAdd(minDistance,sizes[i][1]),y));}
    }
    refreshArrows();
}
void Journal::upClicked(ButtonPrototype*) {auto& j=*instance();j.setFirst(int(unsigned(j.first)-1));} //00446a60
void Journal::downClicked(ButtonPrototype*) {auto& j=*instance();j.setFirst(int(unsigned(j.first)+1));} //00446a80
void Journal::okClicked(ButtonPrototype*) {instance()->close();}
}
