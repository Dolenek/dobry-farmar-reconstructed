#pragma once
#include <cstdint>
#include <string_view>

namespace farmar::recovered {
// Original UTF-16 basic_string layout; the +0 prefix remains unidentified.
#pragma pack(push, 4)
class WString {
public:
    using size_type = std::uint32_t;
    static constexpr size_type npos = 0xffffffffU;
    WString() noexcept;
    WString(const wchar_t* text);
    explicit WString(std::wstring_view text);
    WString(const WString& text);
    ~WString();
    WString& operator=(const WString& text);
    wchar_t* data() noexcept;
    const wchar_t* data() const noexcept;
    const wchar_t* c_str() const noexcept { return data(); }
    size_type size() const noexcept { return length_; }
    size_type capacity() const noexcept { return capacity_; }
    bool empty() const noexcept { return length_ == 0; }
    std::wstring_view view() const noexcept { return {data(), length_}; }
    WString& assign(const wchar_t* text, size_type count);
    WString& assign(const WString& text, size_type start = 0, size_type count = npos);
    WString& erase(size_type start = 0, size_type count = npos);
    WString& append(const wchar_t* text, size_type count);
    WString& append(const WString& text, size_type start = 0, size_type count = npos);
    int compare(size_type start, size_type count, const wchar_t* text, size_type text_count) const;
private:
    std::uint32_t prefix_ = 0;
    union Storage { wchar_t local[8]; wchar_t* heap; Storage() {} } storage_;
    size_type length_;
    size_type capacity_;
    void grow(size_type requested);
};
#pragma pack(pop)
static_assert(sizeof(wchar_t) == 2, "Original UTF-16 Windows character type");
static_assert(sizeof(WString) == 28, "Original WString size");
}
