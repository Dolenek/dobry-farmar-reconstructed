#include "GameString.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <new>
#include <stdexcept>

namespace farmar::recovered {
String::String() noexcept : length_(0), capacity_(15) { storage_.local[0] = '\0'; }
String::String(const char* text) : String(std::string_view(text)) {}
String::String(std::string_view text) : String() {
    if (text.size() >= npos) throw std::length_error("string too long");
    assign(text.data(), static_cast<size_type>(text.size()));
}
String::String(const String& text) : String() { assign(text); }
String::~String() { if (capacity_ >= 16) std::free(storage_.heap); }
String& String::operator=(const String& text) { return assign(text); }
char* String::data() noexcept { return capacity_ < 16 ? storage_.local : storage_.heap; }
const char* String::data() const noexcept { return capacity_ < 16 ? storage_.local : storage_.heap; }

// 00401350..00401458, including the allocation-failure fallback at 004013c3.
void String::grow(size_type requested) {
    size_type next = requested | 15U;
    if (next > npos - 1) next = requested;
    else if (next / 3 < capacity_ / 2 && capacity_ <= npos - 1 - capacity_ / 2)
        next = capacity_ + capacity_ / 2;
    auto* memory = static_cast<char*>(std::malloc(static_cast<std::size_t>(next) + 1));
    if (!memory) {
        next = requested;
        memory = static_cast<char*>(std::malloc(static_cast<std::size_t>(next) + 1));
        if (!memory) throw std::bad_alloc();
    }
    std::memcpy(memory, data(), length_);
    memory[length_] = '\0';
    if (capacity_ >= 16) std::free(storage_.heap);
    storage_.heap = memory;
    capacity_ = next;
}

// 004019c0: pointers inside the existing contents use substring assignment.
String& String::assign(const char* text, size_type count) {
    const auto address = reinterpret_cast<std::uintptr_t>(text);
    const auto begin = reinterpret_cast<std::uintptr_t>(data());
    if (address >= begin && address - begin < length_)
        return assign(*this, static_cast<size_type>(address - begin), count);
    if (count == npos) throw std::length_error("string too long");
    if (count > capacity_) grow(count);
    if (count) std::memcpy(data(), text, count);
    length_ = count;
    data()[length_] = '\0';
    return *this;
}

// 004016d0: validate offset first, clamp count, handle self-assignment by erasing.
String& String::assign(const String& text, size_type start, size_type count) {
    if (start > text.size()) throw std::out_of_range("invalid string position");
    count = std::min(count, text.size() - start);
    if (this == &text) { erase(start + count); return erase(0, start); }
    return assign(text.data() + start, count);
}

// 00401250: preserve allocated capacity even when the result becomes empty.
String& String::erase(size_type start, size_type count) {
    if (start > length_) throw std::out_of_range("invalid string position");
    count = std::min(count, length_ - start);
    if (count) {
        std::memmove(data() + start, data() + start + count, length_ - start - count);
        length_ -= count;
        data()[length_] = '\0';
    }
    return *this;
}
String String::substr(size_type start, size_type count) const {
    String result; result.assign(*this, start, count); return result;
}

// 00548260 and 00548330: explicit lengths also search embedded NUL bytes.
String::size_type String::find(const char* text, size_type start, size_type count) const noexcept {
    if (!count) return start <= length_ ? start : npos;
    if (start >= length_ || count > length_ - start) return npos;
    const auto last = length_ - count;
    for (auto at = start; at <= last; ++at)
        if (data()[at] == text[0] && std::memcmp(data() + at, text, count) == 0) return at;
    return npos;
}
String::size_type String::rfind(const char* text, size_type start, size_type count) const noexcept {
    if (!count) return std::min(start, length_);
    if (count > length_) return npos;
    auto at = std::min(start, length_ - count);
    do {
        if (data()[at] == text[0] && std::memcmp(data() + at, text, count) == 0) return at;
        if (!at) break;
        --at;
    } while (true);
    return npos;
}
String::size_type String::rfind(char character, size_type start) const noexcept {
    return rfind(&character, start, 1);
}

// 00403960: byte comparison is unsigned; results are exactly -1, 0 or +1.
int String::compare(size_type start, size_type count, const char* text, size_type text_count) const {
    if (start > length_) throw std::out_of_range("invalid string position");
    count = std::min(count, length_ - start);
    const auto common = std::min(count, text_count);
    if (common) {
        const auto result = std::memcmp(data() + start, text, common);
        if (result) return result < 0 ? -1 : 1;
    }
    return count == text_count ? 0 : (count < text_count ? -1 : 1);
}
void String::lowercase() {
    for (size_type at = 0; at < length_; ++at)
        data()[at] = static_cast<char>(std::tolower(static_cast<unsigned char>(data()[at])));
}
}
