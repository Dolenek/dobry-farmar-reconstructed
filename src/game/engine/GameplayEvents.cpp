#include "GameplayEvents.h"
#include "XmlDocument.h"
#include <algorithm>
#include <cstring>
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
namespace {
template<class T> void attribute(const XmlNode& node,const wchar_t* name,T& output) {
    const auto text=node.attribute(WString(name));if(!text.empty()) { std::wistringstream stream(text.c_str());stream>>output; }
}
void sectionText(const XmlNode& node,const char* name,WString& output) {
    XmlNode* child=nullptr;if(node.section(String(name),child)) output=child->text();
}
}
GameplayEvent::GameplayEvent(GameplayEventType value):type(value) {
    switch(type) {
    case GameplayEventType::DepotUpgrade:data.emplace<DepotUpgradeEvent>();break;
    case GameplayEventType::SeedAvailable:data.emplace<SeedAvailableEvent>();break;
    case GameplayEventType::Money:data.emplace<RandomValueEvent>().value=0;break;
    case GameplayEventType::DestroyField:case GameplayEventType::DestroyDepot:data.emplace<RandomValueEvent>();break;
    case GameplayEventType::Contract:data.emplace<ContractEvent>();break;
    case GameplayEventType::Popularity:data.emplace<PopularityEvent>();break;
    case GameplayEventType::FieldUpgrade:data.emplace<FieldUpgradeEvent>();break;
    case GameplayEventType::Trigger:data.emplace<TriggerEvent>();break;
    case GameplayEventType::End:data.emplace<EndEvent>();break;
    default:throw std::runtime_error("Unsupported original Gameplay event type");
    }
}
void GameplayEvent::loadXML(const XmlNode& node) {
    time.loadXML(node.attribute(WString(L"Time")));sectionText(node,"Note",note);
    XmlNode* child=nullptr;const bool hasData=node.section(String("Data"),child);
    if(hasData) attribute(*child,L"Range",range); // 004b1240 shared base
    if(type==GameplayEventType::Trigger) std::get<TriggerEvent>(data).wareLevel=-1;
    if(hasData) switch(type) {
    case GameplayEventType::DepotUpgrade: {
        auto& value=std::get<DepotUpgradeEvent>(data);attribute(*child,L"Upgrade",value.upgrade);attribute(*child,L"DepotType",value.depotType);break;
    }
    case GameplayEventType::SeedAvailable:attribute(*child,L"SeedType",std::get<SeedAvailableEvent>(data).seedType);break;
    case GameplayEventType::Money:case GameplayEventType::DestroyField:case GameplayEventType::DestroyDepot: {
        auto& value=std::get<RandomValueEvent>(data);attribute(*child,L"RandomFactor",value.randomFactor);attribute(*child,L"Value",value.value);break;
    }
    case GameplayEventType::Contract: {
        auto& value=std::get<ContractEvent>(data);value.winTime.loadXML(child->attribute(WString(L"WinTime")));
        attribute(*child,L"Bonus",value.bonus);attribute(*child,L"Penalty",value.penalty);attribute(*child,L"RandomFactor",value.randomFactor);
        XmlNode* requirements=nullptr;
        if(child->section(String("Requirements"),requirements)) {
            const auto count=std::min<std::size_t>(requirements->children().size(),2);
            for(std::uint32_t i=0;i<count;++i) { XmlNode* requirement=nullptr;if(requirements->section(i,requirement)) {
                attribute(*requirement,L"ProductType",value.requirements[i].productType);attribute(*requirement,L"Count",value.requirements[i].count);
                attribute(*requirement,L"ReqType",value.requirements[i].reqType);
            } }
        }
        sectionText(*child,"WinNote",value.winNote);sectionText(*child,"LosNote",value.lossNote);
        value.letterTime=GameplayTime::fromSeconds(60);value.letterTime.loadXML(child->attribute(WString(L"LetterTime")));break;
    }
    case GameplayEventType::Popularity: {
        auto& value=std::get<PopularityEvent>(data);
        const auto count=std::min<std::size_t>(child->children().size(),19);
        for(std::uint32_t i=0;i<count;++i) { XmlNode* popularity=nullptr;if(child->section(i,popularity)) {
            PopularityValue entry;attribute(*popularity,L"ProductType",entry.productType);attribute(*popularity,L"Value",entry.value);value.values.push_back(entry);
        } }break;
    }
    case GameplayEventType::FieldUpgrade:attribute(*child,L"UpgradeIndex",std::get<FieldUpgradeEvent>(data).upgradeIndex);break;
    case GameplayEventType::Trigger: {
        auto& value=std::get<TriggerEvent>(data);attribute(*child,L"Money",value.money);attribute(*child,L"WareLevel",value.wareLevel);break;
    }
    case GameplayEventType::End:break;
    }
    if(type==GameplayEventType::Contract) {
        auto& value=std::get<ContractEvent>(data);value.initialBonus=value.bonus;value.initialPenalty=value.penalty;
        // Original copies both 12-byte records, including unspecified scalar
        // object representations when an XML definition supplies only one.
        std::memcpy(value.initialRequirements.data(),value.requirements.data(),sizeof(value.requirements));
    }
    if(type==GameplayEventType::Popularity) {auto& value=std::get<PopularityEvent>(data);value.initialValues=value.values;}
}
void GameplayEventDefinitions::loadXML(const XmlNode& node) {
    XmlNode* section=nullptr;if(!node.section(String("Events"),section)) return;
    for(std::uint32_t i=0;i<section->children().size();++i) {
        XmlNode* child=nullptr;if(!section->section(i,child)) continue;
        std::uint32_t type;attribute(*child,L"Type",type);if(type>9) continue;
        auto event=std::make_unique<GameplayEvent>(static_cast<GameplayEventType>(type));event->loadXML(*child);
        // Equal times stay in XML order: the original inserts only before the
        // first strictly later item, rather than using an unstable sort.
        const auto position=std::find_if(events_.begin(),events_.end(),[&](const auto& current) {return event->time<current->time;});
        events_.insert(position,std::move(event));
    }
    if(!events_.empty()) {
        endTime=events_.back()->time;noTimedEvents=endTime==GameplayTime{};
        for(std::uint32_t i=0;i<events_.size();++i) {events_[i]->index=i;if(events_[i]->type==GameplayEventType::End) endEventIndex=i;}
    }
}
}
