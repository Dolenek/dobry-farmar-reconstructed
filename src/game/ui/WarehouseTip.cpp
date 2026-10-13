#include "WarehouseTip.h"
#include "Warehouse.h"
#include "GameConfiguration.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
namespace farmar::recovered {
namespace {
WarehouseTip* warehouseTipSingleton=nullptr;
void child(View& control,const XmlNode& node,const char* name) {XmlNode* section=nullptr;if(node.section(String(name),section)) control.loadXML(*section);}
}
WarehouseTipLifetime::WarehouseTipLifetime(WarehouseTip* self) {if(warehouseTipSingleton) throw "Object already exist.";warehouseTipSingleton=self;}
WarehouseTipLifetime::~WarehouseTipLifetime() {warehouseTipSingleton=nullptr;}
WarehouseTip* WarehouseTip::instance() noexcept {return warehouseTipSingleton;}
WarehouseTip::WarehouseTip(View* parent,WarehouseTipContext& c)
    :MovableUnion(parent,c.resources,c.views,c.controls),WarehouseTipLifetime(this),tipContext_(c) {
    loadXML(*c.resources.loadSettingsXML(String("UI/gameUI/WareDetail.xml")));
}
void WarehouseTip::loadRow(WarehouseTipRow& row,const XmlNode& node) {
    child(*row.icon,node,"Icon");child(*row.quantity,node,"Text");
    const auto type=node.attribute(WString(L"IconType"));if(!type.empty()) {std::wistringstream stream(type.c_str());stream>>row.iconType;}
}
void WarehouseTip::loadXML(const XmlNode& node) {
    MovableUnion::loadXML(node);
    const auto duration=node.attribute(WString(L"VisibleTime"));if(!duration.empty()) {std::wistringstream stream(duration.c_str());stream>>visibleTime;}
    XmlNode* entries=nullptr;if(node.section(String("ProductInfo"),entries)) {
        productCount=static_cast<unsigned>((std::min)(entries->children().size(),std::size_t(19)));
        for(unsigned i=0;i<productCount;++i) {
            XmlNode* entry=nullptr;if(!entries->section(i,entry)) continue;
            auto& row=rows[i];row.icon=new Picture(this,tipContext_.resources,tipContext_.views,tipContext_.controls);
            row.quantity=new TextLine(this,tipContext_.resources,tipContext_.settings,tipContext_.strings,tipContext_.views,tipContext_.controls);
            loadRow(row,*entry); // native reload overwrites row pointers; old children remain View-owned
        }
    }
    setPosition(hiddenPlace);hide();
}
void WarehouseTip::refresh() {
    if(!opened) return;unsigned visible=0;
    for(unsigned product=0;product<productCount;++product) {
        const auto count=Warehouse::instance()->goods(product);if(!count) continue;
        auto& row=rows[visible];row.quantity->setString(WString(std::wstring(L"x")+std::to_wstring(count)));
        const auto& info=tipContext_.configuration.products.at(product);
        row.icon->copyAppearance(row.iconType ? info.iconLarge:info.icon);row.icon->markTransformDirty();
        row.icon->show();row.quantity->show();++visible;
    }
    for(;visible<productCount;++visible) {rows[visible].icon->hide();rows[visible].quantity->hide();}
}
void WarehouseTip::open() {remainingVisible=1000000000.0f;opened=true;refresh();move(2,{geometry().x,geometry().y},originalPosition);}
void WarehouseTip::armHide() {remainingVisible=visibleTime;}
void WarehouseTip::close() {opened=false;move(1,{geometry().x,geometry().y},hiddenPlace);}
void WarehouseTip::absoluteStep() {
    MovableUnion::absoluteStep();if(!uiElapsedTime()) throw std::logic_error("WarehouseTip timer is not bound");
    const float elapsed=uiElapsedTime()();const auto next=originalSubtract(remainingVisible,elapsed);remainingVisible=float(next);
    if(next<=0) close(); // compare original unspilled subtraction, after storing its float
}
}
