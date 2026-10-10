#include "VolumeTable.h"
#include <cmath>

namespace farmar::recovered {
std::array<int, 100> makeSquallVolumeTable() {
    std::array<int, 100> table{};
    table[0] = -10000;
    for (int index = 1; index < 100; ++index) {
        // PE constants: float at 10048194, float at 10048190, double at 10048188.
        // The original x87 evaluates these with extended precision and truncates.
        const long double input = static_cast<long double>(0.009F) * index
                                + static_cast<long double>(0.1F);
        table[index] = static_cast<int>(10000.0L * std::log10(input));
    }
    return table;
}
int squallVolumeFromAttenuation(int attenuation) {
    // Original double constants at 100481b0/b8/a8/a0/98, respectively.
    return static_cast<int>((std::pow(10.0,attenuation*0.0001)-0.10000000149011612)
                            *100.0*1.1111111405455043);
}
}
