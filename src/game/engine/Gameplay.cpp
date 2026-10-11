#include "Gameplay.h"
#include "ProfileProperties.h"
#include "ResourceManager.h"
#include "XmlResource.h"
#include "OriginalArithmetic.h"
#include <cstring>
#include <stdexcept>
namespace farmar::recovered {
namespace {
Gameplay* currentGameplay=nullptr;
std::uint32_t seconds(const GameplayTime& time) noexcept {return (time.hours*60U+time.minutes)*60U+time.seconds;}
std::uint32_t truncate(double value) noexcept {
#if defined(_MSC_VER) && defined(_M_IX86)
    unsigned short savedControl,truncControl;std::int64_t result;
    __asm {
        fnstcw savedControl
        mov ax,savedControl
        or ax,0c00h
        mov truncControl,ax
        fld value
        fldcw truncControl
        fistp result
        fldcw savedControl
    }
    return static_cast<std::uint32_t>(result);
#else
    return static_cast<std::uint32_t>(static_cast<std::int64_t>(value));
#endif
}
std::uint32_t bits(const std::uint32_t& value) noexcept {std::uint32_t result;std::memcpy(&result,&value,4);return result;}
void add(std::int32_t& output,std::uint32_t value) noexcept {
    std::uint32_t result;std::memcpy(&result,&output,4);result+=value;std::memcpy(&output,&result,4);
}
void quantize(std::int32_t& value) noexcept {
    // The original tests >100 first. Its following >1000 branch therefore
    // remains unreachable for these signed integer inputs; preserve the order.
    const double input=value;
    if(input>100) {
        const auto scaled=truncate(originalMultiply(input,0.1f));std::int32_t signedScaled;std::memcpy(&signedScaled,&scaled,4);
        const auto result=truncate(originalMultiply(signedScaled,10.f));std::memcpy(&value,&result,4);
    } else if(input>1000) {
        const auto scaled=truncate(originalMultiply(input,0.01f));std::int32_t signedScaled;std::memcpy(&signedScaled,&scaled,4);
        const auto result=truncate(originalMultiply(signedScaled,100.f));std::memcpy(&value,&result,4);
    }
}
}
GameplayLifetime::GameplayLifetime(Gameplay* value) {currentGameplay=value;}
GameplayLifetime::~GameplayLifetime() {currentGameplay=nullptr;}
Gameplay* Gameplay::instance() noexcept {return currentGameplay;}
Gameplay::Gameplay(ResourceManager& resources,ProfileProperties& profile,GameRandom& random,GameplayEventExecutor& executor)
    :GameplayLifetime(this),profile_(profile),random_(random),executor_(executor) {
    std::unique_ptr<XmlResource> document(resources.getXML(String("Gameplay/Gameplay.xml")));
    // The native XML object is destroyed before reading GameTime/randomizing.
    definitions_.loadXML(document->document());
    document.reset();initialize();
}
Gameplay::Gameplay(const XmlNode& node,ProfileProperties& profile,GameRandom& random,GameplayEventExecutor& executor)
    :GameplayLifetime(this),profile_(profile),random_(random),executor_(executor) {definitions_.loadXML(node);initialize();}
Gameplay::~Gameplay()=default;
void Gameplay::initialize() {
    if(definitions_.noTimedEvents) return;
    const float gameTime=profile_.floats["GameTime"];
    endSeconds_=static_cast<float>(originalUnsigned(seconds(definitions_.endTime)));
    const auto restored=GameplayTime::fromSeconds(gameTime);const auto& events=definitions_.events();
    while(currentEvent_<events.size() && !(restored<events[currentEvent_]->time)) ++currentEvent_;
    if(currentEvent_==events.size()) throw std::runtime_error("\xCD\xE5 \xEC\xEE\xE6\xE5\xF2 \xE1\xFB\xF2\xFC"); // original CP1251 005f9cb0
    if(gameTime==0) {coefficients_.resize(events.size(),0);regenerateCoefficients();applyCoefficients();}
}
void Gameplay::regenerateCoefficients() {
    const auto& events=definitions_.events();
    for(std::size_t i=0;i<coefficients_.size();++i) {
        const float range=events.at(i)->range,negative=-range;const auto sample=random_.next();
        coefficients_[i]=static_cast<float>(originalAdd(originalMultiply(originalMultiply(originalSubtract(range,negative),sample),3.0518509447574615e-05f),negative));
    }
}
void Gameplay::applyCoefficients() {
    const auto& events=definitions_.events();
    for(std::size_t i=0;i<events.size();++i) {
        auto& event=*events[i];
        if(event.type==GameplayEventType::Contract) {
            auto& contract=std::get<ContractEvent>(event.data);const auto coefficient=coefficients_.at(i);
            add(contract.penalty,truncate(originalMultiply(contract.initialPenalty,coefficient)));
            add(contract.bonus,truncate(originalMultiply(contract.initialBonus,coefficient)));
            quantize(contract.penalty);quantize(contract.bonus);
            for(unsigned j=0;j<2;++j) contract.requirements[j].count+=truncate(originalMultiply(originalUnsigned(bits(contract.initialRequirements[j].count)),coefficient));
        } else if(event.type==GameplayEventType::Popularity) {
            auto& popularity=std::get<PopularityEvent>(event.data);
            for(std::size_t j=0;j<popularity.initialValues.size();++j)
                add(popularity.values.at(j).value,truncate(originalMultiply(popularity.initialValues[j].value,coefficients_.at(i))));
        }
    }
}
void Gameplay::restoreCoefficients(const std::vector<float>& values) {coefficients_=values;applyCoefficients();}
void Gameplay::step() {
    if(definitions_.noTimedEvents) return;
    const float gameTime=profile_.floats["GameTime"];const auto& events=definitions_.events();
    auto& event=*events.at(currentEvent_);
    if(originalUnsigned(seconds(event.time))<=gameTime) {
        if(event.type!=GameplayEventType::Contract || executor_.contractAvailable()) executor_.execute(event);
        if(++currentEvent_==events.size()) currentEvent_=0;
    }
    if(endSeconds_<=gameTime) {
        const auto& end=*events.at(definitions_.endEventIndex);
        profile_.floats["GameTime"]=static_cast<float>(originalAdd(originalSubtract(gameTime,endSeconds_),originalUnsigned(seconds(end.time))));
        regenerateCoefficients();applyCoefficients();currentEvent_=definitions_.endEventIndex;
    }
}
}
