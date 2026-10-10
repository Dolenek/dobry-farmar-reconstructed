#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "EditBox.h"
#include "Font.h"
#include "Texture.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include <sstream>
namespace farmar::recovered {
namespace {
void readFloat(const XmlNode& node,const wchar_t* name,float& output) {
    const auto value=node.attribute(WString(name));if(!value.empty()) { std::wistringstream stream(value.c_str());stream>>output; }
}
}
EditBox::EditBox(View* parent,ResourceManager& resources,Settings& settings,StringManager& strings,InputBindings& input,ViewRegistry& views,ControlRegistry& controls)
    :TextLine(parent,resources,settings,strings,views,controls),input_(input) {
    localized_=false;keyboardLanguage=[] { return unsigned(reinterpret_cast<std::uintptr_t>(GetKeyboardLayout(0))&0xffff); };
}
EditBox::~EditBox() { onDisabled(); }
void EditBox::dispatch(wchar_t value,void* self) { static_cast<EditBox*>(self)->character(value); }
void EditBox::onEnabled() { View::onEnabled();input_.bindKeyboard(&dispatch,this); }
void EditBox::onDisabled() { TextLine::onDisabled();input_.unbindKeyboard(&dispatch,this); }
void EditBox::character(wchar_t value) {
    if(confirm && !confirm(&value)) return;
    const bool previous=rightInput_;rightInput_=false;
    if(rightToLeft_ && keyboardLanguage()==0x040d) {
        if(value==8) rightInput_=previous;else if(static_cast<unsigned short>(value)>=0xff) rightInput_=true;
    }
    if(value==8) {
        const auto count=text_.empty() ? text_.size():text_.size()-1;
        setString(WString(text_.view().substr(0,count)));
    } else if(value!=10 && value!=13) {
        const float imageWidth=static_cast<float>(originalMultiply(static_cast<int>(textTexture_->allocatedWidth()),textTexture_->scaleU()));
        double advance=0;bool available=false;
        if(value==L' ') { advance=originalMultiply(spaceWidth_,multiplier_);available=true; }
        else { FontGlyph glyph{};available=font_->getGlyph(static_cast<unsigned short>(value),glyph);if(available) advance=originalMultiply(glyph.advance,imageWidth); }
        // The original reads cached width and uses unscaled glyph advance.
        // Equality and unordered values fail its strict width comparison.
        if(available && originalAdd(originalAdd(caretAdvance_,width_),advance)<maximumWidth_) {
            WString next(text_);next.append(&value,1);setString(next);
        }
    }
    if(changed) changed(this); // Also called for Enter, rejected glyphs and width limits.
}
void EditBox::loadXML(const XmlNode& node) {
    TextLine::loadXML(node);readFloat(node,L"MaxLength",maximumWidth_);readFloat(node,L"BlinkingPeriod",blinkingPeriod_);
    auto* texture=resources_.getTexture(textureName_);auto* font=resources_.getFont(fontName_);
    const float imageHeight=static_cast<float>(originalMultiply(static_cast<int>(texture->allocatedHeight()),texture->scaleV()));
    const float imageWidth=static_cast<float>(originalMultiply(static_cast<int>(texture->allocatedWidth()),texture->scaleU()));
    if(!caret_) caret_=new Letter(this,resources_,registry_,controls_);
    FontGlyph glyph{};
    if(font->getGlyph(0x5f,glyph)) {
        const float v0=static_cast<float>(originalSubtract(1,glyph.v0)),v1=static_cast<float>(originalSubtract(1,glyph.v1));
        caretAdvance_=static_cast<float>(originalMultiply(imageWidth,glyph.advance));
        caret_->setUV({glyph.u0,v0,glyph.u1,v1});caret_->setTextureName(textureName_);
        const float rawHeight=static_cast<float>(originalMultiply(originalSubtract(v1,v0),imageHeight));
        const float height=static_cast<float>(originalMultiply(rawHeight,multiplier_));
        const float width=static_cast<float>(originalMultiply(originalMultiply(originalSubtract(glyph.u1,glyph.u0),imageWidth),multiplier_));
        caret_->setSize({width,height});caret_->setDeep(deep_+20.f);caret_->setAlpha(alpha());caret_->setPackedColor(packedColor_);
        caret_->setRenderFlags(flags_);caret_->show();caret_->enable();
    }
    texture->releaseReference();font->releaseReference();
}
void EditBox::enabledStep() {
    const double total=originalAdd(uiElapsedTime() ? uiElapsedTime()():0,blinkTime_);blinkTime_=static_cast<float>(total);
    if(total>blinkingPeriod_) { blinkTime_=0;caretShown_=!caretShown_;if(caretShown_) caret_->show();else caret_->hide(); }
    TextLine::enabledStep();
}
void EditBox::process() { Control::process();caret_->markTransformDirty(); }
void EditBox::makeLetters() {
    TextLine::makeLetters();if(!caret_) return;
    double offset=0;
    if(alignment_==0) offset=rightInput_ ? originalSubtract(originalMultiply(width_,-0.5),originalMultiply(caretAdvance_,0.5)):originalMultiply(originalAdd(caretAdvance_,width_),0.5);
    else if(alignment_==1) offset=rightInput_ ? originalSubtract(-double(width_),originalMultiply(caretAdvance_,0.5)):originalAdd(originalMultiply(caretAdvance_,0.5),width_);
    else if(alignment_==2) offset=originalMultiply(caretAdvance_,rightInput_ ? -0.5:0.5);
    else return;
    caret_->setPosition({static_cast<float>(originalAdd(offset,geometry_.x)),geometry_.y});
}
}
