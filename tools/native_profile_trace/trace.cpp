// Diagnostic hardware entry breakpoints; no game instruction or data writes.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <filesystem>
#include <array>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <string>
#include <stdexcept>
#include <map>
namespace {
constexpr DWORD single=0x004d95c0,twice=0x004d9a90;
constexpr DWORD singleAfter=0x004d9611,twiceAfter=0x004d9ae1;
struct Pending {std::string key;DWORD output;bool one;};
std::map<DWORD,Pending> pendingParses;
void need(bool good,const char* text){if(!good)throw std::runtime_error(text);}
bool read(HANDLE p,DWORD at,void* out,SIZE_T size){SIZE_T done{};return ReadProcessMemory(p,reinterpret_cast<void*>(at),out,size,&done)&&done==size;}
DWORD word(HANDLE p,DWORD at){DWORD value{};need(read(p,at,&value,4),"Read diagnostic word");return value;}
std::string text(HANDLE p,DWORD object,bool wide){
    DWORD length=word(p,object+20),capacity=word(p,object+24);need(length<=4096&&length<=capacity,"Diagnostic string bounds");
    DWORD data=capacity>=(wide ? 8U:16U) ? word(p,object+4):object+4;
    if(!wide){std::string s(length,'\0');need(read(p,data,s.data(),length),"Read key bytes");return s;}
    std::wstring value(length,L'\0');need(read(p,data,value.data(),length*2),"Read value units");std::string result;
    for(wchar_t c:value){need(c<128,"Numeric fixture text must be ASCII");result.push_back(char(c));}return result;
}
void install(DWORD tid,bool clear=false){
    HANDLE thread=OpenThread(THREAD_GET_CONTEXT|THREAD_SET_CONTEXT,FALSE,tid);need(thread!=nullptr,"Open debug thread");
    CONTEXT c{};c.ContextFlags=CONTEXT_DEBUG_REGISTERS;need(GetThreadContext(thread,&c)!=FALSE,"Get hardware context");
    c.Dr0=clear ? 0:single;c.Dr1=clear ? 0:twice;c.Dr2=clear ? 0:singleAfter;c.Dr3=clear ? 0:twiceAfter;c.Dr6=0;c.Dr7=(c.Dr7&~0xffff00ffU)|(clear ? 0:0x55);
    need(SetThreadContext(thread,&c)!=FALSE,"Set hardware context");CloseHandle(thread);
}
void clearAll(DWORD pid){
    HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);if(snap==INVALID_HANDLE_VALUE)return;
    THREADENTRY32 e{};e.dwSize=sizeof(e);
    for(BOOL found=Thread32First(snap,&e);found;found=Thread32Next(snap,&e))if(e.th32OwnerProcessID==pid){
        HANDLE thread=OpenThread(THREAD_SUSPEND_RESUME,FALSE,e.th32ThreadID);if(!thread)continue;
        if(SuspendThread(thread)!=DWORD(-1)){try{install(e.th32ThreadID,true);}catch(...){}ResumeThread(thread);}CloseHandle(thread);
    }
    CloseHandle(snap);
}
}
int wmain(int argc,wchar_t** argv){
    DWORD pid{},tid{};HANDLE process{};bool attached=false,pending=false;unsigned entries{};
    try{
        need(argc==3||(argc==4&&std::wstring(argv[3])==L"pc53"),"Expected owned reference pid, exact image path and optional pc53");pid=std::stoul(argv[1]);
        process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|PROCESS_VM_READ|SYNCHRONIZE,FALSE,pid);need(process!=nullptr,"Open owned reference");
        wchar_t path[32768]{};DWORD length=32768;need(QueryFullProcessImageNameW(process,0,path,&length)!=FALSE,"Query reference path");
        need(std::filesystem::equivalent(path,std::filesystem::absolute(argv[2]))&&std::filesystem::path(path).filename()==L"Dobrý farmář.exe","Refuse unrelated process");
        for(DWORD address:{single,twice}){std::array<unsigned char,3> bytes{};need(read(process,address,bytes.data(),3)&&bytes==std::array<unsigned char,3>{0x64,0xa1,0},"Original parser entry bytes differ");}
        need(DebugActiveProcess(pid)!=FALSE,"Attach reference diagnostic");attached=true;need(DebugSetProcessKillOnExit(FALSE)!=FALSE,"Keep reference alive");bool done=false,ready=false;auto start=GetTickCount64();
        while(!done){
            DEBUG_EVENT e{};
            if(!WaitForDebugEvent(&e,1000)){need(GetLastError()==ERROR_SEM_TIMEOUT,"Wait parser event");need(GetTickCount64()-start<180000,"Parser diagnostic deadline");continue;}
            pending=true;tid=e.dwThreadId;DWORD action=DBG_CONTINUE;
            if(e.dwDebugEventCode==CREATE_PROCESS_DEBUG_EVENT){install(tid);if(e.u.CreateProcessInfo.hFile)CloseHandle(e.u.CreateProcessInfo.hFile);CloseHandle(e.u.CreateProcessInfo.hThread);CloseHandle(e.u.CreateProcessInfo.hProcess);}
            else if(e.dwDebugEventCode==CREATE_THREAD_DEBUG_EVENT){install(tid);CloseHandle(e.u.CreateThread.hThread);}
            else if(e.dwDebugEventCode==LOAD_DLL_DEBUG_EVENT){if(e.u.LoadDll.hFile)CloseHandle(e.u.LoadDll.hFile);}
            else if(e.dwDebugEventCode==EXIT_PROCESS_DEBUG_EVENT){done=true;std::cout<<"{\"event\":\"exit\",\"exit_code\":"<<e.u.ExitProcess.dwExitCode<<",\"entries\":"<<entries<<"}"<<std::endl;}
            else if(e.dwDebugEventCode==EXCEPTION_DEBUG_EVENT){const auto& x=e.u.Exception.ExceptionRecord;
                if(x.ExceptionCode==EXCEPTION_SINGLE_STEP&&((DWORD)x.ExceptionAddress==single||(DWORD)x.ExceptionAddress==twice||(DWORD)x.ExceptionAddress==singleAfter||(DWORD)x.ExceptionAddress==twiceAfter)){
                    HANDLE thread=OpenThread(THREAD_GET_CONTEXT|THREAD_SET_CONTEXT,FALSE,tid);need(thread!=nullptr,"Open parser context");CONTEXT c{};c.ContextFlags=CONTEXT_FULL|CONTEXT_DEBUG_REGISTERS|CONTEXT_FLOATING_POINT;need(GetThreadContext(thread,&c)!=FALSE,"Get parser context");
                    bool one=(DWORD)x.ExceptionAddress==single;DWORD args[3]{};need(read(process,c.Esp,args,sizeof(args)),"Read parser arguments");
                    if((DWORD)x.ExceptionAddress==singleAfter||(DWORD)x.ExceptionAddress==twiceAfter){
                        const auto found=pendingParses.find(tid);
                        if(found!=pendingParses.end()){
                            const auto p=found->second;DWORD stream=c.Esp+4,offset=word(process,word(process,stream)+4);need(offset<1024,"Original stream virtual base offset");DWORD state=word(process,stream+offset+8);std::uint64_t value{};need(read(process,p.output,&value,p.one ? 4:8),"Read extracted output");
                            std::cout<<"{\"event\":\"result\",\"key\":"<<std::quoted(p.key)<<",\"state\":"<<state<<",\"value\":\""<<std::hex<<std::setfill('0')<<std::setw(p.one ? 8:16)<<value<<std::dec<<"\"}"<<std::endl;pendingParses.erase(found);
                        }
                    }else if(args[0]==(one ? 0x004eb340U:0x004eb3f1U)){
                        auto key=text(process,word(process,args[1]+(one ? -4:56)),false);
                        if(key.rfind("FormatProbe_",0)==0){
                            // Optional diagnostic FP fixture only. The default
                            // leaves the actual game's PC24 control unchanged.
                            if(argc==4) c.FloatSave.ControlWord=(c.FloatSave.ControlWord&~0x300U)|0x200U;
                            std::uint64_t seed{};need(read(process,args[1],&seed,one ? 4:8),"Read untouched output seed");auto value=text(process,args[2],true);++entries;
                            std::cout<<"{\"event\":\"parse\",\"key\":"<<std::quoted(key)<<",\"text\":"<<std::quoted(value)<<",\"single\":"<<(one ? "true":"false")<<",\"output_address\":"<<args[1]<<",\"seed\":\""<<std::hex<<std::setfill('0')<<std::setw(one ? 8:16)<<seed<<std::dec<<"\",\"fp_control\":"<<c.FloatSave.ControlWord<<"}"<<std::endl;
                            if(value.empty()) std::cout<<"{\"event\":\"result\",\"key\":"<<std::quoted(key)<<",\"state\":null,\"empty_skipped\":true}"<<std::endl;
                            else pendingParses[tid]=Pending{key,args[1],one};
                        }
                    }
                    c.EFlags|=0x10000;c.Dr6=0;need(SetThreadContext(thread,&c)!=FALSE,"Resume unchanged parser entry");CloseHandle(thread);
                }else if(x.ExceptionCode==EXCEPTION_BREAKPOINT&&!ready){ready=true;std::cout<<"{\"event\":\"ready\",\"pid\":"<<pid<<",\"game_memory_written\":false}"<<std::endl;}
                else action=DBG_EXCEPTION_NOT_HANDLED;
            }
            need(ContinueDebugEvent(e.dwProcessId,e.dwThreadId,action)!=FALSE,"Continue parser debug event");pending=false;
        }
        attached=false;CloseHandle(process);return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';if(attached){clearAll(pid);if(pending)ContinueDebugEvent(pid,tid,DBG_CONTINUE);DebugActiveProcessStop(pid);}if(process)CloseHandle(process);return 1;}
}
