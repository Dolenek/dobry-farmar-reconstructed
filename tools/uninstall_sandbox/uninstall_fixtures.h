#pragma once
#include <windows.h>
#include <filesystem>
#include <string>
void seedUninstallFixture(const std::filesystem::path& root,HKEY cu,HKEY lm,const std::wstring& scenario);
void verifyUninstallFixture(const std::filesystem::path& root,HKEY cu,HKEY lm,const std::wstring& scenario);
void verifyCancelledUninstallFixture(const std::filesystem::path& root,HKEY cu,HKEY lm,const std::wstring& scenario);
