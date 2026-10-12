#include "ProductionControls.h"
#include "XmlDocument.h"
namespace farmar::recovered {
ProductionStartStopButton::ProductionStartStopButton(View* parent,ProductionBuilding* owner,ResourceManager& resources,InputBindings& input,ButtonInteraction& interaction,ViewRegistry& views,ControlRegistry& controls)
    :AnimationButton(parent,resources,input,interaction,views,controls),owner_(owner) {}
void ProductionStartStopButton::loadXML(const XmlNode& node) {
    AnimationButton::loadXML(node);XmlNode* section=nullptr;
    if(node.section(String("StateStopSign"),section)) signs_[0].loadXML(*section,resources_);
    if(node.section(String("StateStartSign"),section)) signs_[1].loadXML(*section,resources_);
}
void ProductionStartStopButton::setStoppedSign(unsigned char stopped) {copyAppearance(signs_[stopped==1 ? 1:0]);}
}
