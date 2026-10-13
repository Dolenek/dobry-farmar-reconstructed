#include "GameEffects.h"
#include "GameConfiguration.h"
#include <string>
namespace farmar::recovered {
FlyingText* GameEffects::productExpense(unsigned product) {
    auto* effect=new FlyingText(&game_,context_,String("UI/FlyProductExpence.xml"));
    effect->setString(WString(L"1"));effect->setIconAppearance(configuration_.products.at(product).icon);return effect;
}
FlyingText* GameEffects::moneyExpense(std::uint32_t amount) {
    auto* effect=new FlyingText(&game_,context_,String("UI/FlyTextExpence.xml"));effect->setString(WString(std::to_wstring(amount)));return effect;
}
FlyingText* GameEffects::moneyIncome(std::uint32_t amount) {
    auto* effect=new FlyingText(&game_,context_,String("UI/FlyTextIncome.xml"));effect->setString(WString(std::to_wstring(amount)));return effect;
}
}
