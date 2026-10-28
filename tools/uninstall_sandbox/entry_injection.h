#pragma once
#include <windows.h>
#include <filesystem>
DWORD runGuardedEntry(const PROCESS_INFORMATION& process,const std::filesystem::path& root,
                      HANDLE privateCu,HANDLE privateLm);
