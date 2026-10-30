#pragma once
#include <cstdint>
namespace farmar::recovered {
class WString;
// Destination words survive an empty/failed original stream extraction.
std::uint32_t parseLegacyProfileFloat(const WString& text,std::uint32_t destination);
std::uint64_t parseLegacyProfileDouble(const WString& text,std::uint64_t destination);
std::uint64_t captureProfileDoubleScratch();
}
