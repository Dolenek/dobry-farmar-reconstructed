#include "GameWString.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <new>
#include <stdexcept>

namespace farmar::recovered {
WString::WString() noexcept : length_(0), capacity_(7) { storage_.local[0] = 0; }
WString::WString(const wchar_t* text) : WString(std::wstring_view(text)) {}
WString::WString(std::wstring_view text) : WString() {
    if (text.size() > 0x7ffffffeU) throw std::length_error("string too long");
    assign(text.data(), static_cast<size_type>(text.size()));
}
WString::WString(const WString& text) : WString() { assign(text); }
WString::~WString() { if (capacity_ >= 8) std::free(storage_.heap); }
WString& WString::operator=(const WString& text) { return assign(text); }
wchar_t* WString::data() noexcept { return capacity_ < 8 ? storage_.local : storage_.heap; }
const wchar_t* WString::data() const noexcept { return capacity_ < 8 ? storage_.local : storage_.heap; }
// 00401490, 0040152b; exact-size fallback 00401505..00401528 / 004010e0.
// Old-MSVC exception runtime interoperability remains unverified.
void WString::grow(size_type requested) {
    auto next = requested | 7U;
    if (next > 0x7ffffffeU) next = requested;
    else if (next / 3 < capacity_ / 2 && capacity_ <= 0x7ffffffeU - capacity_ / 2)
        next = capacity_ + capacity_ / 2;
    auto* memory = static_cast<wchar_t*>(std::malloc((static_cast<std::size_t>(next)+1)*2));
    if (!memory) {
        next = requested;
        memory = static_cast<wchar_t*>(std::malloc((static_cast<std::size_t>(next)+1)*2));
        if (!memory) throw std::bad_alloc();
    }
    std::memcpy(memory, data(), length_*2);
    memory[length_] = 0;
    if (capacity_ >= 8) std::free(storage_.heap);
    storage_.heap = memory;
    capacity_ = next;
}
WString& WString::assign(const wchar_t* text, size_type count) { // 00401ab0
    const auto at = reinterpret_cast<std::uintptr_t>(text);
    const auto begin = reinterpret_cast<std::uintptr_t>(data());
    if (at >= begin && at-begin < length_*2) return assign(*this, (at-begin)/2, count);
    if (count > 0x7ffffffeU) throw std::length_error("string too long");
    if (count > capacity_) grow(count);
    if (count) std::memcpy(data(), text, count*2);
    length_ = count;
    data()[length_] = 0;
    return *this;
}
WString& WString::assign(const WString& text, size_type start, size_type count) { // 004017c0
    if (start > text.size()) throw std::out_of_range("invalid string position");
    count = std::min(count, text.size()-start);
    if (this == &text) { erase(start+count); return erase(0, start); }
    return assign(text.data()+start, count);
}
WString& WString::erase(size_type start, size_type count) { // 004012d0
    if (start > length_) throw std::out_of_range("invalid string position");
    count = std::min(count, length_-start);
    if (count) {
        std::memmove(data()+start, data()+start+count, (length_-start-count)*2);
        length_ -= count;
        data()[length_] = 0;
    }
    return *this;
}
WString& WString::append(const wchar_t* text, size_type count) { // 004df820
    const auto at = reinterpret_cast<std::uintptr_t>(text);
    const auto begin = reinterpret_cast<std::uintptr_t>(data());
    if (at >= begin && at-begin < length_*2) return append(*this, (at-begin)/2, count);
    if (count > 0x7ffffffeU-length_) throw std::length_error("string too long");
    if (count) {
        const auto next = length_+count;
        if (next > capacity_) grow(next);
        std::memcpy(data()+length_, text, count*2);
        length_ = next;
        data()[length_] = 0;
    }
    return *this;
}
WString& WString::append(const WString& text, size_type start, size_type count) { // 004d9dc0
    if (start > text.size()) throw std::out_of_range("invalid string position");
    count = std::min(count, text.size()-start);
    if (count > 0x7ffffffeU-length_) throw std::length_error("string too long");
    if (count) {
        const auto next = length_+count;
        if (next > capacity_) grow(next);
        std::memcpy(data()+length_, text.data()+start, count*2);
        length_ = next;
        data()[length_] = 0;
    }
    return *this;
}
int WString::compare(size_type start, size_type count, const wchar_t* text, size_type text_count) const { // 00401050
    if (start > length_) throw std::out_of_range("invalid string position");
    count = std::min(count, length_-start);
    for (auto at=0U; at<std::min(count, text_count); ++at) {
        const auto a = static_cast<std::uint16_t>(data()[start+at]);
        const auto b = static_cast<std::uint16_t>(text[at]);
        if (a != b) return a < b ? -1 : 1;
    }
    return count == text_count ? 0 : (count < text_count ? -1 : 1);
}
}
