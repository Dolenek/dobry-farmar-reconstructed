#include "GameString.h"
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

int main() {
    using farmar::recovered::String;
    int failures = 0;
    auto check = [&](bool condition, const char* message) {
        if (!condition) { std::cerr << message << '\n'; ++failures; }
    };
    String text;
    check(text.empty() && text.capacity() == 15 && text.c_str()[0] == 0, "empty SSO");
    text.assign("123456789012345", 15);
    check(text.capacity() == 15, "15 bytes inline");
    text.assign("1234567890123456", 16);
    check(text.capacity() == 31, "16 bytes on heap");
    text.assign(std::string(32, 'x').data(), 32);
    check(text.capacity() == 47, "32 bytes capacity");
    text.assign(std::string(48, 'y').data(), 48);
    check(text.capacity() == 70, "original growth curve");
    String copy(text);
    check(copy.view() == text.view() && copy.data() != text.data(), "copy owns data");
    check(copy.capacity() == 63, "copy capacity is independent");
    text.erase();
    check(text.empty() && text.capacity() == 70 && text.c_str()[0] == 0, "erase keeps heap");
    text.assign("abcdefgh", 8);
    text.assign(text, 2, 3);
    check(text.view() == "cde", "self substring assignment");
    text.assign("abcdef", 6);
    text.assign(text.data() + 2, 100);
    check(text.view() == "cdef", "aliased assignment clamps to source");
    text.assign("a\0b\0a", 5);
    check(text.size() == 5 && text.find("\0b", 0, 2) == 1, "embedded NUL search");
    check(text.rfind("\0", String::npos, 1) == 3, "embedded NUL reverse search");
    check(text.find(nullptr, 5, 0) == 5 && text.find(nullptr, 6, 0) == String::npos, "empty search bounds");
    check(text.rfind(nullptr, String::npos, 0) == 5, "empty reverse search bounds");
    check(text.substr(1, 2).view() == std::string_view("\0b", 2), "substring preserves NUL");
    check(text.compare(0, 1, "b", 1) == -1, "comparison sign");
    text.assign("\xff", 1);
    check(text.compare(0, 1, "\x7f", 1) == 1, "comparison uses unsigned bytes");
    for (int operation = 0; operation < 3; ++operation) {
        bool threw = false;
        try {
            if (operation == 0) text.erase(2);
            if (operation == 1) text.substr(2);
            if (operation == 2) text.compare(2, 1, "", 0);
        } catch (const std::out_of_range&) { threw = true; }
        check(threw, "invalid offset throws");
    }
    // Raw field locations are also consumed by the original-function oracle.
    std::uint32_t length, capacity;
    const auto* bytes = reinterpret_cast<const char*>(&text);
    std::memcpy(&length, bytes + 20, 4); std::memcpy(&capacity, bytes + 24, 4);
    check(length == text.size() && capacity == text.capacity(), "field offsets");
    return failures ? 1 : 0;
}
