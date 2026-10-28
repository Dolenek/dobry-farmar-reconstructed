#pragma once
#include <windows.h>
#include <filesystem>
#include <functional>
DWORD runGuardedEntry(const PROCESS_INFORMATION& process,const std::filesystem::path& root,
                      HANDLE privateCu,HANDLE privateLm,const std::function<void()>& onStarted={});
