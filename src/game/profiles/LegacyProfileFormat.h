#pragma once
#include <string>
namespace farmar::recovered {
// The profile writers use the old wide ostream's default six-digit format.
std::wstring legacyProfileFloatingText(double value);
}
