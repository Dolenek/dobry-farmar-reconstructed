#pragma once
#include <windows.h>
#include <filesystem>
#include <string>
void driveBootstrapTree(const PROCESS_INFORMATION& parent,HANDLE restricted,HANDLE job,HDESK desktop,
                        const std::filesystem::path& root,const std::wstring& registry,
                        const std::wstring& uiAction);
