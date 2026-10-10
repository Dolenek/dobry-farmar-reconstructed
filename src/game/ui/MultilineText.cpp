#include "MultilineText.h"
#include "StringManager.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include <sstream>
namespace farmar::recovered {
namespace {
template<class T> void read(const XmlNode& node,const wchar_t* name,T& output) {
    const auto value=node.attribute(WString(name));if(!value.empty()) { std::wistringstream stream(value.c_str());stream>>output; }
}
}
MultilineText::MultilineText(View* parent,ResourceManager& resources,Settings& settings,StringManager& strings,ViewRegistry& views,ControlRegistry& controls)
    :Control(parent,resources,views,controls),strings_(strings),prototype_(new TextLine(this,resources,settings,strings,views,controls)) {} // 004f5e70
WString MultilineText::normalize(const WString& value) {
    std::wstring result(value.view());std::size_t position=0;
    while((position=result.find(L"/n",position))!=std::wstring::npos) result.replace(position,2,L"\n");
    return WString(result);
}
void MultilineText::setString(const WString& value,bool immediate) {
    const auto normalized=normalize(value);if(normalized.view()==text_.view()) return;
    text_=normalized;if(immediate) makeLines();else textDirty_=true;
}
void MultilineText::enabledStep() { View::enabledStep();if(textDirty_) makeLines(); }
void MultilineText::setAlpha(float value) { Control::setAlpha(value);for(auto* line:lines_) line->setAlpha(value); }
TextLine* MultilineText::createLine(unsigned index) {
    auto* line=new TextLine(*prototype_);line->setParent(this);
    line->setPosition({geometry_.x,static_cast<float>(originalAdd(originalMultiply(originalUnsigned(index),interlinear_),geometry_.y))});
    line->show();line->enable();line->setPackedColor(textColor_.toARGB());line->setAlpha(alpha());lines_.push_back(line);return line;
}
void MultilineText::wrapLine(const WString& text,unsigned& index,TextLine* line) {
    std::wstring current(text.view()),remainder;line->setString(text);
    while(line->textWidth()>geometry_.width) {
        std::size_t at=std::wstring::npos;
        while(true) {
            at=current.rfind(L' ',at);if(at==std::wstring::npos) break;
            if(at==0) { createLine(++index)->setString(WString(remainder));return; }
            remainder=current.substr(at);line->setString(WString(std::wstring_view(current.data(),at)));
            if(line->textWidth()<=geometry_.width) break;
            --at;
        }
        if(remainder.empty()) break;
        line=createLine(++index);current=remainder;line->setString(WString(current));remainder.clear();
    }
}
void MultilineText::makeLines() {
    for(auto* line:lines_) delete line;lines_.clear();prototype_->setSize({geometry_.width,geometry_.height});
    const auto& translated=localized_ ? strings_.getString(text_):text_;unsigned index=0;std::size_t start=0;
    for(std::size_t end=translated.view().find(L'\n');end!=std::wstring_view::npos;end=translated.view().find(L'\n',start)) {
        const WString segment(translated.view().substr(start,end-start));auto* line=createLine(index);line->setString(segment);if(wrap_) wrapLine(segment,index,line);start=end+1;++index;
    }
    if(start<translated.size()) { const WString tail(translated.view().substr(start));auto* line=createLine(index);line->setString(tail);if(wrap_) wrapLine(tail,index,line); }
    float offset=0;
    if(verticalAlignment_==0) offset=static_cast<float>(originalSubtract(originalMultiply(interlinear_,0.5),originalMultiply(originalMultiply(originalUnsigned(static_cast<unsigned>(lines_.size())),interlinear_),0.5)));
    else if(verticalAlignment_==1) offset=static_cast<float>(originalAdd(originalMultiply(geometry_.height,-0.5),originalMultiply(interlinear_,0.5)));
    else if(verticalAlignment_==2) offset=static_cast<float>(originalAdd(originalSubtract(originalMultiply(geometry_.height,0.5),originalMultiply(originalUnsigned(static_cast<unsigned>(lines_.size())),interlinear_)),originalMultiply(interlinear_,0.5)));
    if(offset!=0) for(auto* line:lines_) line->setPosition({line->geometry().x,static_cast<float>(originalAdd(offset,line->geometry().y))});
    textDirty_=false;
}
void MultilineText::loadXML(const XmlNode& node) {
    View::loadXML(node);prototype_->loadXML(node);geometry_.x=prototype_->geometry().x;geometry_.y=prototype_->geometry().y;
    prototype_->setString(WString());prototype_->hide();prototype_->setLocalized(false);
    XmlNode* section=nullptr;text_=normalize(node.section(String("Text"),section) ? section->text():node.attribute(WString(L"String")));
    const auto size=node.attribute(WString(L"size"));if(!size.empty()) { std::wistringstream stream(size.c_str());stream>>geometry_.width;stream.ignore(1);stream>>geometry_.height; }
    read(node,L"Localized",localized_);read(node,L"Interlinear",interlinear_);read(node,L"WrapText",wrap_);
    unsigned alignment=3;read(node,L"VerticalAlign",alignment);verticalAlignment_=alignment;textColor_=Color::fromARGB(prototype_->packedColor());makeLines();
}
}
