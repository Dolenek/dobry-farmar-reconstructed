#pragma once
#include <windows.h>
#include <cstdint>

// Diagnostic-only protocol. No production game or uninstaller links this code.
struct FarmarGuardConfig {
    std::uint32_t version;
    std::uint32_t privateCu;
    std::uint32_t privateLm;
    char root[MAX_PATH];
};
constexpr DWORD farmarGuardReady = 0x4641524d;
