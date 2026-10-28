// Owned test program. The launcher installs guards before this PE's entry point.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <string>
#include <stdexcept>
#include <cstring>

void check(bool ok,const char* message) {if(!ok)throw std::runtime_error(message);}
int main(int argc,char** argv) {
    if(argc!=3)return 2;
    const std::string root=argv[1],outside=argv[2];
    try {
        const auto file=root+"\\guard-client-data.txt";
        HANDLE f=CreateFileA(file.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
        check(f!=INVALID_HANDLE_VALUE,"inside create");DWORD written=0;
        check(WriteFile(f,"inside",6,&written,nullptr)&&written==6,"inside write");CloseHandle(f);
        f=CreateFileA(outside.c_str(),GENERIC_WRITE,0,nullptr,OPEN_EXISTING,0,nullptr);
        check(f==INVALID_HANDLE_VALUE&&GetLastError()==ERROR_ACCESS_DENIED,"outside create access");
        check(!DeleteFileA(outside.c_str())&&GetLastError()==ERROR_ACCESS_DENIED,"outside delete");
        check(!CopyFileA(file.c_str(),outside.c_str(),FALSE)&&GetLastError()==ERROR_ACCESS_DENIED,"outside copy");
        check(!MoveFileA(file.c_str(),outside.c_str())&&GetLastError()==ERROR_ACCESS_DENIED,"outside move");
        check(!SetFileAttributesA(outside.c_str(),FILE_ATTRIBUTE_NORMAL)&&GetLastError()==ERROR_ACCESS_DENIED,"outside attributes");
        check(!WritePrivateProfileStringA("guard","key","changed",outside.c_str())&&GetLastError()==ERROR_ACCESS_DENIED,"outside ini");
        check(!CreateDirectoryA((root+"\\..\\forbidden-directory").c_str(),nullptr)&&GetLastError()==ERROR_ACCESS_DENIED,"parent traversal");

        // This deliberately opens our Untrusted outside guard through the test's
        // W import. NSIS has no such import; WriteFile/SetFileTime must still
        // reject the escaped handle rather than relying on CreateFileA alone.
        wchar_t wide[32768]{};check(MultiByteToWideChar(CP_ACP,0,outside.c_str(),-1,wide,32768)>0,"outside wide path");
        f=CreateFileW(wide,GENERIC_WRITE,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);
        check(f!=INVALID_HANDLE_VALUE,"controlled outside Untrusted handle");
        check(!WriteFile(f,"changed",7,&written,nullptr)&&GetLastError()==ERROR_ACCESS_DENIED,"outside handle write");
        FILETIME time{};GetSystemTimeAsFileTime(&time);check(!SetFileTime(f,nullptr,nullptr,&time)&&GetLastError()==ERROR_ACCESS_DENIED,"outside handle timestamp");CloseHandle(f);
        using Move=BOOL(WINAPI*)(LPCSTR,LPCSTR,DWORD);
        auto move=reinterpret_cast<Move>(GetProcAddress(GetModuleHandleA("kernel32.dll"),"MoveFileExA"));check(move!=nullptr,"guarded dynamic MoveFileExA");
        check(move(file.c_str(),nullptr,MOVEFILE_DELAY_UNTIL_REBOOT)!=FALSE,"private delayed delete intent");
        check(GetFileAttributesA(file.c_str())!=INVALID_FILE_ATTRIBUTES,"delayed intent must not delete file");
        check(GetProcAddress(GetModuleHandleA("ntdll.dll"),"NtDeleteFile")==nullptr,"unknown dynamic export");
        check(!LoadLibraryA("forbidden-application.dll")&&GetLastError()==ERROR_ACCESS_DENIED,"unknown library");
        for(const int id:{CSIDL_PROGRAMS,CSIDL_COMMON_PROGRAMS,CSIDL_DESKTOPDIRECTORY,CSIDL_COMMON_DESKTOPDIRECTORY}) {
            LPITEMIDLIST list=nullptr;check(SUCCEEDED(SHGetSpecialFolderLocation(nullptr,id,&list))&&list,"private folder pidl");
            char path[MAX_PATH]{};check(SHGetPathFromIDListA(list,path)&&std::string(path).find(root+"\\shell\\")==0,"private folder path");CoTaskMemFree(list);
        }
        LPITEMIDLIST unknown=nullptr;check(SHGetSpecialFolderLocation(nullptr,CSIDL_PERSONAL,&unknown)==E_ACCESSDENIED&&!unknown,"unknown special folder");
        for(auto hive:{HKEY_CURRENT_USER,HKEY_LOCAL_MACHINE}) {
            HKEY key=nullptr;check(RegCreateKeyExA(hive,"Software\\FarmarGuardClient",0,nullptr,0,KEY_ALL_ACCESS,nullptr,&key,nullptr)==ERROR_SUCCESS,"private registry create");
            DWORD value=73;check(RegSetValueExA(key,"proof",0,REG_DWORD,reinterpret_cast<BYTE*>(&value),4)==ERROR_SUCCESS,"private registry write");RegCloseKey(key);
        }
        f=CreateFileA((root+"\\guard-client-proof.txt").c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,0,nullptr);check(f!=INVALID_HANDLE_VALUE,"proof create");
        constexpr char proof[]="guarded-before-entry";check(WriteFile(f,proof,sizeof(proof)-1,&written,nullptr)&&written==sizeof(proof)-1,"proof write");CloseHandle(f);return 0;
    }catch(const std::exception& error){OutputDebugStringA(error.what());return 11;}
}
