// Read-only observer metadata. No game construction or original-code calls.
#include "FarmField.h"
#include <cstddef>
#include <iostream>
using namespace farmar::recovered;
int main() {
    static_assert(sizeof(void*)==4,"Observer requires Win32 release metadata");
    std::cout<<"{\"object_size\":"<<sizeof(FarmField)
        <<",\"id\":"<<offsetof(FarmField,id)
        <<",\"upgrade_index\":"<<offsetof(FarmField,upgradeIndex)
        <<",\"active\":"<<offsetof(FarmField,active)<<"}\n";
}
