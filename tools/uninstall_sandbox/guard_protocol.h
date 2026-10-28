#pragma once
#include <windows.h>
#include <cstdint>

// Diagnostic-only protocol. No production game or uninstaller links this code.
struct FarmarGuardConfig {
    std::uint32_t version;
    std::uint32_t privateCu;
    std::uint32_t privateLm;
    char root[MAX_PATH];
    std::uint32_t allowBootstrap;
    std::uint32_t generation;
};
constexpr DWORD farmarGuardReady = 0x4641524d;
constexpr DWORD farmarGuardVersion = 2;
struct FarmarBootstrapLaunch {
    std::uint32_t magic;
    std::uint32_t parentPid;
    std::uint32_t childPid;
    std::uint32_t childThread;
    std::uint32_t originalFlags;
    std::uint32_t creationBrokered;
    char executable[MAX_PATH];
    char directory[MAX_PATH];
    char command[2048];
};
constexpr DWORD farmarBootstrapMagic = 0x314c4246; // FBL1, owned protocol only
struct FarmarBootstrapReply {
    std::uint32_t magic;
    std::uint32_t parentPid;
    std::uint32_t childPid;
    std::uint32_t childThread;
    std::uint32_t processHandle;
    std::uint32_t threadHandle;
};
constexpr DWORD farmarBootstrapReplyMagic=0x31524246; // FBR1
