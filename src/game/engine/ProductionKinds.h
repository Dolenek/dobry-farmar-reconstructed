#pragma once
#include <array>
#include <cstdint>
namespace farmar::recovered {
// Values set by the eleven original derived constructors after 0049e9f0.
// Direct input is a raw product index; manufactured input/output use +8.
struct ProductionKindDefinition {
    const char* xml;
    std::int32_t directInput,manufacturedInput;
    unsigned manufacturedOutput;
    unsigned inputProduct() const noexcept {return directInput==-1 ? unsigned(manufacturedInput+8):unsigned(directInput);}
    unsigned outputProduct() const noexcept {return manufacturedOutput+8;}
};
inline constexpr std::array<ProductionKindDefinition,11> productionKinds={{{"Manufacture/SheepFold.xml",3,-1,1},
    {"Manufacture/HenHouse.xml",7,-1,0},{"Manufacture/CowShed.xml",0,-1,2},{"Manufacture/PigPen.xml",1,-1,3},
    {"Manufacture/CheeseDairy.xml",-1,5,4},{"Manufacture/Creamery.xml",-1,2,5},{"Manufacture/Weavery.xml",-1,1,6},
    {"Manufacture/Incubator.xml",-1,0,7},{"Manufacture/TextileDep.xml",-1,6,8},{"Manufacture/Bakery.xml",-1,9,10},
    {"Manufacture/Mill.xml",7,-1,9}}};
inline const ProductionKindDefinition& productionKind(unsigned type) {if(type>=productionKinds.size()) throw "unknown building";return productionKinds[type];}
}
