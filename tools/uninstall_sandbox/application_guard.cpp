// Diagnostic OS boundary for the known NSIS reference/source executables.
// Entry injection must complete this setup before any application instruction.
#define WIN32_LEAN_AND_MEAN
#include "guard_protocol.h"
#include <shlobj.h>
#include <shellapi.h>
#include <algorithm>
#include <cstring>
#include <string>
#include <unordered_set>
#include <vector>

namespace {
std::string root, shortRoot;
HANDLE logFile=INVALID_HANDLE_VALUE;
CRITICAL_SECTION lock;
std::unordered_set<HKEY> keys;
std::unordered_set<HMODULE> patched;

std::string quoted(const char* value) {
    std::string out="\"";
    if(value) for(const unsigned char c:std::string(value)) {
        if(c=='"'||c=='\\') {out+='\\';out+=char(c);}
        else if(c<32||c>=127) {const char digits[]="0123456789abcdef";out+="\\u00";out+=digits[c>>4];out+=digits[c&15];}
        else out+=char(c);
    }
    return out+'"';
}
void event(const char* api,const char* path,bool allowed,DWORD error=0) {
    const auto saved=GetLastError();
    const auto line=std::string("{\"api\":")+quoted(api)+",\"path\":"+quoted(path)+
        ",\"allowed\":"+(allowed?"true":"false")+",\"error\":"+std::to_string(error)+"}\n";
    EnterCriticalSection(&lock);DWORD written=0;WriteFile(logFile,line.data(),DWORD(line.size()),&written,nullptr);LeaveCriticalSection(&lock);
    if(written!=line.size()) ExitProcess(0xe1);
    SetLastError(saved);
}
bool inside(const char* path) {
    if(!path||!*path||!std::strncmp(path,"\\\\",2)) return false;
    char full[32768]{};const DWORD n=GetFullPathNameA(path,sizeof(full),full,nullptr);
    if(!n||n>=sizeof(full)) return false;
    std::string value(full);std::replace(value.begin(),value.end(),'/','\\');
    if(value.find(':',2)!=std::string::npos) return false; // no alternate streams/devices
    size_t prefix=0;
    for(const auto* candidate:{&root,&shortRoot})
        if(!candidate->empty()&&value.size()>candidate->size()&&value[candidate->size()]=='\\'&&
           !_strnicmp(value.c_str(),candidate->c_str(),candidate->size())) prefix=candidate->size();
    if(!prefix) return false; // the sandbox root itself is not removable
    for(size_t i=prefix;i<=value.size();++i) if(i==value.size()||value[i]=='\\') {
        const auto part=value.substr(0,i);const DWORD attrs=GetFileAttributesA(part.c_str());
        if(attrs!=INVALID_FILE_ATTRIBUTES&&(attrs&FILE_ATTRIBUTE_REPARSE_POINT)) return false;
    }
    return true;
}
bool permit(const char* api,const char* path) {
    const bool ok=inside(path);event(api,path,ok,ok?0:ERROR_ACCESS_DENIED);
    if(!ok)SetLastError(ERROR_ACCESS_DENIED);return ok;
}
bool fileHandle(const char* api,HANDLE file) {
    char path[32768]{};const auto n=GetFinalPathNameByHandleA(file,path,sizeof(path),FILE_NAME_NORMALIZED|VOLUME_NAME_DOS);
    if(!n||n>=sizeof(path)){event(api,"<unknown handle>",false,ERROR_ACCESS_DENIED);SetLastError(ERROR_ACCESS_DENIED);return false;}
    return permit(api,!std::strncmp(path,"\\\\?\\",4)?path+4:path);
}
bool knownKey(HKEY key) {
    if(key==HKEY_CURRENT_USER||key==HKEY_LOCAL_MACHINE) return true;
    EnterCriticalSection(&lock);const bool found=keys.count(key)!=0;LeaveCriticalSection(&lock);return found;
}
bool regPermit(const char* api,HKEY key,const char* name) {
    const bool ok=knownKey(key);event(api,name,ok,ok?0:ERROR_ACCESS_DENIED);return ok;
}
void remember(HKEY key) {EnterCriticalSection(&lock);keys.insert(key);LeaveCriticalSection(&lock);}

HANDLE WINAPI guardedCreateFileA(LPCSTR path,DWORD access,DWORD share,LPSECURITY_ATTRIBUTES sa,DWORD creation,DWORD flags,HANDLE templ) {
    const DWORD writes=GENERIC_WRITE|GENERIC_ALL|DELETE|WRITE_DAC|WRITE_OWNER|FILE_WRITE_DATA|FILE_APPEND_DATA|FILE_WRITE_EA|FILE_WRITE_ATTRIBUTES;
    if((access&writes)||creation!=OPEN_EXISTING||(flags&FILE_FLAG_DELETE_ON_CLOSE))
        if(!permit("CreateFileA",path))return INVALID_HANDLE_VALUE;
    return CreateFileA(path,access,share,sa,creation,flags,templ);
}
BOOL WINAPI guardedWriteFile(HANDLE file,LPCVOID data,DWORD length,LPDWORD written,LPOVERLAPPED overlapped) {
    if(!fileHandle("WriteFile",file))return FALSE;return WriteFile(file,data,length,written,overlapped);
}
BOOL WINAPI guardedSetFileTime(HANDLE file,const FILETIME* a,const FILETIME* b,const FILETIME* c) {
    if(!fileHandle("SetFileTime",file))return FALSE;return SetFileTime(file,a,b,c);
}
BOOL WINAPI guardedCreateDirectoryA(LPCSTR path,LPSECURITY_ATTRIBUTES sa) {
    char full[32768]{};const auto length=path?GetFullPathNameA(path,sizeof(full),full,nullptr):0;
    if(length&&length<sizeof(full)) {
        std::string value(full);while(value.size()>3&&value.back()=='\\')value.pop_back();
        for(const auto* candidate:{&root,&shortRoot})if(!value.empty()&&candidate->size()>=value.size()&&
            !_strnicmp(candidate->c_str(),value.c_str(),value.size())&&
            (candidate->size()==value.size()||(*candidate)[value.size()]=='\\')) {
            // A known existing ancestor of our newly created root: return the
            // ordinary no-op result without calling any outside mutator.
            event("CreateDirectoryA.existing-ancestor-noop",path,true,ERROR_ALREADY_EXISTS);SetLastError(ERROR_ALREADY_EXISTS);return FALSE;
        }
    }
    return permit("CreateDirectoryA",path)&&CreateDirectoryA(path,sa);
}
BOOL WINAPI guardedRemoveDirectoryA(LPCSTR path) {return permit("RemoveDirectoryA",path)&&RemoveDirectoryA(path);}
BOOL WINAPI guardedDeleteFileA(LPCSTR path) {return permit("DeleteFileA",path)&&DeleteFileA(path);}
BOOL WINAPI guardedSetCurrentDirectoryA(LPCSTR path) {return permit("SetCurrentDirectoryA",path)&&SetCurrentDirectoryA(path);}
BOOL WINAPI guardedSetFileAttributesA(LPCSTR path,DWORD flags) {return permit("SetFileAttributesA",path)&&SetFileAttributesA(path,flags);}
BOOL WINAPI guardedCopyFileA(LPCSTR from,LPCSTR to,BOOL fail) {return permit("CopyFileA.destination",to)&&CopyFileA(from,to,fail);}
BOOL WINAPI guardedMoveFileA(LPCSTR from,LPCSTR to) {return permit("MoveFileA.source",from)&&permit("MoveFileA.destination",to)&&MoveFileA(from,to);}
BOOL WINAPI guardedMoveFileExA(LPCSTR from,LPCSTR to,DWORD flags) {
    if(!permit("MoveFileExA.source",from)||(to&&!permit("MoveFileExA.destination",to)))return FALSE;
    if(flags&MOVEFILE_DELAY_UNTIL_REBOOT) {event("MoveFileExA.delayed-intent-only",from,true,flags);return TRUE;}
    return MoveFileExA(from,to,flags);
}
BOOL WINAPI guardedWritePrivateProfileStringA(LPCSTR section,LPCSTR key,LPCSTR value,LPCSTR path) {
    return permit("WritePrivateProfileStringA",path)&&WritePrivateProfileStringA(section,key,value,path);
}
UINT WINAPI guardedGetTempFileNameA(LPCSTR path,LPCSTR prefix,UINT unique,LPSTR result) {
    if(!permit("GetTempFileNameA",path))return 0;return GetTempFileNameA(path,prefix,unique,result);
}
BOOL WINAPI guardedCreateProcessA(LPCSTR application,LPSTR,LPSECURITY_ATTRIBUTES,LPSECURITY_ATTRIBUTES,BOOL,DWORD,LPVOID,LPCSTR,LPSTARTUPINFOA,LPPROCESS_INFORMATION) {
    event("CreateProcessA.blocked-in-single-process-case",application,false,ERROR_ACCESS_DENIED);SetLastError(ERROR_ACCESS_DENIED);return FALSE;
}
BOOL WINAPI guardedExitWindowsEx(UINT,DWORD) {event("ExitWindowsEx",nullptr,false,ERROR_ACCESS_DENIED);SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
HINSTANCE WINAPI guardedShellExecuteA(HWND,LPCSTR,LPCSTR file,LPCSTR,LPCSTR,INT) {event("ShellExecuteA",file,false,ERROR_ACCESS_DENIED);SetLastError(ERROR_ACCESS_DENIED);return reinterpret_cast<HINSTANCE>(INT_PTR(SE_ERR_ACCESSDENIED));}
int WINAPI guardedSHFileOperationA(LPSHFILEOPSTRUCTA) {event("SHFileOperationA",nullptr,false,ERROR_ACCESS_DENIED);return ERROR_ACCESS_DENIED;}
HRESULT WINAPI guardedCoCreateInstance(REFCLSID,LPUNKNOWN,DWORD,REFIID,LPVOID* output) {if(output)*output=nullptr;event("CoCreateInstance",nullptr,false,ERROR_ACCESS_DENIED);return E_ACCESSDENIED;}
BOOL WINAPI guardedSystemParametersInfoA(UINT action,UINT parameter,PVOID data,UINT flags) {
    if((action!=SPI_GETWORKAREA&&action!=SPI_GETNONCLIENTMETRICS)||flags){event("SystemParametersInfoA.setter-or-unknown",nullptr,false,ERROR_ACCESS_DENIED);SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
    return SystemParametersInfoA(action,parameter,data,flags);
}

std::string specialFolder(int id) {
    switch(id&0xff) {
    case CSIDL_PROGRAMS:return root+"\\shell\\user\\programs";
    case CSIDL_COMMON_PROGRAMS:return root+"\\shell\\common\\programs";
    case CSIDL_DESKTOP:case CSIDL_DESKTOPDIRECTORY:return root+"\\shell\\user\\desktop";
    case CSIDL_COMMON_DESKTOPDIRECTORY:return root+"\\shell\\common\\desktop";
    default:return {};
    }
}
HRESULT WINAPI guardedSHGetSpecialFolderLocation(HWND,int id,LPITEMIDLIST* output) {
    if(!output)return E_INVALIDARG;*output=nullptr;const auto path=specialFolder(id);
    if(path.empty()){event("SHGetSpecialFolderLocation.unknown",std::to_string(id).c_str(),false,ERROR_ACCESS_DENIED);return E_ACCESSDENIED;}
    const USHORT bytes=USHORT(6+path.size()+1);auto* memory=static_cast<BYTE*>(CoTaskMemAlloc(bytes+2));if(!memory)return E_OUTOFMEMORY;
    const DWORD magic=0x50494446;std::memcpy(memory,&bytes,2);std::memcpy(memory+2,&magic,4);std::memcpy(memory+6,path.c_str(),path.size()+1);memory[bytes]=memory[bytes+1]=0;
    *output=reinterpret_cast<LPITEMIDLIST>(memory);event("SHGetSpecialFolderLocation",path.c_str(),true,DWORD(id));return S_OK;
}
BOOL WINAPI guardedSHGetPathFromIDListA(PCIDLIST_ABSOLUTE list,LPSTR output) {
    if(!list||!output)return FALSE;DWORD magic=0;std::memcpy(&magic,reinterpret_cast<const BYTE*>(list)+2,4);
    if(magic!=0x50494446){event("SHGetPathFromIDListA.unknown",nullptr,false,ERROR_ACCESS_DENIED);return FALSE;}
    const auto* path=reinterpret_cast<const char*>(list)+6;if(!inside(path))return FALSE;lstrcpynA(output,path,MAX_PATH);return TRUE;
}
LSTATUS WINAPI guardedRegOpenKeyExA(HKEY parent,LPCSTR path,DWORD options,REGSAM rights,PHKEY output) {
    if(!regPermit("RegOpenKeyExA",parent,path))return ERROR_ACCESS_DENIED;const auto result=RegOpenKeyExA(parent,path,options,rights,output);if(result==ERROR_SUCCESS)remember(*output);return result;
}
LSTATUS WINAPI guardedRegCreateKeyExA(HKEY parent,LPCSTR path,DWORD reserved,LPSTR cls,DWORD options,REGSAM rights,const LPSECURITY_ATTRIBUTES sa,PHKEY output,LPDWORD disposition) {
    if(!regPermit("RegCreateKeyExA",parent,path))return ERROR_ACCESS_DENIED;const auto result=RegCreateKeyExA(parent,path,reserved,cls,options,rights,sa,output,disposition);if(result==ERROR_SUCCESS)remember(*output);return result;
}
LSTATUS WINAPI guardedRegDeleteKeyA(HKEY parent,LPCSTR path) {return regPermit("RegDeleteKeyA",parent,path)?RegDeleteKeyA(parent,path):ERROR_ACCESS_DENIED;}
LSTATUS WINAPI guardedRegDeleteKeyExA(HKEY parent,LPCSTR path,REGSAM rights,DWORD reserved) {return regPermit("RegDeleteKeyExA",parent,path)?RegDeleteKeyExA(parent,path,rights,reserved):ERROR_ACCESS_DENIED;}
LSTATUS WINAPI guardedRegDeleteValueA(HKEY parent,LPCSTR name) {return regPermit("RegDeleteValueA",parent,name)?RegDeleteValueA(parent,name):ERROR_ACCESS_DENIED;}
LSTATUS WINAPI guardedRegSetValueExA(HKEY parent,LPCSTR name,DWORD reserved,DWORD type,const BYTE* data,DWORD length) {return regPermit("RegSetValueExA",parent,name)?RegSetValueExA(parent,name,reserved,type,data,length):ERROR_ACCESS_DENIED;}
LSTATUS WINAPI guardedRegQueryValueExA(HKEY parent,LPCSTR name,LPDWORD reserved,LPDWORD type,LPBYTE data,LPDWORD length) {return regPermit("RegQueryValueExA",parent,name)?RegQueryValueExA(parent,name,reserved,type,data,length):ERROR_ACCESS_DENIED;}
LSTATUS WINAPI guardedRegEnumKeyA(HKEY parent,DWORD index,LPSTR name,DWORD length) {return regPermit("RegEnumKeyA",parent,nullptr)?RegEnumKeyA(parent,index,name,length):ERROR_ACCESS_DENIED;}
LSTATUS WINAPI guardedRegEnumValueA(HKEY parent,DWORD index,LPSTR name,LPDWORD nameLength,LPDWORD reserved,LPDWORD type,LPBYTE data,LPDWORD length) {return regPermit("RegEnumValueA",parent,nullptr)?RegEnumValueA(parent,index,name,nameLength,reserved,type,data,length):ERROR_ACCESS_DENIED;}
LSTATUS WINAPI guardedRegCloseKey(HKEY key) {
    if(!knownKey(key))return ERROR_ACCESS_DENIED;EnterCriticalSection(&lock);keys.erase(key);LeaveCriticalSection(&lock);return RegCloseKey(key);
}

FARPROC replacement(const char* name);
bool patch(HMODULE module) {
    EnterCriticalSection(&lock);const bool already=patched.count(module)!=0;LeaveCriticalSection(&lock);if(already)return true;
    auto* base=reinterpret_cast<BYTE*>(module);auto* dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);if(dos->e_magic!=IMAGE_DOS_SIGNATURE)return false;
    auto* nt=reinterpret_cast<IMAGE_NT_HEADERS32*>(base+dos->e_lfanew);if(nt->Signature!=IMAGE_NT_SIGNATURE||nt->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC)return false;
    auto directory=nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    for(auto* desc=reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base+directory.VirtualAddress);directory.VirtualAddress&&desc->Name;++desc) {
        if(!desc->OriginalFirstThunk)return false;
        auto* names=reinterpret_cast<IMAGE_THUNK_DATA32*>(base+desc->OriginalFirstThunk);auto* functions=reinterpret_cast<IMAGE_THUNK_DATA32*>(base+desc->FirstThunk);
        for(;names->u1.AddressOfData;++names,++functions) {
            if(names->u1.Ordinal&IMAGE_ORDINAL_FLAG32)continue;
            const auto* name=reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base+names->u1.AddressOfData)->Name;const auto hook=replacement(reinterpret_cast<const char*>(name));
            if(hook){DWORD old=0,ignored=0;if(!VirtualProtect(&functions->u1.Function,sizeof(DWORD),PAGE_READWRITE,&old))return false;functions->u1.Function=DWORD(reinterpret_cast<uintptr_t>(hook));if(!VirtualProtect(&functions->u1.Function,sizeof(DWORD),old,&ignored))return false;}
        }
    }
    EnterCriticalSection(&lock);patched.insert(module);LeaveCriticalSection(&lock);return true;
}
bool allowedLibrary(const char* name) {
    if(!name||!*name)return false;
    if(std::strpbrk(name,"\\/:"))return inside(name);
    auto text=std::string(name);std::transform(text.begin(),text.end(),text.begin(),[](char c){return char(c>='A'&&c<='Z'?c+32:c);});if(text.size()>4&&text.substr(text.size()-4)==".dll")text.resize(text.size()-4);
    return text=="kernel32"||text=="user32"||text=="advapi32"||text=="shlwapi"||text=="comctl32"||text=="ole32"||text=="riched20"||text=="riched32";
}
HMODULE WINAPI guardedLoadLibraryExA(LPCSTR name,HANDLE file,DWORD flags) {
    if(!allowedLibrary(name)){event("LoadLibraryExA",name,false,ERROR_ACCESS_DENIED);SetLastError(ERROR_ACCESS_DENIED);return nullptr;}
    std::string resolved=name;if(!std::strpbrk(name,"\\/:")){char system[MAX_PATH]{};if(!GetSystemDirectoryA(system,MAX_PATH))return nullptr;resolved=std::string(system)+"\\"+name;if(resolved.size()<4||_stricmp(resolved.c_str()+resolved.size()-4,".dll"))resolved+=".dll";}
    event("LoadLibraryExA.begin",name,true,flags);auto module=LoadLibraryExA(resolved.c_str(),file,flags);const auto error=module?ERROR_SUCCESS:GetLastError();event("LoadLibraryExA.result",name,module!=nullptr,error);
    // Determine ownership from the loaded image, never from a bare DLL name
    // resolved against the current (private) working directory.
    char actual[32768]{};if(module){const auto length=GetModuleFileNameA(module,actual,sizeof(actual));if(!length||length>=sizeof(actual))ExitProcess(0xe2);if(inside(actual)&&!patch(module)){event("LoadLibraryExA.patch-failed",actual,false);ExitProcess(0xe2);}}return module;
}
HMODULE WINAPI guardedLoadLibraryA(LPCSTR name) {return guardedLoadLibraryExA(name,nullptr,0);}
BOOL WINAPI guardedFreeLibrary(HMODULE module) {EnterCriticalSection(&lock);patched.erase(module);LeaveCriticalSection(&lock);return FreeLibrary(module);}
FARPROC WINAPI guardedGetProcAddress(HMODULE module,LPCSTR name) {
    if(reinterpret_cast<uintptr_t>(name)<65536){event("GetProcAddress.ordinal",nullptr,false,ERROR_ACCESS_DENIED);SetLastError(ERROR_PROC_NOT_FOUND);return nullptr;}
    if(const auto hook=replacement(name))return hook;
    for(const auto* allowed:{"GetDiskFreeSpaceExA","GetUserDefaultUILanguage","SHAutoComplete","LangDialog"})
        if(!std::strcmp(name,allowed)){const auto function=GetProcAddress(module,name);event("GetProcAddress.allowed",name,function!=nullptr,function?0:GetLastError());return function;}
    event("GetProcAddress.unknown",name,false,ERROR_ACCESS_DENIED);SetLastError(ERROR_PROC_NOT_FOUND);return nullptr;
}
FARPROC replacement(const char* name) {
#define HOOK(api) if(!std::strcmp(name,#api))return reinterpret_cast<FARPROC>(&guarded##api)
    HOOK(CreateFileA);HOOK(WriteFile);HOOK(SetFileTime);HOOK(CreateDirectoryA);HOOK(RemoveDirectoryA);HOOK(DeleteFileA);
    HOOK(SetCurrentDirectoryA);HOOK(SetFileAttributesA);HOOK(CopyFileA);HOOK(MoveFileA);HOOK(MoveFileExA);HOOK(WritePrivateProfileStringA);
    HOOK(GetTempFileNameA);HOOK(CreateProcessA);HOOK(ExitWindowsEx);HOOK(ShellExecuteA);HOOK(SHFileOperationA);
    HOOK(CoCreateInstance);HOOK(SystemParametersInfoA);
    HOOK(SHGetSpecialFolderLocation);HOOK(SHGetPathFromIDListA);HOOK(RegOpenKeyExA);HOOK(RegCreateKeyExA);HOOK(RegDeleteKeyA);
    HOOK(RegDeleteKeyExA);HOOK(RegDeleteValueA);HOOK(RegSetValueExA);HOOK(RegQueryValueExA);HOOK(RegEnumKeyA);HOOK(RegEnumValueA);
    HOOK(RegCloseKey);HOOK(LoadLibraryA);HOOK(LoadLibraryExA);HOOK(FreeLibrary);HOOK(GetProcAddress);
#undef HOOK
    return nullptr;
}
}

