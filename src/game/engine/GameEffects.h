#pragma once
#include "FlyingText.h"
#include <cstdint>
namespace farmar::recovered {
class GameConfiguration;
// Actual cGame effect factories. The supplied game View owns every result.
class GameEffects final {
public:
    GameEffects(View& game,FlyingTextContext& context,GameConfiguration& configuration):game_(game),context_(context),configuration_(configuration) {}
    FlyingText* productExpense(unsigned product); // 0040ef00
    FlyingText* moneyExpense(std::uint32_t amount); // 0040cf20
    FlyingText* moneyIncome(std::uint32_t amount); // 0040d450
private:
    View& game_;FlyingTextContext& context_;GameConfiguration& configuration_;
};
}
