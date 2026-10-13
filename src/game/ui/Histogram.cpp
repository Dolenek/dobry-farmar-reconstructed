#include "Histogram.h"
#include "ProgressBar.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
namespace {
template<class T> void attribute(const XmlNode& node,const wchar_t* name,T& value) {
    const auto text=node.attribute(WString(name));if(!text.empty()) {std::wistringstream input(text.c_str());input>>value;}
}
Point2 point(const XmlNode& node,const wchar_t* name) {
    Point2 value{};const auto text=node.attribute(WString(name));
    std::wistringstream input(text.c_str());input>>value[0];input.ignore(1);input>>value[1];return value;
}
}
void Histogram::loadXML(const XmlNode& node) {
    for(auto* bar:bars_) delete bar;bars_.clear();
    Control::loadXML(node);attribute(node,L"Threshold",threshold);
    unsigned count=0;attribute(node,L"BarCount",count);
    const auto step=point(node,L"Step");auto origin=point(node,L"position");
    XmlNode* definition=nullptr;
    if(!node.section(String("Bar"),definition)) throw std::runtime_error("Histogram requires Bar XML section");
    const auto offset=point(*definition,L"position");
    origin[0]=float(originalAdd(offset[0],origin[0]));origin[1]=float(originalAdd(offset[1],origin[1]));
    bars_.resize(count,nullptr);
    for(unsigned i=0;i<count;++i) {
        auto* bar=new ProgressBar(this,resources_,registry_,controls_);bar->loadXML(*definition);
        const auto index=originalUnsigned(i);
        const float dy=float(originalMultiply(step[1],index)); // original spill before adding root Y
        bar->setPosition({float(originalAdd(originalMultiply(index,step[0]),origin[0])),float(originalAdd(origin[1],dy))});
        bar->show();bars_[i]=bar;
    }
}
void Histogram::applyValues(const std::vector<float>& values) {
    float maximum=0;for(const auto value:values) if(maximum<value) maximum=value;
    auto next=values.begin();
    for(auto* bar:bars_) {
        bar->maximum=maximum;bar->updateLid();
        bar->current=next==values.end() ? 0:*next++;bar->updateLid();
    }
}
void Histogram::setMaximum(float value) {for(auto* bar:bars_) {bar->maximum=value;bar->updateLid();}}
}
