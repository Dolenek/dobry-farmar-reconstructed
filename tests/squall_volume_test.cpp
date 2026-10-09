// Expected values computed independently with 70-digit decimal arithmetic from PE constants.
#include "VolumeTable.h"
#include <array>
#include <iostream>

int main() {
    const std::array<int, 100> expected = {
        -10000, -9625, -9281, -8961, -8664, -8386, -8124, -7878, -7644, -7423,
        -7212, -7011, -6819, -6635, -6458, -6289, -6126, -5968, -5816, -5670,
        -5528, -5391, -5257, -5128, -5003, -4881, -4762, -4647, -4534, -4424,
        -4317, -4213, -4111, -4012, -3914, -3819, -3726, -3635, -3545, -3458,
        -3372, -3288, -3205, -3124, -3045, -2967, -2890, -2814, -2740, -2668,
        -2596, -2525, -2456, -2388, -2321, -2254, -2189, -2125, -2062, -1999,
        -1938, -1877, -1817, -1758, -1700, -1643, -1586, -1530, -1475, -1420,
        -1366, -1313, -1260, -1209, -1157, -1106, -1056, -1007, -958, -909,
        -861, -814, -767, -721, -675, -629, -584, -540, -496, -452,
        -409, -366, -324, -282, -241, -199, -159, -118, -78, -39,
    };
    const auto actual = farmar::recovered::makeSquallVolumeTable();
    for (std::size_t i = 0; i < expected.size(); ++i) {
        if (actual[i] != expected[i]) {
            std::cerr << "Volume index " << i << ": expected " << expected[i]
                      << ", got " << actual[i] << "\n";
            return 1;
        }
    }
    return 0;
}