extern "C" __declspec(dllexport) DWORD WINAPI FarmarSandboxSetup(void* parameter) {
    const auto* config=static_cast<const FarmarGuardConfig*>(parameter);
    if(!config||config->version!=1||!config->privateCu||!config->privateLm||!std::memchr(config->root,0,MAX_PATH))return 1;
    root=config->root;while(!root.empty()&&root.back()=='\\')root.pop_back();if(root.size()<4)return 2;
    char shortPath[MAX_PATH]{};if(GetShortPathNameA(root.c_str(),shortPath,MAX_PATH))shortRoot=shortPath;
    InitializeCriticalSection(&lock);
    logFile=CreateFileA((root+"\\guard-events.jsonl").c_str(),GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);if(logFile==INVALID_HANDLE_VALUE)return 3;
    const auto cu=reinterpret_cast<HKEY>(config->privateCu),lm=reinterpret_cast<HKEY>(config->privateLm);remember(cu);remember(lm);
    if(RegOverridePredefKey(HKEY_CURRENT_USER,cu)!=ERROR_SUCCESS||RegOverridePredefKey(HKEY_LOCAL_MACHINE,lm)!=ERROR_SUCCESS)return 4;
    const auto temp=root+"\\temp";if(!CreateDirectoryA(temp.c_str(),nullptr)&&GetLastError()!=ERROR_ALREADY_EXISTS)return 5;
    if(!SetEnvironmentVariableA("TEMP",temp.c_str())||!SetEnvironmentVariableA("TMP",temp.c_str()))return 6;
    if(!patch(GetModuleHandleW(nullptr)))return 7;
    event("setup-before-application-entry",root.c_str(),true);return farmarGuardReady;
}

BOOL WINAPI DllMain(HINSTANCE,DWORD,void*) {return TRUE;}
