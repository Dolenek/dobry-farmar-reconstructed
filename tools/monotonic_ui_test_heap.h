#pragma once
// Test-only allocator: original equal-depth UI ordering uses raw addresses.
// Two independently constructed hierarchies must have matching allocation
// ranks to compare their overlapping pixels. This heap preserves allocation
// order without modifying either depth comparator or render order.
#include <windows.h>
#include <new>
#include <cstddef>
namespace ui_test_heap {
inline unsigned char* base=nullptr;
inline std::size_t used=0,committed=0;
// The connected Market/Warehouse/Manufacture comparison includes 80 complete
// frames and repeated XML ownership. Keep all allocation ranks across that
// whole run; no address reuse or ordering substitution is introduced.
constexpr std::size_t capacity=768U*1024*1024;
inline bool owns(const void* pointer) noexcept {
    const auto address=reinterpret_cast<std::size_t>(pointer),start=reinterpret_cast<std::size_t>(base);
    return base && address>=start && address<start+capacity;
}
inline void* allocate(std::size_t count) {
    if(!base) base=static_cast<unsigned char*>(VirtualAlloc(nullptr,capacity,MEM_RESERVE,PAGE_READWRITE));
    if(!base) throw std::bad_alloc();
    count=(count+15)&~std::size_t(15);if(!count) count=16;
    if(count>capacity-used) throw std::bad_alloc();
    const auto next=used+count;
    if(next>committed) {
        const auto target=(next+65535)&~std::size_t(65535);
        if(!VirtualAlloc(base+committed,target-committed,MEM_COMMIT,PAGE_READWRITE)) throw std::bad_alloc();
        committed=target;
    }
    auto* pointer=base+used;used=next;return pointer;
}
}
void* operator new(std::size_t count) { return ui_test_heap::allocate(count); }
void* operator new[](std::size_t count) { return ui_test_heap::allocate(count); }
void operator delete(void*) noexcept {}
void operator delete[](void*) noexcept {}
void operator delete(void*,std::size_t) noexcept {}
void operator delete[](void*,std::size_t) noexcept {}
