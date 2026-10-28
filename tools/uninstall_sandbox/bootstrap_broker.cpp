// Own process-tree observer. All descendants stay suspended until guarded.
#define WIN32_LEAN_AND_MEAN
#include "bootstrap_broker.h"
#include "entry_injection.h"
#include "guard_protocol.h"
#include "ui_driver.h"
#include <fstream>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <memory>
namespace fs=std::filesystem;
namespace {
void need(bool ok,const char* text){if(!ok)throw std::runtime_error(std::string(text)+" Win32="+std::to_string(GetLastError()));}
std::vector<char> bytes(const fs::path& path){std::ifstream f(path,std::ios::binary);need(bool(f),"Read own bootstrap image");return {std::istreambuf_iterator<char>(f),{}};}
fs::path wide(const char* value) {wchar_t output[32768]{};need(MultiByteToWideChar(CP_ACP,0,value,-1,output,32768)>0,"Bootstrap ANSI path");return output;}
void createByOwner(const PROCESS_INFORMATION& parent,HANDLE restricted,HANDLE job,HDESK desktop,const fs::path& root,const FarmarBootstrapLaunch& launch,PROCESS_INFORMATION& child) {
    const auto directory=wide(launch.directory).lexically_normal();
    const auto relative=directory.lexically_relative(root);need(!relative.empty()&&*relative.begin()!=L".."&&fs::is_directory(directory),"Broker must preserve only an owned working directory");
    std::vector<std::wstring> environment;auto* inherited=GetEnvironmentStringsW();need(inherited!=nullptr,"Broker environment");
    for(auto* item=inherited;*item;item+=wcslen(item)+1)if(_wcsnicmp(item,L"__COMPAT_LAYER=",15)&&_wcsnicmp(item,L"TEMP=",5)&&_wcsnicmp(item,L"TMP=",4))environment.emplace_back(item);FreeEnvironmentStringsW(inherited);
    environment.emplace_back(L"__COMPAT_LAYER=RunAsInvoker");environment.emplace_back(L"TEMP="+(root/L"temp").wstring());environment.emplace_back(L"TMP="+(root/L"temp").wstring());std::sort(environment.begin(),environment.end(),[](const auto& a,const auto& b){return _wcsicmp(a.c_str(),b.c_str())<0;});std::vector<wchar_t> block;for(const auto& value:environment){block.insert(block.end(),value.begin(),value.end());block.push_back(0);}block.push_back(0);
    wchar_t stationName[256]{},desktopName[256]{};DWORD length=0;need(GetUserObjectInformationW(GetProcessWindowStation(),UOI_NAME,stationName,sizeof(stationName),&length)&&GetUserObjectInformationW(desktop,UOI_NAME,desktopName,sizeof(desktopName),&length),"Broker private desktop identity");
    std::wstring selected=std::wstring(stationName)+L"\\"+desktopName;need(std::wstring(desktopName).find(L"Contained-")==0,"Broker never selects user desktop");
    SIZE_T size=0;InitializeProcThreadAttributeList(nullptr,1,0,&size);std::vector<BYTE> storage(size);auto* attributes=reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(storage.data());need(InitializeProcThreadAttributeList(attributes,1,0,&size)!=FALSE,"Broker parent attribute list");
    const auto destroy=[](LPPROC_THREAD_ATTRIBUTE_LIST value){DeleteProcThreadAttributeList(value);};std::unique_ptr<_PROC_THREAD_ATTRIBUTE_LIST,decltype(destroy)> cleanup(attributes,destroy);
    HANDLE parentHandle=parent.hProcess;need(UpdateProcThreadAttribute(attributes,0,PROC_THREAD_ATTRIBUTE_PARENT_PROCESS,&parentHandle,sizeof(parentHandle),nullptr,nullptr)!=FALSE,"Preserve original bootstrap parent process identity");
    STARTUPINFOEXW startup{};startup.StartupInfo.cb=sizeof(startup);startup.StartupInfo.lpDesktop=selected.data();startup.lpAttributeList=attributes;auto command=wide(launch.command).wstring();const auto executable=wide(launch.executable);
    need(CreateProcessAsUserW(restricted,executable.c_str(),command.data(),nullptr,nullptr,FALSE,CREATE_SUSPENDED|CREATE_UNICODE_ENVIRONMENT|EXTENDED_STARTUPINFO_PRESENT,block.data(),directory.c_str(),&startup.StartupInfo,&child)!=FALSE,"Broker genuine CreateProcessAsUser with same restricted token");
    BOOL member=FALSE;need(IsProcessInJob(child.hProcess,job,&member)!=FALSE,"Broker child Job query");if(!member)need(AssignProcessToJobObject(job,child.hProcess)!=FALSE,"Assign own suspended broker child to exact Job");
}
void replyToParent(const PROCESS_INFORMATION& parent,const PROCESS_INFORMATION& child,const fs::path& root) {
    HANDLE processValue=nullptr,threadValue=nullptr;need(DuplicateHandle(GetCurrentProcess(),child.hProcess,parent.hProcess,&processValue,0,FALSE,DUPLICATE_SAME_ACCESS)&&DuplicateHandle(GetCurrentProcess(),child.hThread,parent.hProcess,&threadValue,0,FALSE,DUPLICATE_SAME_ACCESS),"Return genuine process/thread handles to original parent");
    FarmarBootstrapReply reply{farmarBootstrapReplyMagic,parent.dwProcessId,child.dwProcessId,child.dwThreadId,DWORD(reinterpret_cast<uintptr_t>(processValue)),DWORD(reinterpret_cast<uintptr_t>(threadValue))};
    const auto partial=root/L"bootstrap-reply.partial",destination=root/L"bootstrap-reply.bin";std::ofstream file(partial,std::ios::binary);file.write(reinterpret_cast<const char*>(&reply),sizeof(reply));file.close();need(bool(file),"Write owned broker response");fs::rename(partial,destination);
}
}
void driveBootstrapTree(const PROCESS_INFORMATION& parent,HANDLE restricted,HANDLE job,HDESK desktop,const fs::path& root,const std::wstring& registry,const std::wstring& uiAction) {
    FarmarBootstrapLaunch launch{};const auto request=root/L"bootstrap-launch.bin";const auto deadline=GetTickCount64()+25000;
    while(!fs::exists(request)&&GetTickCount64()<deadline){if(WaitForSingleObject(parent.hProcess,0)==WAIT_OBJECT_0)break;Sleep(10);}
    need(fs::exists(request),"Owned bootstrap parent produced no launch request");need(fs::file_size(request)==sizeof(launch),"Owned bootstrap launch size");
    std::ifstream input(request,std::ios::binary);input.read(reinterpret_cast<char*>(&launch),sizeof(launch));need(bool(input),"Read complete owned bootstrap launch");
    need(launch.magic==farmarBootstrapMagic&&launch.parentPid==parent.dwProcessId&&launch.originalFlags==0&&launch.creationBrokered<=1&&
        (launch.creationBrokered?(!launch.childPid&&!launch.childThread):(launch.childPid&&launch.childThread)),"Owned bootstrap launch identity");
    need(std::memchr(launch.executable,0,sizeof(launch.executable))&&std::memchr(launch.command,0,sizeof(launch.command))&&std::memchr(launch.directory,0,sizeof(launch.directory)),"Bounded bootstrap launch strings");
    const auto executable=wide(launch.executable).lexically_normal();const auto name=executable.filename().wstring();
    need(executable.parent_path()==root/L"temp"/L"~nsu.tmp"&&name.size()==7&&name[0]>=L'A'&&name[0]<=L'Z'&&name.substr(1)==L"u_.exe","Bootstrap executable must be owned temp self-copy");
    need(bytes(executable)==bytes(root/L"runner"/L"Uninstall.exe"),"Bootstrap copy differs from own input image");
    PROCESS_INFORMATION child{};HANDLE cu=nullptr,lm=nullptr;bool attached=false;
    try {
        if(launch.creationBrokered)createByOwner(parent,restricted,job,desktop,root,launch,child);
        else {child.dwProcessId=launch.childPid;child.dwThreadId=launch.childThread;child.hProcess=OpenProcess(PROCESS_ALL_ACCESS,FALSE,child.dwProcessId);need(child.hProcess!=nullptr,"Open owned suspended bootstrap child");child.hThread=OpenThread(THREAD_ALL_ACCESS,FALSE,child.dwThreadId);}
        need(child.hThread!=nullptr&&GetProcessIdOfThread(child.hThread)==child.dwProcessId,"Owned bootstrap primary thread identity");
        BOOL inJob=FALSE;need(IsProcessInJob(child.hProcess,job,&inJob)&&inJob,"Bootstrap child must belong to this exact Job");
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};need(QueryInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits),nullptr)&&
            (limits.BasicLimitInformation.LimitFlags&JOB_OBJECT_LIMIT_ACTIVE_PROCESS)&&limits.BasicLimitInformation.ActiveProcessLimit==2,"Bootstrap exact Job limit must be two");
        wchar_t image[32768]{};DWORD length=32768;need(QueryFullProcessImageNameW(child.hProcess,0,image,&length)&&fs::path(image)==executable,"Actual owned bootstrap process image");
        using Query=LONG(NTAPI*)(HANDLE,ULONG,PVOID,ULONG,PULONG);auto query=reinterpret_cast<Query>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"),"NtQueryInformationProcess"));DWORD info[6]{};
        need(query&&query(child.hProcess,0,info,sizeof(info),nullptr)>=0&&info[5]==parent.dwProcessId,"Bootstrap actual parent process identity");
        HANDLE token=nullptr;need(OpenProcessToken(child.hProcess,TOKEN_QUERY,&token)&&IsTokenRestricted(token),"Bootstrap inherited restricted primary token");
        BYTE labelData[256]{};DWORD labelSize=sizeof(labelData);const bool got=GetTokenInformation(token,TokenIntegrityLevel,labelData,sizeof(labelData),&labelSize)!=FALSE;CloseHandle(token);need(got,"Bootstrap child token integrity");
        auto* label=reinterpret_cast<TOKEN_MANDATORY_LABEL*>(labelData);need(*GetSidSubAuthority(label->Label.Sid,*GetSidSubAuthorityCount(label->Label.Sid)-1)==0,"Bootstrap child must remain Untrusted");
        if(launch.creationBrokered)replyToParent(parent,child,root);
        // RegOverridePredefKey is process-local. Give the real new process its
        // own exact handles, then map them in our setup before its PE entry.
        HKEY localCu=nullptr,localLm=nullptr;need(RegOpenKeyExW(HKEY_CURRENT_USER,(registry+L"\\inside\\HKCU").c_str(),0,KEY_ALL_ACCESS,&localCu)==ERROR_SUCCESS,"Open own bootstrap HKCU");
        auto status=RegOpenKeyExW(HKEY_CURRENT_USER,(registry+L"\\inside\\HKLM").c_str(),0,KEY_ALL_ACCESS,&localLm);if(status!=ERROR_SUCCESS){RegCloseKey(localCu);need(false,"Open own bootstrap HKLM");}
        const bool duplicatedCu=DuplicateHandle(GetCurrentProcess(),localCu,child.hProcess,&cu,0,FALSE,DUPLICATE_SAME_ACCESS)!=FALSE;
        const bool duplicatedLm=DuplicateHandle(GetCurrentProcess(),localLm,child.hProcess,&lm,0,FALSE,DUPLICATE_SAME_ACCESS)!=FALSE;RegCloseKey(localCu);RegCloseKey(localLm);need(duplicatedCu&&duplicatedLm,"Duplicate only private hive handles to own bootstrap child");
        need(DebugActiveProcess(child.dwProcessId)!=FALSE,"Attach own suspended bootstrap child before loader/entry");attached=true;
        std::function<void()> driver;if(!uiAction.empty())driver=[&]{driveUninstallUi(child,desktop,root,uiAction);};
        const auto code=runGuardedEntry(child,root,cu,lm,driver,GuardedEntryOptions{false,1});attached=false;
        need(code==(uiAction==L"language-cancel"?2U:uiAction==L"cancel"?1U:0U),"Owned bootstrap descendant exit differs");
        std::ofstream report(root/L"bootstrap-tree-result.json");report<<"{\"parent_pid\":"<<parent.dwProcessId<<",\"child_pid\":"<<child.dwProcessId<<",\"child_exit_code\":"<<code<<",\"copy_letter\":\""<<char(name[0])<<"\",\"same_input_image\":true,\"same_owned_job\":true,\"job_active_process_limit\":2,\"creation_brokered_for_error740\":"<<(launch.creationBrokered?"true":"false")<<",\"same_restricted_primary_preserved\":true,\"integrity_rid\":0,\"child_hives_mapped_before_entry\":true,\"child_application_entry_guarded\":true}\n";
        CloseHandle(child.hThread);CloseHandle(child.hProcess);
    }catch(...){if(child.hProcess)TerminateProcess(child.hProcess,0xe5);if(attached)DebugActiveProcessStop(child.dwProcessId);if(child.hProcess)WaitForSingleObject(child.hProcess,5000);if(child.hThread)CloseHandle(child.hThread);if(child.hProcess)CloseHandle(child.hProcess);throw;}
}
