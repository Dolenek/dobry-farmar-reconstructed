// Diagnostic containment proof only. No uninstall program is executed.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <aclapi.h>
#include <sddl.h>
#include <objbase.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstring>
namespace fs=std::filesystem;
namespace {
void require(bool ok,const char* text){if(!ok)throw std::runtime_error(std::string(text)+" Win32="+std::to_string(GetLastError()));}
void access(HANDLE object,SE_OBJECT_TYPE type,PSID sid,DWORD rights) {
 PACL oldAcl=nullptr,newAcl=nullptr;PSECURITY_DESCRIPTOR sd=nullptr;
 auto error=GetSecurityInfo(object,type,DACL_SECURITY_INFORMATION,nullptr,nullptr,&oldAcl,nullptr,&sd);require(error==ERROR_SUCCESS,"Read owned ACL");
 EXPLICIT_ACCESSW entry{};entry.grfAccessPermissions=rights;entry.grfAccessMode=GRANT_ACCESS;entry.grfInheritance=SUB_CONTAINERS_AND_OBJECTS_INHERIT;
 entry.Trustee.TrusteeForm=TRUSTEE_IS_SID;entry.Trustee.TrusteeType=TRUSTEE_IS_UNKNOWN;entry.Trustee.ptstrName=reinterpret_cast<wchar_t*>(sid);
 error=SetEntriesInAclW(1,&entry,oldAcl,&newAcl);require(error==ERROR_SUCCESS,"Extend owned ACL");error=SetSecurityInfo(object,type,DACL_SECURITY_INFORMATION,nullptr,nullptr,newAcl,nullptr);LocalFree(newAcl);LocalFree(sd);require(error==ERROR_SUCCESS,"Set owned ACL");
}
std::string read(const fs::path& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
void marker(const fs::path& p){HANDLE f=CreateFileW(p.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);require(f!=INVALID_HANDLE_VALUE,"Create owned marker");DWORD n;require(WriteFile(f,"initial",7,&n,nullptr)&&n==7,"Write owned marker");CloseHandle(f);}
}
int wmain(int argc,wchar_t** argv){
 if(argc!=2)return 2;SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
 HANDLE token=nullptr,restricted=nullptr;PSID sid=nullptr;std::wstring registry;bool impersonating=false;
 try {
  const auto root=fs::absolute(argv[1]).lexically_normal();require(!fs::exists(root),"Probe directory must be new");require(root.has_parent_path()&&fs::is_directory(root.parent_path()),"Probe parent must exist");fs::create_directory(root);
  GUID id{};require(SUCCEEDED(CoCreateGuid(&id)),"Create probe id");wchar_t guid[40]{};StringFromGUID2(id,guid,40);
  wchar_t sidName[160]{};DWORD a=id.Data1,b=(DWORD(id.Data2)<<16)|id.Data3,c=0,d=0;std::memcpy(&c,id.Data4,4);std::memcpy(&d,id.Data4+4,4);swprintf_s(sidName,L"S-1-5-21-%u-%u-%u-%u",a,b,c,d);
  require(ConvertStringSidToSidW(sidName,&sid)!=FALSE,"Create unique restricting SID");
  HANDLE directory=CreateFileW(root.c_str(),READ_CONTROL|WRITE_DAC,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,nullptr);require(directory!=INVALID_HANDLE_VALUE,"Open owned test directory");access(directory,SE_FILE_OBJECT,sid,FILE_ALL_ACCESS);CloseHandle(directory);
  const auto inside=root/L"inside.txt",outside=root.parent_path()/(std::wstring(L"outside-")+guid+L".txt");marker(inside);marker(outside);
  registry=std::wstring(L"Software\\FarmarReconstruction\\UninstallProbe\\")+guid;
  HKEY key=nullptr;require(RegCreateKeyExW(HKEY_CURRENT_USER,(registry+L"\\inside").c_str(),0,nullptr,0,KEY_ALL_ACCESS,nullptr,&key,nullptr)==ERROR_SUCCESS,"Create private inside registry key");
  access(key,SE_REGISTRY_KEY,sid,KEY_ALL_ACCESS);DWORD value=1;require(RegSetValueExW(key,L"marker",0,REG_DWORD,reinterpret_cast<const BYTE*>(&value),4)==ERROR_SUCCESS,"Set inside registry marker");RegCloseKey(key);
  require(RegCreateKeyExW(HKEY_CURRENT_USER,(registry+L"\\outside").c_str(),0,nullptr,0,KEY_ALL_ACCESS,nullptr,&key,nullptr)==ERROR_SUCCESS,"Create private outside registry key");require(RegSetValueExW(key,L"marker",0,REG_DWORD,reinterpret_cast<const BYTE*>(&value),4)==ERROR_SUCCESS,"Set outside registry marker");RegCloseKey(key);
  require(OpenProcessToken(GetCurrentProcess(),TOKEN_DUPLICATE|TOKEN_QUERY|TOKEN_IMPERSONATE|TOKEN_ASSIGN_PRIMARY,&token)!=FALSE,"Open own token");
  BYTE adminBuffer[SECURITY_MAX_SID_SIZE];DWORD adminSize=sizeof(adminBuffer);require(CreateWellKnownSid(WinBuiltinAdministratorsSid,nullptr,adminBuffer,&adminSize)!=FALSE,"Administrators SID");
  BYTE packages[SECURITY_MAX_SID_SIZE];DWORD packageSize=sizeof(packages);
  wchar_t domain[256]{};DWORD domainLength=256;SID_NAME_USE use{};
  // Restricting SIDs never add normal rights: both access checks must pass.
  // Protected system files allow TrustedInstaller, while user workspace files
  // do not; intersect this with the caller's existing read-only system access.
  require(LookupAccountNameW(nullptr,L"NT SERVICE\\TrustedInstaller",packages,&packageSize,domain,&domainLength,&use)!=FALSE,"Resolve system read restricting SID");
  SID_AND_ATTRIBUTES disabled{adminBuffer,0};SID_AND_ATTRIBUTES limited[]={{sid,0},{packages,0}};
  require(CreateRestrictedToken(token,DISABLE_MAX_PRIVILEGE,1,&disabled,0,nullptr,2,limited,&restricted)!=FALSE,"Create fully restricted token");require(IsTokenRestricted(restricted)!=FALSE,"Token is not restricted");
  require(ImpersonateLoggedOnUser(restricted)!=FALSE,"Use restricted token");impersonating=true;
  HANDLE f=CreateFileW(inside.c_str(),GENERIC_WRITE,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);require(f!=INVALID_HANDLE_VALUE,"Inside file write permission");DWORD bytes;require(WriteFile(f,"changed",7,&bytes,nullptr)&&bytes==7,"Inside actual write");CloseHandle(f);
  f=CreateFileW(outside.c_str(),GENERIC_WRITE|DELETE|WRITE_DAC,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);DWORD outsideError=GetLastError();if(f!=INVALID_HANDLE_VALUE)CloseHandle(f);require(f==INVALID_HANDLE_VALUE&&outsideError==ERROR_ACCESS_DENIED,"Outside file mutation was not denied");
  for(DWORD mask:{DWORD(DELETE),DWORD(WRITE_DAC),DWORD(WRITE_OWNER)}) {
   f=CreateFileW(outside.c_str(),mask,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);DWORD error=GetLastError();if(f!=INVALID_HANDLE_VALUE)CloseHandle(f);require(f==INVALID_HANDLE_VALUE&&error==ERROR_ACCESS_DENIED,"Outside independent delete/DACL/owner access was not denied");
  }
  BOOL deleted=DeleteFileW(outside.c_str());DWORD deleteError=GetLastError();require(!deleted&&deleteError==ERROR_ACCESS_DENIED,"Outside file deletion was not denied");
  f=CreateFileW(outside.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);bool outsideReadable=f!=INVALID_HANDLE_VALUE;if(outsideReadable)CloseHandle(f);
  wchar_t system[MAX_PATH];GetSystemDirectoryW(system,MAX_PATH);fs::path systemFile=fs::path(system)/L"kernel32.dll";
  f=CreateFileW(systemFile.c_str(),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);require(f!=INVALID_HANDLE_VALUE,"Read installed system runtime");CloseHandle(f);
  auto status=RegOpenKeyExW(HKEY_CURRENT_USER,(registry+L"\\inside").c_str(),0,KEY_SET_VALUE,&key);require(status==ERROR_SUCCESS,"Inside registry write permission");value=2;require(RegSetValueExW(key,L"marker",0,REG_DWORD,reinterpret_cast<const BYTE*>(&value),4)==ERROR_SUCCESS,"Inside actual registry write");RegCloseKey(key);
  status=RegOpenKeyExW(HKEY_CURRENT_USER,(registry+L"\\outside").c_str(),0,KEY_SET_VALUE|DELETE|WRITE_DAC,&key);if(status==ERROR_SUCCESS)RegCloseKey(key);require(status==ERROR_ACCESS_DENIED,"Outside registry mutation was not denied");
  require(RevertToSelf()!=FALSE,"Restore harness token");impersonating=false;require(read(inside)=="changed"&&read(outside)=="initial","Marker contents differ");
  for(const auto name:{L"inside",L"outside"}) {require(RegOpenKeyExW(HKEY_CURRENT_USER,(registry+L"\\"+name).c_str(),0,KEY_QUERY_VALUE,&key)==ERROR_SUCCESS,"Read private registry marker");DWORD actual=0,length=4;require(RegQueryValueExW(key,L"marker",nullptr,nullptr,reinterpret_cast<BYTE*>(&actual),&length)==ERROR_SUCCESS,"Read registry marker value");RegCloseKey(key);require(actual==(name==std::wstring(L"inside") ? 2U:1U),"Registry marker changed unexpectedly");}
  require(RegDeleteTreeW(HKEY_CURRENT_USER,registry.c_str())==ERROR_SUCCESS,"Remove own probe registry keys");registry.clear();
  std::cout<<"{\"status\":\"passed_for_fully_restricted_impersonation_only\",\"inside_file_write\":true,\"outside_file_write_denied\":true,\"outside_file_delete_denied\":true,\"outside_dacl_owner_mutations_denied\":true,\"system_runtime_read\":true,\"inside_registry_write\":true,\"outside_registry_write_denied\":true,\"outside_markers_unchanged\":true,\"private_registry_keys_removed\":true,\"uninstaller_executed\":false}\n";
 }catch(const std::exception& e){if(impersonating)RevertToSelf();if(!registry.empty()&&registry.find(L"Software\\FarmarReconstruction\\UninstallProbe\\")==0)RegDeleteTreeW(HKEY_CURRENT_USER,registry.c_str());std::cerr<<e.what()<<'\n';if(restricted)CloseHandle(restricted);if(token)CloseHandle(token);if(sid)LocalFree(sid);return 1;}
 if(restricted)CloseHandle(restricted);if(token)CloseHandle(token);if(sid)LocalFree(sid);return 0;
}
