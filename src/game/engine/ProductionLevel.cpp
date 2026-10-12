#include "ProductionLevel.h"
#include "Control.h"
#include "XmlDocument.h"
#include "StringConversion.h"
#include <sstream>
namespace farmar::recovered {
void ProductionLevel::loadXML(const XmlNode& node,ResourceManager& resources) {
    const auto time=node.attribute(WString(L"MaxProductionTime"));if(!time.empty()) {std::wistringstream stream(time.c_str());stream>>maximumProductionTime;}
    const auto count=node.attribute(WString(L"ProductionNum"));if(!count.empty()) {std::wistringstream stream(count.c_str());stream>>productionCount;}
    appearance.loadXML(node,resources);hovered.copyAppearance(appearance);
    maskName=toNarrow(node.attribute(WString(L"texture_mask")));
    XmlNode* section=nullptr;if(node.section(String("Hovered"),section)) hovered.loadXML(*section,resources);
}
}
