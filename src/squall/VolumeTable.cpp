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
}
