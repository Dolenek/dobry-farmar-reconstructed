#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace farmar::recovered {
// Recovered subset of the original narrow String. See docs/GAME.md.
// Offsets match the inventoried x86 object; no modern std::string ABI is used.
#pragma pack(push, 4)
class String {
public:
    using size_type = std::uint32_t;
    static constexpr size_type npos = 0xffffffffU;
    String() noexcept;
    String(const char* text);
    explicit String(std::string_view text);
    String(const String& text);
    ~String();
    String& operator=(const String& text);

    char* data() noexcept;
    const char* data() const noexcept;
    const char* c_str() const noexcept { return data(); }
    size_type size() const noexcept { return length_; }
    size_type capacity() const noexcept { return capacity_; }
    bool empty() const noexcept { return length_ == 0; }
    std::string_view view() const noexcept { return {data(), length_}; }

    String& assign(const char* text, size_type count);
    String& assign(const String& text, size_type start = 0, size_type count = npos);
    String& erase(size_type start = 0, size_type count = npos);
    String substr(size_type start = 0, size_type count = npos) const;
    size_type find(const char* text, size_type start, size_type count) const noexcept;
    size_type rfind(const char* text, size_type start, size_type count) const noexcept;
    size_type rfind(char character, size_type start = npos) const noexcept;
    int compare(size_type start, size_type count, const char* text, size_type text_count) const;
    void lowercase();

private:
    std::uint32_t prefix_ = 0; // +0 is not read by the recovered routines; role unverified.
    union Storage { char local[16]; char* heap; Storage() {} } storage_;
    size_type length_;
    size_type capacity_;
    void grow(size_type requested);
};
#pragma pack(pop)
static_assert(sizeof(String) == 28, "Recovered String object size");
static_assert(alignof(String) == 4, "Recovered String object alignment");
}
