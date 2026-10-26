#pragma once
#include "GameString.h"
#include <shlobj.h>
namespace farmar::recovered {
class FileIndex;
struct GameFileSystemServices {HRESULT(WINAPI* folderPath)(HWND,int,HANDLE,DWORD,LPSTR)=&SHGetFolderPathA;};
// Bootstrap portion of0054d720, before ResourceManager/Settings construction.
// An explicit root is for isolated installations/tests; default uses original
// COMMON_APPDATA/app_directory and original settings.xml fallback.
String mountGameCommonFiles(FileIndex&,const String& explicitRoot=String(),GameFileSystemServices services={});
}
