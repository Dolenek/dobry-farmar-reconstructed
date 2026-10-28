#pragma once
#include <windows.h>
#include <filesystem>
#include <functional>
struct GuardedEntryOptions {
    bool bootstrapParent=false;
    DWORD generation=0;
};
DWORD runGuardedEntry(const PROCESS_INFORMATION& process,const std::filesystem::path& root,
                      HANDLE privateCu,HANDLE privateLm,const std::function<void()>& onStarted={},
                      const GuardedEntryOptions& options={});
