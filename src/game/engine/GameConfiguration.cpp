#include "GameConfiguration.h"
#include "ResourceManager.h"
#include "Resource.h"
#include "Sound.h"
#include "XmlDocument.h"
#include "StringConversion.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
namespace farmar::recovered {
std::uint32_t GameConfiguration::upgradeCost(std::uint32_t item,std::uint32_t level) const {
    if(level>3) throw std::runtime_error("bad level");
    if(item>14) throw std::runtime_error("bad upgrade item");
    return costs[item][level];
}
namespace {
template<class T> void attribute(const XmlNode& node,const wchar_t* name,T& output) {
    const auto text=node.attribute(WString(name));
    if(!text.empty()) { std::wistringstream stream(text.c_str());stream>>output; }
}
}
void ProductionInfo::loadXML(const XmlNode& node,ResourceManager& resources) {
    attribute(node,L"CreationTime",creationTime);attribute(node,L"DieTime",dieTime);
    attribute(node,L"PriceRadius",priceRadius);attribute(node,L"GoodAttPriceRadius",goodAttPriceRadius);
    attribute(node,L"MaxSellFrequency",maxSellFrequency);
    XmlNode* child=nullptr;
    if(node.section(String("Icon"),child)) icon.loadXML(*child,resources);
    if(node.section(String("IconLarge"),child)) iconLarge.loadXML(*child,resources);
}
void SeedInfo::loadXML(const XmlNode& node,ResourceManager& resources) {
    attribute(node,L"Price",price);XmlNode* child=nullptr;
    if(node.section(String("Icon"),child)) icon.loadXML(*child,resources);
}
GameConfiguration::~GameConfiguration() {releaseUpgradeSound();}
void GameConfiguration::releaseUpgradeSound() {if(upgradeSound_) {upgradeSound_->releaseReference();upgradeSound_=nullptr;}}
void GameConfiguration::loadXML(const XmlNode& node) {
    XmlNode *section=nullptr,*child=nullptr;
    if(node.section(String("ProductionInfo"),section)) {
        // Original indexes the fixed array without a count guard; Id is ignored.
        // Reject out-of-bounds data instead of inventing clamping semantics.
        if(section->children().size()>products.size()) throw std::runtime_error("cGame ProductionInfo exceeds its original 19 records");
        for(std::uint32_t i=0;i<section->children().size();++i)
            if(section->section(i,child)) products[i].loadXML(*child,resources_);
    }
    if(node.section(String("Costs"),section)) {
        const auto count=std::min(section->children().size(),costs.size());
        for(std::uint32_t i=0;i<count;++i) if(section->section(i,child)) {
            const auto levels=std::min<std::size_t>(child->children().size(),4);
            for(std::uint32_t j=0;j<levels;++j) {
                XmlNode* level=nullptr;
                if(child->section(String(std::string("Up")+std::to_string(j)),level)) {
                    std::wistringstream stream(level->text().c_str());stream>>costs[i][j];
                }
            }
        }
    }
    if(node.section(String("Seeds"),section)) {
        if(section->children().size()>seeds.size()) throw std::runtime_error("cGame Seeds exceeds its original 8 records");
        for(std::uint32_t i=0;i<section->children().size();++i)
            if(section->section(i,child)) seeds[i].loadXML(*child,resources_);
    }
    if(node.section(String("UpgradeStrings"),section)) {
        const auto count=std::min(section->children().size(),upgradeStrings.size());
        for(std::uint32_t i=0;i<count;++i) if(section->section(i,child)) upgradeStrings[i]=child->attribute(WString(L"string"));
    }
    const auto sound=toNarrow(node.attribute(WString(L"UpgradeSound")));
    // Native replacement retains earlier sound references; release only the
    // current pointer in destruction, just as 00410830 does.
    if(!sound.empty()) upgradeSound_=resources_.getSound(sound);
}
}
