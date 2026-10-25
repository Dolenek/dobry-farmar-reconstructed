#include "HelpView.h"
#include "GamePauseMenu.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include <utility>
namespace farmar::recovered {
namespace {HelpView* currentHelp=nullptr;}
HelpViewLifetime::HelpViewLifetime(HelpView* value) {if(currentHelp) throw "Object already exist.";currentHelp=value;}
HelpViewLifetime::~HelpViewLifetime() {currentHelp=nullptr;}
HelpView* HelpView::instance() noexcept {return currentHelp;}
HelpView::HelpView(View* parent,HelpViewContext& c):MenuView(parent,c.resources,c.settings,c.strings,c.views,c.controls),HelpViewLifetime(this),context_(c) {
    const auto button=[&] {return new TextAnimationButton(this,c.resources,c.settings,c.strings,c.input,c.interaction,c.views,c.controls);};
    ok_=button();next_=button();previous_=button();
    auto xml=c.resources.loadSettingsXML(String("UI/HelpView.xml"));loadXML(*xml);
    ok_->clickEvent=&okClicked;next_->clickEvent=&nextClicked;previous_->clickEvent=&previousClicked;
    previous_->hide(); //0047fc5d: receiver is the PrevButton, not the HelpView
}
HelpView::~HelpView()=default;
void HelpPage::readContent(const XmlNode& node,HelpViewContext& c) {
    XmlNode* list=nullptr;
    if(node.section(String("Pictures"),list)) for(unsigned i=0;i<list->children().size();++i) {
        XmlNode* child=nullptr;if(!list->section(i,child)) continue;
        auto* picture=new Picture(this,c.resources,c.views,c.controls);picture->loadXML(*child);pictures_.push_back(picture);
    }
    if(node.section(String("Texts"),list)) for(unsigned i=0;i<list->children().size();++i) {
        XmlNode* child=nullptr;if(!list->section(i,child)) continue;
        auto* text=new MultilineText(this,c.resources,c.settings,c.strings,c.views,c.controls);text->loadXML(*child);texts_.push_back(text);
    }
}
void HelpView::loadXML(const XmlNode& node) {
    MenuView::loadXML(node);XmlNode* child=nullptr;
    for(const auto& entry:{std::pair<const char*,View*>{"OkButton",ok_},{"NextButton",next_},{"PrevButton",previous_}})
        if(node.section(String(entry.first),child)) entry.second->loadXML(*child);
    if(node.section(String("Pages"),child)) {
        pages_.resize(child->children().size(),nullptr);
        for(unsigned i=0;i<child->children().size();++i) {
            XmlNode* entry=nullptr;if(!child->section(i,entry)) continue;
            auto* page=new HelpPage(this,context_);pages_[i]=page;page->readContent(*entry,context_);
        }
    }
    page_=0;pages_[0]->show();pages_[0]->enable();pages_[1]->hide();
}
void HelpView::nextPage() {
    pages_[page_]->hide();++page_;pages_[page_]->show();pages_[page_]->enable();
    if(page_==pages_.size()-1) next_->hide();previous_->show();previous_->enable();
}
void HelpView::previousPage() {
    pages_[page_]->hide();--page_;pages_[page_]->show();pages_[page_]->enable();
    if(page_==0) previous_->hide();next_->show();next_->enable();
}
void HelpView::close() {hideTransparently();GamePauseMenu::instance()->showTransparently();}
void HelpView::okClicked(ButtonPrototype*) {instance()->close();}
void HelpView::nextClicked(ButtonPrototype*) {instance()->nextPage();}
void HelpView::previousClicked(ButtonPrototype*) {instance()->previousPage();}
}
