// Diagnostic only: our setup DLL runs while the application's entry is suspended.
#define WIN32_LEAN_AND_MEAN
#include "entry_injection.h"
#include "guard_protocol.h"
#include <tlhelp32.h>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <string>
#include <cstring>
namespace fs=std::filesystem;
namespace {
void need(bool ok,const char* message) {if(!ok)throw std::runtime_error(std::string(message)+" Win32="+std::to_string(GetLastError()));}
void read(HANDLE process,uintptr_t address,void* out,size_t bytes) {SIZE_T done=0;need(ReadProcessMemory(process,reinterpret_cast<void*>(address),out,bytes,&done)&&done==bytes,"Read own child");}
void write(HANDLE process,uintptr_t address,const void* bytes,size_t count) {
    DWORD old=0;need(VirtualProtectEx(process,reinterpret_cast<void*>(address),count,PAGE_EXECUTE_READWRITE,&old),"Child patch protection");SIZE_T done=0;
    need(WriteProcessMemory(process,reinterpret_cast<void*>(address),bytes,count,&done)&&done==count,"Patch own child");DWORD ignored=0;
    need(VirtualProtectEx(process,reinterpret_cast<void*>(address),count,old,&ignored),"Restore child protection");need(FlushInstructionCache(process,reinterpret_cast<void*>(address),count),"Flush own child patch");
}
uintptr_t remoteOsFunction(DWORD pid,const char* name) {
    const auto function=GetProcAddress(GetModuleHandleW(L"kernel32.dll"),name);need(function!=nullptr,"Local loader export");HMODULE local=nullptr;
    need(GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<wchar_t*>(function),&local),"Actual forwarded OS module");
    wchar_t filename[MAX_PATH]{};need(GetModuleFileNameW(local,filename,MAX_PATH)!=0,"OS module filename");const auto basename=fs::path(filename).filename().wstring();
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,pid);need(snapshot!=INVALID_HANDLE_VALUE,"Child module snapshot");MODULEENTRY32W entry{sizeof(entry)};uintptr_t base=0;
    for(BOOL more=Module32FirstW(snapshot,&entry);more;more=Module32NextW(snapshot,&entry))if(!_wcsicmp(entry.szModule,basename.c_str())){base=reinterpret_cast<uintptr_t>(entry.modBaseAddr);break;}
    CloseHandle(snapshot);need(base!=0,"Resolved OS module in own child");return base+(reinterpret_cast<uintptr_t>(function)-reinterpret_cast<uintptr_t>(local));
}
DWORD runThread(const PROCESS_INFORMATION& process,uintptr_t function,void* argument) {
    HANDLE thread=CreateRemoteThread(process.hProcess,nullptr,0,reinterpret_cast<LPTHREAD_START_ROUTINE>(function),argument,0,nullptr);need(thread!=nullptr,"Create own setup thread");
    const auto waited=WaitForSingleObject(thread,10000);DWORD result=0;const bool readCode=GetExitCodeThread(thread,&result)!=FALSE;CloseHandle(thread);
    need(waited==WAIT_OBJECT_0&&readCode,"Own setup thread timeout/exit");return result;
}
DWORD setupRva(const fs::path& path) {
    std::ifstream file(path,std::ios::binary);std::vector<BYTE> bytes{std::istreambuf_iterator<char>(file),{}};need(bytes.size()>512,"Own setup DLL file");
    auto* dos=reinterpret_cast<IMAGE_DOS_HEADER*>(bytes.data());need(dos->e_magic==IMAGE_DOS_SIGNATURE&&dos->e_lfanew>0&&size_t(dos->e_lfanew)+sizeof(IMAGE_NT_HEADERS32)<bytes.size(),"Own setup DLL DOS header");
    auto* nt=reinterpret_cast<IMAGE_NT_HEADERS32*>(bytes.data()+dos->e_lfanew);need(nt->Signature==IMAGE_NT_SIGNATURE&&nt->OptionalHeader.Magic==IMAGE_NT_OPTIONAL_HDR32_MAGIC,"Own setup DLL PE32");
    auto at=[&](DWORD rva,size_t length)->const BYTE* {for(unsigned i=0;i<nt->FileHeader.NumberOfSections;++i){const auto& section=IMAGE_FIRST_SECTION(nt)[i];if(rva>=section.VirtualAddress&&uint64_t(rva-section.VirtualAddress)+length<=section.SizeOfRawData){const auto off=uint64_t(section.PointerToRawData)+rva-section.VirtualAddress;need(off+length<=bytes.size(),"Own DLL RVA bounds");return bytes.data()+off;}}throw std::runtime_error("Own DLL unmapped RVA");};
    auto* exports=reinterpret_cast<const IMAGE_EXPORT_DIRECTORY*>(at(nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress,sizeof(IMAGE_EXPORT_DIRECTORY)));
    auto* names=reinterpret_cast<const DWORD*>(at(exports->AddressOfNames,exports->NumberOfNames*4));auto* ordinals=reinterpret_cast<const WORD*>(at(exports->AddressOfNameOrdinals,exports->NumberOfNames*2));auto* functions=reinterpret_cast<const DWORD*>(at(exports->AddressOfFunctions,exports->NumberOfFunctions*4));
    for(DWORD i=0;i<exports->NumberOfNames;++i)if(!std::strcmp(reinterpret_cast<const char*>(at(names[i],32)),"FarmarSandboxSetup")){need(ordinals[i]<exports->NumberOfFunctions,"Own setup ordinal");return functions[ordinals[i]];}
    throw std::runtime_error("Missing own FarmarSandboxSetup export");
}
}
DWORD runGuardedEntry(const PROCESS_INFORMATION& process,const fs::path& root,HANDLE privateCu,HANDLE privateLm,const std::function<void()>& onStarted) {
    bool debug=true;uintptr_t entry=0;BYTE original=0;bool stopped=false;
    try {
        need(ResumeThread(process.hThread)!=DWORD(-1),"Resume own child loader");const auto deadline=GetTickCount64()+20000;
        while(!stopped&&GetTickCount64()<deadline) {
            DEBUG_EVENT e{};if(!WaitForDebugEvent(&e,1000)){need(GetLastError()==ERROR_SEM_TIMEOUT,"Own child loader event");continue;}DWORD action=DBG_CONTINUE;
            if(e.dwDebugEventCode==CREATE_PROCESS_DEBUG_EVENT) {
                const auto base=reinterpret_cast<uintptr_t>(e.u.CreateProcessInfo.lpBaseOfImage);IMAGE_DOS_HEADER dos{};read(process.hProcess,base,&dos,sizeof(dos));need(dos.e_magic==IMAGE_DOS_SIGNATURE,"Own child DOS header");IMAGE_NT_HEADERS32 nt{};read(process.hProcess,base+dos.e_lfanew,&nt,sizeof(nt));need(nt.Signature==IMAGE_NT_SIGNATURE&&nt.OptionalHeader.Magic==IMAGE_NT_OPTIONAL_HDR32_MAGIC,"Own child PE32 header");
                entry=base+nt.OptionalHeader.AddressOfEntryPoint;read(process.hProcess,entry,&original,1);const BYTE trap=0xcc;write(process.hProcess,entry,&trap,1);if(e.u.CreateProcessInfo.hFile)CloseHandle(e.u.CreateProcessInfo.hFile);
            } else if(e.dwDebugEventCode==LOAD_DLL_DEBUG_EVENT) {if(e.u.LoadDll.hFile)CloseHandle(e.u.LoadDll.hFile);}
            else if(e.dwDebugEventCode==EXIT_PROCESS_DEBUG_EVENT)throw std::runtime_error("Owned child exited before entry setup: "+std::to_string(e.u.ExitProcess.dwExitCode));
            else if(e.dwDebugEventCode==EXCEPTION_DEBUG_EVENT) {
                const auto& exception=e.u.Exception.ExceptionRecord;
                if(exception.ExceptionCode==EXCEPTION_BREAKPOINT&&reinterpret_cast<uintptr_t>(exception.ExceptionAddress)==entry) {
                    need(e.dwThreadId==process.dwThreadId,"Application entry must be on primary thread");write(process.hProcess,entry,&original,1);CONTEXT context{};context.ContextFlags=CONTEXT_CONTROL;need(GetThreadContext(process.hThread,&context),"Read own entry context");need(context.Eip==entry+1,"Own entry breakpoint EIP");context.Eip=DWORD(entry);need(SetThreadContext(process.hThread,&context),"Restore own entry EIP");need(SuspendThread(process.hThread)!=DWORD(-1),"Hold primary before application entry");stopped=true;
                } else if(exception.ExceptionCode!=EXCEPTION_BREAKPOINT)action=DBG_EXCEPTION_NOT_HANDLED;
            }
            need(ContinueDebugEvent(e.dwProcessId,e.dwThreadId,action),"Continue own child loader");
        }
        need(stopped,"Own application entry timeout");need(DebugActiveProcessStop(process.dwProcessId),"Detach while primary stays suspended");debug=false;
        const auto dll=root/L"uninstall-application-guard.dll";const auto wide=dll.wstring();
        auto* remote=VirtualAllocEx(process.hProcess,nullptr,8192,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);need(remote!=nullptr,"Allocate own setup data");SIZE_T written=0;
        need(wide.size()*2+2<4096&&WriteProcessMemory(process.hProcess,remote,wide.c_str(),wide.size()*2+2,&written)&&written==wide.size()*2+2,"Write own setup DLL path");
        const auto module=runThread(process,remoteOsFunction(process.dwProcessId,"LoadLibraryW"),remote);need(module!=0,"Load own setup DLL in child");
        FarmarGuardConfig config{};config.version=1;config.privateCu=DWORD(reinterpret_cast<uintptr_t>(privateCu));config.privateLm=DWORD(reinterpret_cast<uintptr_t>(privateLm));BOOL replaced=FALSE;
        const bool utf8=GetACP()==CP_UTF8;const auto count=WideCharToMultiByte(CP_ACP,utf8?WC_ERR_INVALID_CHARS:WC_NO_BEST_FIT_CHARS,root.c_str(),-1,config.root,MAX_PATH,nullptr,utf8?nullptr:&replaced);need(count>0&&count<MAX_PATH&&!replaced,"Exact ANSI sandbox path");
        auto* remoteConfig=static_cast<BYTE*>(remote)+4096;need(WriteProcessMemory(process.hProcess,remoteConfig,&config,sizeof(config),&written)&&written==sizeof(config),"Write private setup config");
        const auto result=runThread(process,uintptr_t(module)+setupRva(dll),remoteConfig);need(result==farmarGuardReady,(std::string("Own setup failed: ")+std::to_string(result)).c_str());
        need(VirtualFreeEx(process.hProcess,remote,0,MEM_RELEASE),"Free own setup data");
        std::ofstream(root/L"entry-boundary.json")<<"{\"entry_held_until_guard_setup\":true,\"primary_token_unchanged\":true,\"guard_ready\":"<<result<<",\"entry\":"<<entry<<"}\n";
        need(ResumeThread(process.hThread)==1,"Release exactly one primary suspend after setup");
        if(onStarted)onStarted();
        need(WaitForSingleObject(process.hProcess,20000)==WAIT_OBJECT_0,"Own guarded application timeout");DWORD code=0;need(GetExitCodeProcess(process.hProcess,&code),"Own guarded application exit");return code;
    } catch(...) {TerminateProcess(process.hProcess,0xe3);if(debug)DebugActiveProcessStop(process.dwProcessId);WaitForSingleObject(process.hProcess,5000);throw;}
}
