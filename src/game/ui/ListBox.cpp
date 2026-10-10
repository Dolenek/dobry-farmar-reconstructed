#include "ListBox.h"
#include "OriginalArithmetic.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
namespace {
template<class T> void read(const XmlNode& node,const wchar_t* name,T& output) {
    const auto text=node.attribute(WString(name));if(!text.empty()) { std::wistringstream stream(text.c_str());stream>>output; }
}
void readPoint(const XmlNode& node,const wchar_t* name,Point2& output) {
    const auto text=node.attribute(WString(name));if(!text.empty()) { std::wistringstream stream(text.c_str());stream>>output[0];stream.ignore(1);stream>>output[1]; }
}
}
ListBox::ListBox(View* parent,ResourceManager& resources,Settings& settings,StringManager& strings,InputBindings& input,ButtonInteraction& interaction,ViewRegistry& views,ControlRegistry& controls)
    :Picture(parent,resources,views,controls) {
    up_=new Button(this,resources,input,interaction,views,controls);down_=new Button(this,resources,input,interaction,views,controls);
    prototype_=new TextButton(this,resources,settings,strings,input,interaction,views,controls);
    up_->clickEvent=&onUp;down_->clickEvent=&onDown;prototype_->clickEvent=&onSelect;rows_.reserve(20);
}
void ListBox::onUp(ButtonPrototype* button) { static_cast<ListBox*>(button->parent())->scrollUp(); }
void ListBox::onDown(ButtonPrototype* button) { static_cast<ListBox*>(button->parent())->scrollDown(); }
void ListBox::onSelect(ButtonPrototype* button) { auto* box=static_cast<ListBox*>(button->parent());if(box->selected) box->selected(button->userData,box); }
void ListBox::scrollUp() { if(firstRow_>0) { --firstRow_;updateList(); } }
void ListBox::scrollDown() { const unsigned next=firstRow_+1;if(rows_.size()>next) { firstRow_=next;updateList(); } }
void ListBox::append(const WString& name) {
    auto* row=new TextButton(*prototype_);rows_.push_back(row);row->userData=static_cast<unsigned>(rows_.size()-1);row->setText(name);updateList();
}
void ListBox::clear() { for(auto* row:rows_) delete row;std::vector<TextButton*>().swap(rows_);updateList(); }
void ListBox::updateList() {
    const float height=static_cast<float>(originalSubtract(bottomRight_[1],topLeft_[1]));
    const unsigned visible=static_cast<unsigned>(originalDivide(originalSubtract(bottomRight_[1],topLeft_[1]),interlinear_));
    const unsigned count=static_cast<unsigned>(rows_.size());
    if(count-firstRow_<visible) { const unsigned remainder=count-visible;firstRow_=static_cast<int>(remainder)<0 ? 0:remainder; }
    const unsigned end=(std::min)(firstRow_+visible,count);
    for(unsigned i=0;i<firstRow_;++i) rows_.at(i)->hide();
    float y=static_cast<float>(originalAdd(originalMultiply(interlinear_,0.5),originalSubtract(geometry_.y,originalMultiply(height,0.5))));
    for(unsigned i=firstRow_;i<end;++i) { rows_[i]->show();rows_[i]->enable();rows_[i]->setPosition({geometry_.x,y});y=static_cast<float>(originalAdd(y,interlinear_)); }
    for(unsigned i=end;i<count;++i) rows_[i]->hide();
    if(end==count) down_->hide();else down_->show();
    if(firstRow_==0) up_->hide();else up_->show();
}
void ListBox::loadXML(const XmlNode& node) {
    Picture::loadXML(node);read(node,L"Interlinear",interlinear_);read(node,L"MaxRows",maxRows_);readPoint(node,L"TopLeft",topLeft_);readPoint(node,L"BottomRight",bottomRight_);
    XmlNode* child=nullptr;
    if(node.section(String("UpButton"),child)) up_->loadXML(*child);
    if(node.section(String("DownButton"),child)) down_->loadXML(*child);
    if(node.section(String("Button"),child)) { prototype_->loadXML(*child);prototype_->hide(); }
    if(node.section(String("Strings"),child)) {
        unsigned count=0;read(*child,L"Count",count);
        // 00520b45 writes into reserved storage without advancing vector end.
        // Existing logical entries are replaced; additional clones stay owned
        // by the View hierarchy, but are absent from the active row vector.
        if(count>rows_.capacity()) throw std::out_of_range("Original ListBox Strings exceeds its reserved row storage");
        for(unsigned i=0;i<count;++i) {
            auto* row=new TextButton(*prototype_);row->userData=i;
            if(i<rows_.size()) rows_[i]=row;
            const std::wstring key=L"str"+std::to_wstring(i);
            row->setText(child->attribute(WString(key)));
        }
    }
    updateList();
}
}
