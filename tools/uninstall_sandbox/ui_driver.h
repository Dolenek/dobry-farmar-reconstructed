#pragma once
#include <windows.h>
#include <filesystem>
#include <string>
void driveUninstallUi(const PROCESS_INFORMATION& process,HDESK desktop,
                      const std::filesystem::path& root,const std::wstring& action);
