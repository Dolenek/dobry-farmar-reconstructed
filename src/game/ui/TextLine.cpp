#include "TextLine.h"
#include "LetterSprite.h"
#include "ResourceManager.h"
#include "Texture.h"
#include "Settings.h"
#include "StringManager.h"
#include "XmlDocument.h"
#include "StringConversion.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
namespace {
template<class T> void attribute(const XmlNode& node,const wchar_t* name,T& value) {
    const auto text=node.attribute(WString(name)); if(!text.empty()) { std::wistringstream stream(text.c_str()); stream>>value; }
}
bool asciiRun(wchar_t c) { return (c>=L'0' && c<=L'9') || (c>=L'A' && c<=L'Z') || (c>=L'a' && c<=L'z'); }
}
WString reverseTextDirection(const WString& text) {
    std::wstring result(text.c_str(),text.size()); std::reverse(result.begin(),result.end());
    for(auto current=result.begin();current!=result.end();) {
        if(!asciiRun(*current)) { ++current; continue; }
        auto end=current; while(end!=result.end() && asciiRun(*end)) ++end; std::reverse(current,end); current=end;
    }
    return WString(std::wstring_view(result));
}
void Letter::setUV(const std::array<float,4>& value) { if(uv_!=value) { uv_=value; dirty_=true; } }
void Letter::setPackedColor(unsigned value) { if(packedColor_!=value) { packedColor_=value; dirty_=true; } }
void Letter::updateTransform() {
    if(!dirty_) return;
    TextLetter letter{{uv_[0],uv_[1],uv_[2],uv_[3],0},geometry_.x,geometry_.y,geometry_.width,geometry_.height,deep_,0};
    updateLetterSprite(first_,letter,packedColor_,parent() ? rootTransparency():1,alpha()); dirty_=false;
}
TextLine::TextLine(View* parent,ResourceManager& resources,Settings& settings,StringManager& strings,ViewRegistry& views,ControlRegistry& controls)
    : Control(parent,resources,views,controls),strings_(strings),settings_(settings) {
    settings.get(String("game"),String("RightToLeftEnable"),rightToLeft_);
}
TextLine::TextLine(const TextLine& source):Control(source),strings_(source.strings_),settings_(source.settings_),fontName_(source.fontName_),textureName_(source.textureName_),localized_(source.localized_),spaceWidth_(source.spaceWidth_),multiplier_(source.multiplier_),width_(source.width_),height_(source.height_),packedColor_(source.packedColor_),alignment_(source.alignment_),font_(source.font_),textTexture_(source.font_ ? source.textTexture_:nullptr) {
    if(font_) font_->addReference();if(textTexture_) textTexture_->addReference();
    settings_.get(String("game"),String("RightToLeftEnable"),rightToLeft_);setString(source.text_);
}
TextLine::~TextLine() {
    callbacks_.clear();
    if(font_) font_->releaseReference();
    if(textTexture_) textTexture_->releaseReference();
    // Original list stores child pointers only. The base View destructor owns
    // the actual Letter objects and releases their texture references afterward.
    letters_.clear();
}
void TextLine::onShown() {
    if(state_.enabled && state_.parentEnabled) onEnabled();
    makeLetters(); View::onShown();
}
void TextLine::onDisabled() { if(textDirty_) makeLetters(); View::onDisabled(); }
void TextLine::enabledStep() { View::enabledStep(); if(textDirty_) makeLetters(); }
void TextLine::setDeep(float value) { Control::setDeep(value); controls_.erase(this); }
void TextLine::setAlpha(float value) { Control::setAlpha(value); for(auto* letter:letters_) letter->setAlpha(value); }
void TextLine::setTexture(const String& name) {
    if(textureName_.view()==name.view()) return;
    textureName_=name; if(textTexture_) textTexture_->releaseReference();
    textTexture_=resources_.getTexture(textureName_); textDirty_=true;
}
void TextLine::setPackedColor(unsigned value) { if(packedColor_!=value) { packedColor_=value; textDirty_=true; } }
void TextLine::setMultiplier(float value) { if(multiplier_!=value) { multiplier_=value; textDirty_=true; } }
void TextLine::setAlignment(unsigned value) { if(alignment_!=value) { alignment_=value; textDirty_=true; } }
void TextLine::setString(const WString& text) {
    if(text_.view()==text.view()) return;
    const auto& translated=localized_ ? strings_.getString(text):text;
    if(!font_) throw std::logic_error("TextLine font resource is not loaded");
    unsigned required=0;
    for(unsigned i=0;i<translated.size();++i) {
        FontGlyph glyph{};
        if(translated.data()[i]!=L' ' && font_->getGlyph(static_cast<unsigned short>(translated.data()[i]),glyph)) ++required;
    }
    // 004dd030 removes from the front of the original list, preserving its tail.
    while(letters_.size()>required) { auto* letter=letters_.front(); letters_.erase(letters_.begin()); delete letter; }
    text_=text;
    while(letters_.size()<required) letters_.push_back(new Letter(this,resources_,registry_,controls_));
    textDirty_=true;
    if(state_.shown && state_.parentShown && (!state_.enabled || !state_.parentEnabled)) makeLetters();
}
void TextLine::makeLetters() {
    if(!dirty_ || textureName_.empty() || fontName_.empty()) return;
    const auto& translated=localized_ ? strings_.getString(text_):text_;
    const auto displayed=rightToLeft_ ? reverseTextDirection(translated):translated;
    TextLinePlacement placement{geometry_.x,geometry_.y,deep_,spaceWidth_,multiplier_,1};
    const auto layout=layoutTextLine(*font_,std::wstring_view(displayed.c_str(),displayed.size()),textTexture_->allocatedWidth(),textTexture_->allocatedHeight(),
        textTexture_->scaleU(),textTexture_->scaleV(),placement,height_);
    if(layout.letters.size()>letters_.size()) throw std::logic_error("TextLine letter pool does not cover localized string");
    for(unsigned i=0;i<layout.letters.size();++i) {
        const auto& data=layout.letters[i]; auto* letter=letters_[i];
        letter->setUV({data.glyph.u0,data.glyph.v0,data.glyph.u1,data.glyph.v1}); letter->setTextureName(textureName_);
        letter->setSize({data.width,data.height}); letter->setPosition({data.x,data.y}); letter->setDeep(data.depth);
        letter->setAlpha(alpha()); letter->setPackedColor(packedColor_); letter->setRenderFlags(flags_); letter->show(); letter->enable();
    }
    width_=layout.width; height_=layout.height;
    const float offset=alignment_==0 ? static_cast<float>(double(width_)*-0.5):alignment_==2 ? -width_:0;
    for(auto* letter:letters_) letter->setPosition({static_cast<float>(double(letter->geometry().x)+offset),letter->geometry().y+0.f});
    textDirty_=false;
}
float TextLine::textWidth() { if(textDirty_) makeLetters(); return width_; }
float TextLine::textHeight() { if(textDirty_) makeLetters(); return height_; }
bool TextLine::isCharAvailable(wchar_t character) const {
    if(!font_) throw std::logic_error("TextLine font resource is not loaded");
    return font_->isCharAvailable(static_cast<unsigned short>(character));
}
void TextLine::loadXML(const XmlNode& node) {
    Control::loadXML(node);
    const auto text=node.attribute(WString(L"String"));
    textureName_=toNarrow(node.attribute(WString(L"Texture"))); fontName_=toNarrow(node.attribute(WString(L"Font")));
    attribute(node,L"SpaceWidth",spaceWidth_); attribute(node,L"Multiplier",multiplier_); attribute(node,L"Localized",localized_);
    if(textTexture_) textTexture_->releaseReference(); textTexture_=resources_.getTexture(textureName_);
    if(font_) font_->releaseReference(); font_=resources_.getFont(fontName_);
    unsigned alignment=0; attribute(node,L"Align",alignment); alignment_=alignment;
    const auto color=node.attribute(WString(L"Color"));
    if(!color.empty()) { unsigned value=0; std::wistringstream stream(color.c_str()); stream>>std::hex>>value; packedColor_=Color::fromARGB(value).toARGB(); }
    setString(text); makeLetters();
}
}
