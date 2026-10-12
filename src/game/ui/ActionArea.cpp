#include "ActionArea.h"
#include "TutorialManager.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
namespace {
void pointAttribute(const XmlNode& node,const wchar_t* name,Point2& output) {
    const auto text=node.attribute(WString(name));if(text.empty()) return;
    std::wistringstream stream(text.c_str());stream>>output[0];stream.ignore(1);stream>>output[1];
}
}
ActionArea::ActionArea(unsigned type,View* parent,ActionAreaContext& c)
    :ExtendedFieldButton(parent,c.resources,c.settings,c.strings,c.input,c.interaction,[&c]{return c.world.tutorial().active();},c.views,c.controls),
    areaType(type),areaContext_(c),overlay_(new Picture(this,c.resources,c.views,c.controls)) {
    c.world.registerArea(*this);
}
ActionArea::~ActionArea() {
    // 004187a0 only frees clickOffsets and invokes the tooltip/base dtor.
    // The original caller handles Farm/task removal separately.
    clickOffsets.clear();
}
void ActionArea::loadXML(const XmlNode& node) {
    ExtendedFieldButton::loadXML(node);
    overlay_->setSize({geometry().width,geometry().height});overlay_->setPosition({geometry().x,geometry().y});
    overlay_->setDeep(float(originalAdd(deep(),0.001)));
    Color color=Color::fromARGB(0xff202020);const auto text=node.attribute(WString(L"OverlayColor"));
    if(!text.empty()) {unsigned value=0;std::wistringstream stream(text.c_str());stream>>std::hex>>value;color=Color::fromARGB(value);}
    overlay_->setColor(color);overlay_->setRenderFlags(9);overlay_->hide();
    XmlNode* list=nullptr;
    if(node.section(String("ClickOffsetList"),list)) for(unsigned i=0;i<list->children().size();++i) {
        XmlNode* entry=nullptr;if(list->section(i,entry)) {Point2 offset{};pointAttribute(*entry,L"Offset",offset);clickOffsets.push_back(offset);}
    }
    parkingOffsets.fill(Point2{});
    if(node.section(String("ParkingOffsets"),list)) for(unsigned row=0;row<std::min<std::size_t>(4,list->children().size());++row) {
        XmlNode* entries=nullptr;if(!list->section(row,entries)) continue;
        for(unsigned column=0;column<std::min<std::size_t>(12,entries->children().size());++column) {
            XmlNode* entry=nullptr;if(entries->section(column,entry)) pointAttribute(*entry,L"offset",parkingOffsets[row*12+column]);
        }
    }
    setPosition({geometry().x,geometry().y}); // actual virtual call at the end of 0041cd10
}
void ActionArea::onStateChanged(ButtonState state) {
    switch(state) {
    case ButtonState::Disabled:setFrame(disabledFrame);overlay_->hide();setRenderFlags(renderFlags()&~4U);break;
    case ButtonState::Normal:setFrame(normalFrame);overlay_->hide();setRenderFlags(renderFlags()&~4U);break;
    case ButtonState::Hovered:setFrame(hoverFrame);overlay_->show();setRenderFlags(renderFlags()|4U);break;
    case ButtonState::Pressed:setFrame(pressedFrame);overlay_->show();setRenderFlags(renderFlags()|4U);break;
    default:break;
    }
}
void ActionArea::clicked() {
    if(areaContext_.world.tutorial().activeStep(44)) areaContext_.world.tutorial().requestCancel();
    ButtonPrototype::clicked();tipsDirection=1;enqueueClick();
}
void ActionArea::enqueueClick() {
    if(clickOffsets.empty()) throw std::logic_error("Original ActionArea click requires a nonempty offset list");
    if(clickIndex<clickOffsets.size()) {
        const auto& offset=clickOffsets[clickIndex];const Point2 target{float(originalAdd(geometry().x,offset[0])),float(originalAdd(geometry().y,offset[1]))};
        areaContext_.world.queueAreaAction(*this,target);++clickIndex;
    } else {
        const auto& offset=clickOffsets.back();const Point2 target{float(originalAdd(geometry().x,offset[0])),float(originalAdd(geometry().y,offset[1]))};
        ++clickIndex;areaContext_.world.queueAreaAction(*this,target); // native order differs after exhaustion
    }
}
Point2 ActionArea::goal(unsigned index) {
    auto& world=areaContext_.world;
    if(index==2) index=static_cast<unsigned>(world.digBuildingLevel())+2;
    if(index==6) index=static_cast<unsigned>(world.mowBuildingLevel())+6;
    int level=0;
    switch(areaType) {case 2:level=world.warehouseLevel();break;case 4:level=world.waterSourceLevel();break;case 8:level=world.mowBuildingLevel();break;case 16:level=world.digBuildingLevel();break;default:break;}
    const auto& offset=parkingOffsets.at(index+static_cast<unsigned>(level)*12U);
    return {float(originalAdd(offset[0],geometry().x)),float(originalAdd(offset[1],geometry().y))};
}
Point2 ActionArea::areaExtent() const noexcept {return {geometry().width,areaExtentHeight};}
}
