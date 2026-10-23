#include "FieldTip.h"
#include "FarmField.h"
#include "GameConfiguration.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include <algorithm>
#include <stdexcept>
namespace farmar::recovered {
namespace {
FieldTip* singleton=nullptr;
void child(View& view,const XmlNode& node,const char* name) {XmlNode* section=nullptr;if(node.section(String(name),section)) view.loadXML(*section);}
// Original UTF-16 literals of 0040c750, in culture-index order. TextLine applies
// the actual StringManager translation as for the original global names.
const wchar_t* names[]={L"\u0422\u0440\u0430\u0432\u0430",L"\u041f\u043e\u043c\u0438\u0434\u043e\u0440\u044b",L"\u0414\u044b\u043d\u0438",L"\u041e\u0433\u0443\u0440\u0446\u044b",L"\u0420\u043e\u0437\u044b",L"\u0410\u0440\u0431\u0443\u0437\u044b",L"\u0410\u043d\u0430\u043d\u0430\u0441\u044b",L"\u041f\u0448\u0435\u043d\u0438\u0446\u0430"};
}
FieldTipLifetime::FieldTipLifetime(FieldTip* self) {if(singleton) throw "Object already exist.";singleton=self;}
FieldTipLifetime::~FieldTipLifetime() {singleton=nullptr;}
FieldTip* FieldTip::instance() noexcept {return singleton;}
FieldTip::FieldTip(View* parent,FieldTipContext& c):MovableUnion(parent,c.resources,c.views,c.controls),FieldTipLifetime(this),tipContext_(c) {
    cultureText_=new TextLine(this,c.resources,c.settings,c.strings,c.views,c.controls);
    cultureIcon_=new Picture(this,c.resources,c.views,c.controls);
    water_=new ProgressBar(this,c.resources,c.views,c.controls);fertil_=new ProgressBar(this,c.resources,c.views,c.controls);ready_=new ProgressBar(this,c.resources,c.views,c.controls);
    loadXML(*c.resources.loadSettingsXML(String("UI/gameUI/FarmFieldTip.xml")));
}
void FieldTip::loadXML(const XmlNode& node) {
    MovableUnion::loadXML(node);child(*cultureText_,node,"CultureText");child(*cultureIcon_,node,"CultureIcon");
    child(*water_,node,"WaterFloat");child(*fertil_,node,"FertilFloat");child(*ready_,node,"ReadyFloat");XmlNode* section=nullptr;
    if(node.section(String("Texts"),section)) for(unsigned i=0;i<section->children().size();++i) {XmlNode* item=nullptr;if(section->section(i,item)) {auto* text=new TextLine(this,tipContext_.resources,tipContext_.settings,tipContext_.strings,tipContext_.views,tipContext_.controls);text->loadXML(*item);labels_.push_back(text);}}
    if(node.section(String("Icons"),section)) for(unsigned i=0;i<(std::min)(section->children().size(),std::size_t(8));++i) {XmlNode *item=nullptr,*icon=nullptr;if(section->section(i,item) && item->section(String("Icon"),icon)) icons[i].loadXML(*icon,tipContext_.resources);}
}
void FieldTip::select(FarmField* value) {selected=value;if(value) {showRemaining=.5f;showPending=true;}}
void FieldTip::hideFor(FarmField* value) {if(selected==value) {selected=nullptr;showPending=false;hideRemaining=.3f;}}
void FieldTip::refresh() {
    auto& f=*selected;
    water_->current=float(originalMultiply(originalDivide(f.water(),f.waterCapacity),100));water_->updateLid();
    fertil_->current=float(originalMultiply(originalDivide(f.fertility(),f.fertilCapacity),100));fertil_->updateLid();
    const auto kind=f.culture();if(f.actionState()==1) {ready_->current=100;ready_->updateLid();}
    if(f.actionState()==3) {ready_->current=0;ready_->updateLid();}
    else {const float creation=tipContext_.configuration.products.at(kind).creationTime;
        ready_->current=float(originalMultiply(originalDivide(originalSubtract(creation,f.progress()),creation),100));ready_->updateLid();}
    if(f.actionState()==3) {cultureText_->hide();cultureIcon_->hide();}
    else {cultureText_->show();cultureIcon_->show();cultureText_->setString(WString(names[kind]));cultureIcon_->copyAppearance(icons.at(kind));cultureIcon_->markTransformDirty();}
}
void FieldTip::absoluteStep() {
    MovableUnion::absoluteStep();if(!uiElapsedTime()) throw std::logic_error("FieldTip timer is not bound");
    if(!state().shown || !state().parentShown) {
        if(showPending) {const auto left=originalSubtract(showRemaining,uiElapsedTime()());showRemaining=float(left);
            if(left<=0) {refresh();move(2,{geometry().x,geometry().y},originalPosition);}}
    }else if(selected) refresh();
    else {const auto left=originalSubtract(hideRemaining,uiElapsedTime()());hideRemaining=float(left);if(left<=0) move(1,{geometry().x,geometry().y},hiddenPlace);}
}
}
