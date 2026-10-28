// Diagnostic only. Default mode runs marker probes; explicit --uninstall-quiet
// compares a supplied reference/source copy under the owned entry guards.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "entry_injection.h"
#include "uninstall_fixtures.h"
#include "ui_driver.h"
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
#include <algorithm>
namespace fs=std::filesystem;
namespace {
constexpr DWORD desktopAccess=STANDARD_RIGHTS_REQUIRED|DESKTOP_READOBJECTS|DESKTOP_CREATEWINDOW|DESKTOP_CREATEMENU|DESKTOP_HOOKCONTROL|DESKTOP_JOURNALRECORD|DESKTOP_JOURNALPLAYBACK|DESKTOP_ENUMERATE|DESKTOP_WRITEOBJECTS|DESKTOP_SWITCHDESKTOP;
void require(bool ok,const char* text){if(!ok)throw std::runtime_error(std::string(text)+" Win32="+std::to_string(GetLastError()));}
void access(HANDLE object,SE_OBJECT_TYPE type,PSID sid,DWORD rights) {
 PACL oldAcl=nullptr,newAcl=nullptr;PSECURITY_DESCRIPTOR sd=nullptr;
 auto error=GetSecurityInfo(object,type,DACL_SECURITY_INFORMATION,nullptr,nullptr,&oldAcl,nullptr,&sd);require(error==ERROR_SUCCESS,"Read owned ACL");
 EXPLICIT_ACCESSW entry{};entry.grfAccessPermissions=rights;entry.grfAccessMode=GRANT_ACCESS;entry.grfInheritance=type==SE_WINDOW_OBJECT ? NO_INHERITANCE:SUB_CONTAINERS_AND_OBJECTS_INHERIT;
 entry.Trustee.TrusteeForm=TRUSTEE_IS_SID;entry.Trustee.TrusteeType=TRUSTEE_IS_UNKNOWN;entry.Trustee.ptstrName=reinterpret_cast<wchar_t*>(sid);
 error=SetEntriesInAclW(1,&entry,oldAcl,&newAcl);require(error==ERROR_SUCCESS,"Extend owned ACL");error=SetSecurityInfo(object,type,DACL_SECURITY_INFORMATION,nullptr,nullptr,newAcl,nullptr);LocalFree(newAcl);LocalFree(sd);require(error==ERROR_SUCCESS,"Set owned ACL");
}
void restoreStationAcl(HWINSTA station,PACL original) {
 require(SetSecurityInfo(station,SE_WINDOW_OBJECT,DACL_SECURITY_INFORMATION,nullptr,nullptr,original,nullptr)==ERROR_SUCCESS,"Restore noninteractive station ACL");
 PACL actual=nullptr;PSECURITY_DESCRIPTOR descriptor=nullptr;require(GetSecurityInfo(station,SE_WINDOW_OBJECT,DACL_SECURITY_INFORMATION,nullptr,nullptr,&actual,nullptr,&descriptor)==ERROR_SUCCESS,"Read restored station ACL");
 const bool exact=actual&&original&&actual->AclSize==original->AclSize&&!std::memcmp(actual,original,original->AclSize);LocalFree(descriptor);require(exact,"Restored station ACL differs from original");
}
std::string read(const fs::path& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
void marker(const fs::path& p){HANDLE f=CreateFileW(p.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);require(f!=INVALID_HANDLE_VALUE,"Create owned marker");DWORD n;require(WriteFile(f,"initial",7,&n,nullptr)&&n==7,"Write owned marker");CloseHandle(f);}

int childProbe(const fs::path& root,const fs::path& outside,const std::wstring& expectedStation,const std::wstring& expectedDesktop,HKEY privateCu,HKEY privateLm) {
 try {
  require(RegOverridePredefKey(HKEY_CURRENT_USER,privateCu)==ERROR_SUCCESS,"Child private HKCU mapping");
  require(RegOverridePredefKey(HKEY_LOCAL_MACHINE,privateLm)==ERROR_SUCCESS,"Child private HKLM mapping");
  for(auto hive:{HKEY_CURRENT_USER,HKEY_LOCAL_MACHINE}) {HKEY key=nullptr;require(RegCreateKeyExW(hive,L"Software\\FarmarContainedChild",0,nullptr,0,KEY_ALL_ACCESS,nullptr,&key,nullptr)==ERROR_SUCCESS,"Child redirected registry create");DWORD value=42;require(RegSetValueExW(key,L"proof",0,REG_DWORD,reinterpret_cast<BYTE*>(&value),4)==ERROR_SUCCESS,"Child redirected registry write");RegCloseKey(key);}
  HKEY realCu=nullptr;const auto outsideRegistry=RegOpenCurrentUser(KEY_SET_VALUE|DELETE|WRITE_DAC,&realCu);if(outsideRegistry==ERROR_SUCCESS)RegCloseKey(realCu);require(outsideRegistry==ERROR_ACCESS_DENIED,"Child original HKCU mutations must be denied");
  HANDLE token=nullptr;require(OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token)!=FALSE,"Child token query");require(IsTokenRestricted(token)!=FALSE,"Child primary token must be restricted");DWORD labelLength=0;GetTokenInformation(token,TokenIntegrityLevel,nullptr,0,&labelLength);std::vector<BYTE> labelData(labelLength);require(GetTokenInformation(token,TokenIntegrityLevel,labelData.data(),labelLength,&labelLength)!=FALSE,"Child integrity query");auto label=reinterpret_cast<TOKEN_MANDATORY_LABEL*>(labelData.data());require(*GetSidSubAuthority(label->Label.Sid,*GetSidSubAuthorityCount(label->Label.Sid)-1)==SECURITY_MANDATORY_UNTRUSTED_RID,"Child must be untrusted integrity");CloseHandle(token);BOOL inJob=FALSE;require(IsProcessInJob(GetCurrentProcess(),nullptr,&inJob)&&inJob,"Child must be in containment Job");
  wchar_t station[256]{},desktop[256]{};DWORD length=0;
  require(GetUserObjectInformationW(GetProcessWindowStation(),UOI_NAME,station,sizeof(station),&length)!=FALSE,"Child window station query");
  require(GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&length)!=FALSE,"Child desktop query");
  require(std::wstring(station)==expectedStation && expectedStation!=L"WinSta0" && std::wstring(desktop)==expectedDesktop && expectedDesktop.find(L"Contained-")==0,"Child must own private station/desktop");
  marker(root/L"child-inside.txt");
  for(const auto& guard:{outside,fs::path(outside.wstring()+L".low")}) for(DWORD mask:{DWORD(GENERIC_WRITE),DWORD(DELETE),DWORD(WRITE_DAC),DWORD(WRITE_OWNER)}) {
   HANDLE f=CreateFileW(guard.c_str(),mask,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);DWORD error=GetLastError();if(f!=INVALID_HANDLE_VALUE)CloseHandle(f);require(f==INVALID_HANDLE_VALUE&&error==ERROR_ACCESS_DENIED,"Child outside mutation permission was not denied");
  }
  for(const auto& guard:{outside,fs::path(outside.wstring()+L".low")})require(!DeleteFileW(guard.c_str())&&GetLastError()==ERROR_ACCESS_DENIED,"Child outside medium/low integrity deletion was not denied");
  WNDCLASSW cls{};cls.hInstance=GetModuleHandleW(nullptr);cls.lpfnWndProc=&DefWindowProcW;cls.lpszClassName=L"FarmarContainedProbe";require(RegisterClassW(&cls)!=0,"Child private window class");
  HWND window=CreateWindowExW(0,cls.lpszClassName,L"Contained source probe",WS_OVERLAPPED,0,0,20,20,nullptr,nullptr,cls.hInstance,nullptr);require(window!=nullptr,"Child private USER32 window");DestroyWindow(window);UnregisterClassW(cls.lpszClassName,cls.hInstance);
  HANDLE result=CreateFileW((root/L"child-proof.txt").c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);require(result!=INVALID_HANDLE_VALUE,"Child result inside sandbox");DWORD written;constexpr char proof[]="restricted-primary-private-desktop-and-hives";require(WriteFile(result,proof,sizeof(proof)-1,&written,nullptr)&&written==sizeof(proof)-1,"Child result write");CloseHandle(result);
  return 0;
 }catch(const std::exception& error){std::ofstream(root/L"child-error.txt")<<error.what();return 11;}
}
void childProcessProbe(const fs::path& root,const fs::path& outside,HANDLE restricted,PSID sid,const wchar_t* identifier,const std::wstring& registry,bool guardedClient=false,bool uninstallApp=false,const std::wstring& scenario=L"empty-fallback",const std::wstring& uiAction=L"") {
 HWINSTA old=GetProcessWindowStation(),station=nullptr;HDESK desktop=nullptr;PSECURITY_DESCRIPTOR stationSd=nullptr;PACL stationAcl=nullptr;HANDLE job=nullptr;HKEY privateCu=nullptr,privateLm=nullptr;LPPROC_THREAD_ATTRIBUTE_LIST attributes=nullptr;std::vector<BYTE> attributeData;PROCESS_INFORMATION process{};
 try {
  std::wstring stationName;
  station=CreateWindowStationW(nullptr,0,WINSTA_ALL_ACCESS|READ_CONTROL|WRITE_DAC,nullptr);require(station!=nullptr,"Open noninteractive test station");require(GetSecurityInfo(station,SE_WINDOW_OBJECT,DACL_SECURITY_INFORMATION,nullptr,nullptr,&stationAcl,nullptr,&stationSd)==ERROR_SUCCESS,"Save noninteractive station ACL");access(station,SE_WINDOW_OBJECT,sid,WINSTA_ALL_ACCESS|STANDARD_RIGHTS_REQUIRED);wchar_t actualStation[256]{};DWORD nameLength=0;require(GetUserObjectInformationW(station,UOI_NAME,actualStation,sizeof(actualStation),&nameLength)!=FALSE,"Private station name");stationName=actualStation;require(stationName!=L"WinSta0","Must not use interactive station");
  require(SetProcessWindowStation(station)!=FALSE,"Select private station for desktop creation");const std::wstring privateDesktop=std::wstring(L"Contained-")+identifier;desktop=CreateDesktopW(privateDesktop.c_str(),nullptr,nullptr,0,desktopAccess,nullptr);require(desktop!=nullptr,"Create private test desktop");access(desktop,SE_WINDOW_OBJECT,sid,desktopAccess);
  PSECURITY_DESCRIPTOR labelDesc=nullptr;require(ConvertStringSecurityDescriptorToSecurityDescriptorW(L"S:(ML;;NW;;;S-1-16-0)",SDDL_REVISION_1,&labelDesc,nullptr)!=FALSE,"Private desktop integrity descriptor");BOOL exists=FALSE,def=FALSE;PACL labelAcl=nullptr;require(GetSecurityDescriptorSacl(labelDesc,&exists,&labelAcl,&def)!=FALSE&&exists,"Private desktop label");require(SetSecurityInfo(desktop,SE_WINDOW_OBJECT,LABEL_SECURITY_INFORMATION,nullptr,nullptr,nullptr,labelAcl)==ERROR_SUCCESS,"Private desktop untrusted label");LocalFree(labelDesc);
  wchar_t own[MAX_PATH]{};require(GetModuleFileNameW(nullptr,own,MAX_PATH)!=0,"Own source probe path");const auto child=(uninstallApp?root/L"runner"/L"Uninstall.exe":root/(guardedClient?L"uninstall-guard-client.exe":L"probe-child.exe"));if(!guardedClient&&!uninstallApp)require(CopyFileW(own,child.c_str(),TRUE)!=FALSE,"Copy own probe into granted sandbox");
  job=CreateJobObjectW(nullptr,nullptr);require(job!=nullptr,"Create contained child Job");JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE|JOB_OBJECT_LIMIT_ACTIVE_PROCESS|JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION;limits.BasicLimitInformation.ActiveProcessLimit=1;require(SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits))!=FALSE,"Set contained child Job limits");
  require(RegCreateKeyExW(HKEY_CURRENT_USER,(registry+L"\\inside\\HKCU").c_str(),0,nullptr,0,KEY_ALL_ACCESS,nullptr,&privateCu,nullptr)==ERROR_SUCCESS,"Create private child HKCU");require(RegCreateKeyExW(HKEY_CURRENT_USER,(registry+L"\\inside\\HKLM").c_str(),0,nullptr,0,KEY_ALL_ACCESS,nullptr,&privateLm,nullptr)==ERROR_SUCCESS,"Create private child HKLM");
  if(uninstallApp)seedUninstallFixture(root,privateCu,privateLm,scenario);
  HANDLE inherited[]={privateCu,privateLm};for(auto handle:inherited)require(SetHandleInformation(handle,HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT)!=FALSE,"Whitelist private hive handle inheritance");SIZE_T attributeSize=0;InitializeProcThreadAttributeList(nullptr,1,0,&attributeSize);attributeData.resize(attributeSize);attributes=reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attributeData.data());require(InitializeProcThreadAttributeList(attributes,1,0,&attributeSize)!=FALSE,"Child handle list attributes");require(UpdateProcThreadAttribute(attributes,0,PROC_THREAD_ATTRIBUTE_HANDLE_LIST,inherited,sizeof(inherited),nullptr,nullptr)!=FALSE,"Only private hive handles inherit");
  std::wstring command=L"\""+child.wstring()+L"\" --child-file-probe \""+root.wstring()+L"\" \""+outside.wstring()+L"\" \""+stationName+L"\" \""+privateDesktop+L"\" "+std::to_wstring(reinterpret_cast<uintptr_t>(privateCu))+L" "+std::to_wstring(reinterpret_cast<uintptr_t>(privateLm));
  if(guardedClient)command=L"\""+child.wstring()+L"\" \""+root.wstring()+L"\" \""+outside.wstring()+L".untrusted\"";
  if(uninstallApp)command=L"\""+child.wstring()+L"\" "+(uiAction.empty()?L"/S ":L"")+L"_?="+(root/L"payload").wstring();
  std::wstring desktopName=stationName+L"\\"+privateDesktop;STARTUPINFOEXW startup{};startup.StartupInfo.cb=sizeof(startup);startup.StartupInfo.lpDesktop=desktopName.data();startup.StartupInfo.dwFlags=STARTF_USESHOWWINDOW;startup.StartupInfo.wShowWindow=uiAction.empty()?SW_HIDE:SW_SHOWNORMAL;startup.lpAttributeList=attributes;
  std::vector<wchar_t> environment;
  if(uninstallApp) {
   std::vector<std::wstring> entries;auto* inheritedEnvironment=GetEnvironmentStringsW();require(inheritedEnvironment!=nullptr,"Read own environment for isolated child");
   for(auto* item=inheritedEnvironment;*item;item+=wcslen(item)+1)if(_wcsnicmp(item,L"__COMPAT_LAYER=",15)&&_wcsnicmp(item,L"TEMP=",5)&&_wcsnicmp(item,L"TMP=",4))entries.emplace_back(item);FreeEnvironmentStringsW(inheritedEnvironment);
   entries.emplace_back(L"__COMPAT_LAYER=RunAsInvoker");entries.emplace_back(L"TEMP="+(root/L"temp").wstring());entries.emplace_back(L"TMP="+(root/L"temp").wstring());std::sort(entries.begin(),entries.end(),[](const auto& a,const auto& b){return _wcsicmp(a.c_str(),b.c_str())<0;});
   for(const auto& item:entries){environment.insert(environment.end(),item.begin(),item.end());environment.push_back(0);}environment.push_back(0);
  }
  require(CreateProcessAsUserW(restricted,child.c_str(),command.data(),nullptr,nullptr,TRUE,EXTENDED_STARTUPINFO_PRESENT|CREATE_SUSPENDED|CREATE_NO_WINDOW|DEBUG_ONLY_THIS_PROCESS|CREATE_UNICODE_ENVIRONMENT,environment.empty()?nullptr:environment.data(),root.c_str(),&startup.StartupInfo,&process)!=FALSE,"Create own child under restricted primary token");
  DeleteProcThreadAttributeList(attributes);attributes=nullptr;RegCloseKey(privateCu);privateCu=nullptr;RegCloseKey(privateLm);privateLm=nullptr;
  require(AssignProcessToJobObject(job,process.hProcess)!=FALSE,"Assign owned restricted child to Job");
  if(guardedClient||uninstallApp) {std::function<void()> driver;if(!uiAction.empty())driver=[&]{driveUninstallUi(process,desktop,root,uiAction);};const auto code=runGuardedEntry(process,root,inherited[0],inherited[1],driver);std::ofstream(root/L"guarded-exit-code.json")<<"{\"exit_code\":"<<code<<"}\n";require(code==0||(!uiAction.empty()&&uiAction!=L"complete"&&code<=2),(std::string("Owned guard client exit ")+std::to_string(code)).c_str());if(guardedClient)require(read(root/L"guard-client-proof.txt")=="guarded-before-entry","Guard client proof mismatch");if(uninstallApp){HKEY cu=nullptr,lm=nullptr;require(RegOpenKeyExW(HKEY_CURRENT_USER,(registry+L"\\inside\\HKCU").c_str(),0,KEY_READ,&cu)==ERROR_SUCCESS,"Verify private uninstall HKCU");require(RegOpenKeyExW(HKEY_CURRENT_USER,(registry+L"\\inside\\HKLM").c_str(),0,KEY_READ,&lm)==ERROR_SUCCESS,"Verify private uninstall HKLM");try{if(uiAction.empty()||uiAction==L"complete")verifyUninstallFixture(root,cu,lm,scenario);else verifyCancelledUninstallFixture(root,cu,lm,scenario);}catch(...){RegCloseKey(cu);RegCloseKey(lm);throw;}RegCloseKey(cu);RegCloseKey(lm);}}
  else {require(ResumeThread(process.hThread)!=DWORD(-1),"Resume owned source child");
  std::ofstream loader(root/L"child-loader.txt");DWORD code=STILL_ACTIVE;const auto deadline=GetTickCount64()+20000;
  while(code==STILL_ACTIVE&&GetTickCount64()<deadline) {
   DEBUG_EVENT e{};if(!WaitForDebugEvent(&e,1000)){require(GetLastError()==ERROR_SEM_TIMEOUT,"Wait child diagnostic event");continue;}DWORD action=DBG_CONTINUE;
   if(e.dwDebugEventCode==CREATE_PROCESS_DEBUG_EVENT) {
    if(e.u.CreateProcessInfo.hFile)CloseHandle(e.u.CreateProcessInfo.hFile);
   } else if(e.dwDebugEventCode==LOAD_DLL_DEBUG_EVENT) {
    if(e.u.LoadDll.hFile){wchar_t name[1024]{};GetFinalPathNameByHandleW(e.u.LoadDll.hFile,name,1024,0);loader<<"DLL ";for(auto c:std::wstring(name))loader<<char(c<128?c:'?');loader<<'\n';CloseHandle(e.u.LoadDll.hFile);}
   } else if(e.dwDebugEventCode==OUTPUT_DEBUG_STRING_EVENT) {
    const auto& value=e.u.DebugString;std::vector<char> data(size_t(value.nDebugStringLength)*(value.fUnicode?2:1)+2);SIZE_T count=0;
    if(ReadProcessMemory(process.hProcess,value.lpDebugStringData,data.data(),data.size()-2,&count)) {if(value.fUnicode){for(auto c:std::wstring(reinterpret_cast<wchar_t*>(data.data())))loader<<char(c<128?c:'?');}else loader<<data.data();}loader.flush();
   } else if(e.dwDebugEventCode==EXCEPTION_DEBUG_EVENT) {
    loader<<"EXCEPTION "<<std::hex<<e.u.Exception.ExceptionRecord.ExceptionCode<<" at "<<e.u.Exception.ExceptionRecord.ExceptionAddress<<std::dec<<'\n';if(e.u.Exception.ExceptionRecord.ExceptionCode!=EXCEPTION_BREAKPOINT)action=DBG_EXCEPTION_NOT_HANDLED;
   } else if(e.dwDebugEventCode==EXIT_PROCESS_DEBUG_EVENT)code=e.u.ExitProcess.dwExitCode;
   require(ContinueDebugEvent(e.dwProcessId,e.dwThreadId,action)!=FALSE,"Continue child diagnostic event");
  }
  require(code!=STILL_ACTIVE,"Owned source child did not finish");require(code==0,(std::string("Restricted source child failed with exit ")+std::to_string(code)).c_str());
  require(read(root/L"child-proof.txt")=="restricted-primary-private-desktop-and-hives","Child proof result differs");require(read(root/L"child-inside.txt")=="initial"&&read(outside)=="initial"&&read(fs::path(outside.wstring()+L".low"))=="initial","Child marker results differ");
  }
  CloseHandle(process.hThread);CloseHandle(process.hProcess);process={};CloseHandle(job);job=nullptr;require(SetProcessWindowStation(old)!=FALSE,"Restore harness window station");CloseDesktop(desktop);desktop=nullptr;restoreStationAcl(station,stationAcl);LocalFree(stationSd);stationSd=nullptr;CloseWindowStation(station);station=nullptr;
 }catch(...){
  if(attributes)DeleteProcThreadAttributeList(attributes);if(privateCu)RegCloseKey(privateCu);if(privateLm)RegCloseKey(privateLm);
  if(job)TerminateJobObject(job,0xee);else if(process.hProcess)TerminateProcess(process.hProcess,0xee);
  if(process.hProcess)WaitForSingleObject(process.hProcess,5000);if(process.hThread)CloseHandle(process.hThread);if(process.hProcess)CloseHandle(process.hProcess);if(job)CloseHandle(job);
  SetProcessWindowStation(old);if(desktop)CloseDesktop(desktop);if(station&&stationSd){try{restoreStationAcl(station,stationAcl);}catch(const std::exception& e){std::cerr<<e.what()<<'\n';}}if(stationSd)LocalFree(stationSd);if(station)CloseWindowStation(station);throw;
 }
}
}
int wmain(int argc,wchar_t** argv){
 SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
 if(argc==8 && std::wstring(argv[1])==L"--child-file-probe")return childProbe(argv[2],argv[3],argv[4],argv[5],reinterpret_cast<HKEY>(std::stoul(argv[6])),reinterpret_cast<HKEY>(std::stoul(argv[7])));
 const bool guardedClient=argc==4&&std::wstring(argv[1])==L"--guard-client";
 const bool uiMode=argc==7&&std::wstring(argv[1])==L"--uninstall-ui";
 const bool uninstallApp=uiMode||((argc==5||argc==6)&&std::wstring(argv[1])==L"--uninstall-quiet");
 const bool extra=guardedClient||uninstallApp;
 if(argc!=2&&!extra)return 2;
 HANDLE token=nullptr,restricted=nullptr;PSID sid=nullptr;std::wstring registry;bool impersonating=false;
 try {
  const auto root=fs::absolute(argv[extra?2:1]).lexically_normal();require(!fs::exists(root),"Probe directory must be new");require(root.has_parent_path()&&fs::is_directory(root.parent_path()),"Probe parent must exist");fs::create_directory(root);
  GUID id{};require(SUCCEEDED(CoCreateGuid(&id)),"Create probe id");wchar_t guid[40]{};StringFromGUID2(id,guid,40);
  wchar_t sidName[160]{};DWORD a=id.Data1,b=(DWORD(id.Data2)<<16)|id.Data3,c=0,d=0;std::memcpy(&c,id.Data4,4);std::memcpy(&d,id.Data4+4,4);swprintf_s(sidName,L"S-1-5-21-%u-%u-%u-%u",a,b,c,d);
  require(ConvertStringSidToSidW(sidName,&sid)!=FALSE,"Create unique restricting SID");
  HANDLE directory=CreateFileW(root.c_str(),READ_CONTROL|WRITE_DAC,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,nullptr);require(directory!=INVALID_HANDLE_VALUE,"Open owned test directory");access(directory,SE_FILE_OBJECT,sid,FILE_ALL_ACCESS);CloseHandle(directory);
  const auto inside=root/L"inside.txt",outside=root.parent_path()/(std::wstring(L"outside-")+guid+L".txt");marker(inside);marker(outside);const fs::path outsideLow=outside.wstring()+L".low";marker(outsideLow);
  PSECURITY_DESCRIPTOR lowGuardLabel=nullptr;require(ConvertStringSecurityDescriptorToSecurityDescriptorW(L"S:(ML;;NW;;;LW)",SDDL_REVISION_1,&lowGuardLabel,nullptr)!=FALSE,"Low integrity outside guard label");BOOL lowGuardPresent=FALSE,lowGuardDefault=FALSE;PACL lowGuardSacl=nullptr;require(GetSecurityDescriptorSacl(lowGuardLabel,&lowGuardPresent,&lowGuardSacl,&lowGuardDefault)&&lowGuardPresent,"Read own low integrity guard label");require(SetNamedSecurityInfoW(const_cast<wchar_t*>(outsideLow.c_str()),SE_FILE_OBJECT,LABEL_SECURITY_INFORMATION,nullptr,nullptr,nullptr,lowGuardSacl)==ERROR_SUCCESS,"Label own outside guard low integrity");LocalFree(lowGuardLabel);
  registry=std::wstring(L"Software\\FarmarReconstruction\\UninstallProbe\\")+guid;
  HKEY key=nullptr;require(RegCreateKeyExW(HKEY_CURRENT_USER,(registry+L"\\inside").c_str(),0,nullptr,0,KEY_ALL_ACCESS,nullptr,&key,nullptr)==ERROR_SUCCESS,"Create private inside registry key");
  access(key,SE_REGISTRY_KEY,sid,KEY_ALL_ACCESS);DWORD value=1;require(RegSetValueExW(key,L"marker",0,REG_DWORD,reinterpret_cast<const BYTE*>(&value),4)==ERROR_SUCCESS,"Set inside registry marker");RegCloseKey(key);
  require(RegCreateKeyExW(HKEY_CURRENT_USER,(registry+L"\\outside").c_str(),0,nullptr,0,KEY_ALL_ACCESS,nullptr,&key,nullptr)==ERROR_SUCCESS,"Create private outside registry key");require(RegSetValueExW(key,L"marker",0,REG_DWORD,reinterpret_cast<const BYTE*>(&value),4)==ERROR_SUCCESS,"Set outside registry marker");RegCloseKey(key);
  require(OpenProcessToken(GetCurrentProcess(),TOKEN_DUPLICATE|TOKEN_QUERY|TOKEN_IMPERSONATE|TOKEN_ASSIGN_PRIMARY|TOKEN_ADJUST_DEFAULT,&token)!=FALSE,"Open own token");
  BYTE adminBuffer[SECURITY_MAX_SID_SIZE];DWORD adminSize=sizeof(adminBuffer);require(CreateWellKnownSid(WinBuiltinAdministratorsSid,nullptr,adminBuffer,&adminSize)!=FALSE,"Administrators SID");
  // Builtin Users is needed by the Windows loader. It alone permits writes
  // through inherited workspace ACLs, so the token also uses Untrusted MIC.
  // Only this owned root, private hive and desktop receive Untrusted labels.
  BYTE restrictedCode[SECURITY_MAX_SID_SIZE];DWORD restrictedCodeSize=sizeof(restrictedCode);require(CreateWellKnownSid(WinRestrictedCodeSid,nullptr,restrictedCode,&restrictedCodeSize)!=FALSE,"Restricted code SID");
  BYTE appPackages[SECURITY_MAX_SID_SIZE];DWORD appPackagesSize=sizeof(appPackages);require(CreateWellKnownSid(WinBuiltinUsersSid,nullptr,appPackages,&appPackagesSize)!=FALSE,"Builtin Users restricting SID");
  SID_AND_ATTRIBUTES disabled{adminBuffer,0};SID_AND_ATTRIBUTES limited[]={{sid,0},{restrictedCode,0},{appPackages,0}};
  require(CreateRestrictedToken(token,DISABLE_MAX_PRIVILEGE,1,&disabled,0,nullptr,3,limited,&restricted)!=FALSE,"Create fully restricted token");require(IsTokenRestricted(restricted)!=FALSE,"Token is not restricted");
  DWORD defaultSize=0;GetTokenInformation(restricted,TokenDefaultDacl,nullptr,0,&defaultSize);std::vector<BYTE> defaultBuffer(defaultSize);require(GetTokenInformation(restricted,TokenDefaultDacl,defaultBuffer.data(),defaultSize,&defaultSize)!=FALSE,"Read child default DACL");EXPLICIT_ACCESSW defaultEntry{};defaultEntry.grfAccessPermissions=GENERIC_ALL;defaultEntry.grfAccessMode=GRANT_ACCESS;defaultEntry.Trustee.TrusteeForm=TRUSTEE_IS_SID;defaultEntry.Trustee.ptstrName=reinterpret_cast<wchar_t*>(sid);PACL childDefault=nullptr;require(SetEntriesInAclW(1,&defaultEntry,reinterpret_cast<TOKEN_DEFAULT_DACL*>(defaultBuffer.data())->DefaultDacl,&childDefault)==ERROR_SUCCESS,"Child default DACL entry");TOKEN_DEFAULT_DACL newDefault{childDefault};require(SetTokenInformation(restricted,TokenDefaultDacl,&newDefault,sizeof(newDefault))!=FALSE,"Grant child access to its own created kernel objects");LocalFree(childDefault);

  BYTE lowSid[SECURITY_MAX_SID_SIZE];DWORD lowSize=sizeof(lowSid);require(CreateWellKnownSid(WinUntrustedLabelSid,nullptr,lowSid,&lowSize)!=FALSE,"Untrusted integrity SID");TOKEN_MANDATORY_LABEL label{};label.Label.Sid=lowSid;label.Label.Attributes=SE_GROUP_INTEGRITY;require(SetTokenInformation(restricted,TokenIntegrityLevel,&label,sizeof(label)+GetLengthSid(lowSid))!=FALSE,"Set child untrusted integrity");
  PSECURITY_DESCRIPTOR integrity=nullptr;require(ConvertStringSecurityDescriptorToSecurityDescriptorW(L"S:(ML;OICI;NW;;;S-1-16-0)",SDDL_REVISION_1,&integrity,nullptr)!=FALSE,"Owned untrusted integrity label");BOOL present=FALSE,defaulted=FALSE;PACL sacl=nullptr;require(GetSecurityDescriptorSacl(integrity,&present,&sacl,&defaulted)!=FALSE&&present,"Read owned label");require(SetNamedSecurityInfoW(const_cast<wchar_t*>(root.c_str()),SE_FILE_OBJECT,LABEL_SECURITY_INFORMATION,nullptr,nullptr,nullptr,sacl)==ERROR_SUCCESS,"Label own sandbox directory untrusted");
  require(RegOpenKeyExW(HKEY_CURRENT_USER,(registry+L"\\inside").c_str(),0,WRITE_OWNER,&key)==ERROR_SUCCESS,"Own inside registry label handle");require(SetSecurityInfo(key,SE_REGISTRY_KEY,LABEL_SECURITY_INFORMATION,nullptr,nullptr,nullptr,sacl)==ERROR_SUCCESS,"Label own inside registry untrusted");RegCloseKey(key);LocalFree(integrity);

  if(extra) {
   const auto build=fs::absolute(argv[3]);for(const auto name:{L"uninstall-application-guard.dll",L"uninstall-guard-client.exe"})require(CopyFileW((build/name).c_str(),(root/name).c_str(),TRUE)!=FALSE,"Copy own guard diagnostic binary");
   if(uninstallApp){fs::create_directory(root/L"temp");fs::create_directory(root/L"runner");require(CopyFileW(fs::absolute(argv[4]).c_str(),(root/L"runner"/L"Uninstall.exe").c_str(),TRUE)!=FALSE,"Copy reference/source uninstaller into owned runner");}
   const fs::path untrustedGuard=outside.wstring()+L".untrusted";marker(untrustedGuard);PSECURITY_DESCRIPTOR guardLabel=nullptr;require(ConvertStringSecurityDescriptorToSecurityDescriptorW(L"S:(ML;;NW;;;S-1-16-0)",SDDL_REVISION_1,&guardLabel,nullptr)!=FALSE,"Outside Untrusted guard descriptor");BOOL present=FALSE,defaulted=FALSE;PACL sacl=nullptr;require(GetSecurityDescriptorSacl(guardLabel,&present,&sacl,&defaulted)&&present,"Outside Untrusted guard SACL");require(SetNamedSecurityInfoW(const_cast<wchar_t*>(untrustedGuard.c_str()),SE_FILE_OBJECT,LABEL_SECURITY_INFORMATION,nullptr,nullptr,nullptr,sacl)==ERROR_SUCCESS,"Label own outside Untrusted guard");LocalFree(guardLabel);
  }
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
  require(RevertToSelf()!=FALSE,"Restore harness token");impersonating=false;require(read(inside)=="changed"&&read(outside)=="initial"&&read(outsideLow)=="initial","Marker contents differ");
  for(const auto name:{L"inside",L"outside"}) {require(RegOpenKeyExW(HKEY_CURRENT_USER,(registry+L"\\"+name).c_str(),0,KEY_QUERY_VALUE,&key)==ERROR_SUCCESS,"Read private registry marker");DWORD actual=0,length=4;require(RegQueryValueExW(key,L"marker",nullptr,nullptr,reinterpret_cast<BYTE*>(&actual),&length)==ERROR_SUCCESS,"Read registry marker value");RegCloseKey(key);require(actual==(name==std::wstring(L"inside") ? 2U:1U),"Registry marker changed unexpectedly");}
  childProcessProbe(root,outside,restricted,sid,guid,registry);
  for(const auto name:{L"HKCU",L"HKLM"}) {HKEY proof=nullptr;require(RegOpenKeyExW(HKEY_CURRENT_USER,(registry+L"\\inside\\"+name+L"\\Software\\FarmarContainedChild").c_str(),0,KEY_QUERY_VALUE,&proof)==ERROR_SUCCESS,"Read child's redirected hive result");DWORD actual=0,length=4;require(RegQueryValueExW(proof,L"proof",nullptr,nullptr,reinterpret_cast<BYTE*>(&actual),&length)==ERROR_SUCCESS&&actual==42,"Child hive effect outside private key");RegCloseKey(proof);}
  if(guardedClient) {
   childProcessProbe(root,outside,restricted,sid,guid,registry,true);
   require(read(fs::path(outside.wstring()+L".untrusted"))=="initial","Application guard changed outside Untrusted marker");
   for(const auto name:{L"HKCU",L"HKLM"}) {HKEY proof=nullptr;require(RegOpenKeyExW(HKEY_CURRENT_USER,(registry+L"\\inside\\"+name+L"\\Software\\FarmarGuardClient").c_str(),0,KEY_QUERY_VALUE,&proof)==ERROR_SUCCESS,"Read guarded registry result");DWORD actual=0,length=4;require(RegQueryValueExW(proof,L"proof",nullptr,nullptr,reinterpret_cast<BYTE*>(&actual),&length)==ERROR_SUCCESS&&actual==73,"Guarded hive result differs");RegCloseKey(proof);}
  }
  if(uninstallApp)childProcessProbe(root,outside,restricted,sid,guid,registry,false,true,argc>=6?argv[5]:L"empty-fallback",uiMode?argv[6]:L"");
  require(RegDeleteTreeW(HKEY_CURRENT_USER,registry.c_str())==ERROR_SUCCESS,"Remove own probe registry keys");registry.clear();
  std::cout<<"{\"status\":\"passed_for_restricted_primary_child_private_desktop_and_impersonation\",\"inside_file_write\":true,\"outside_file_write_denied\":true,\"outside_file_delete_denied\":true,\"child_outside_medium_and_low_integrity_mutations_denied\":true,\"outside_dacl_owner_mutations_denied\":true,\"system_runtime_read\":true,\"inside_registry_write\":true,\"outside_registry_write_denied\":true,\"outside_markers_unchanged\":true,\"private_registry_keys_removed\":true,\"restricted_primary_child\":true,\"untrusted_integrity_rid\":0,\"child_private_hkcu_hklm\":true,\"child_original_hkcu_write_denied\":true,\"private_window_station\":false,\"noninteractive_station_acl_restored\":true,\"station_dacl_byte_exact_after_restore\":true,\"private_desktop\":true,\"child_job_limit_one\":true,\"uninstaller_executed\":"<<(uninstallApp?"true":"false")<<",\"guarded_entry_client_verified\":"<<(guardedClient?"true":"false")<<"}\n";
 }catch(const std::exception& e){if(impersonating)RevertToSelf();if(!registry.empty()&&registry.find(L"Software\\FarmarReconstruction\\UninstallProbe\\")==0)RegDeleteTreeW(HKEY_CURRENT_USER,registry.c_str());std::cerr<<e.what()<<'\n';if(restricted)CloseHandle(restricted);if(token)CloseHandle(token);if(sid)LocalFree(sid);return 1;}
 if(restricted)CloseHandle(restricted);if(token)CloseHandle(token);if(sid)LocalFree(sid);return 0;
}
