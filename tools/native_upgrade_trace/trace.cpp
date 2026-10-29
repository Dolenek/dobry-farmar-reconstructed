// Diagnostic only: hardware execution breakpoint, no game memory/code writes.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <filesystem>
#include <array>
#include <iostream>
#include <string>
#include <stdexcept>
namespace {
constexpr DWORD target=0x00417050;
void need(bool good,const char* text){if(!good)throw std::runtime_error(text);}
bool read(HANDLE process,DWORD address,void* out,SIZE_T size){SIZE_T done{};return ReadProcessMemory(process,reinterpret_cast<void*>(address),out,size,&done)&&done==size;}
void install(DWORD tid){
    HANDLE thread=OpenThread(THREAD_GET_CONTEXT|THREAD_SET_CONTEXT,FALSE,tid);need(thread!=nullptr,"Open debug event thread");
    CONTEXT c{};c.ContextFlags=CONTEXT_DEBUG_REGISTERS;
    bool good=GetThreadContext(thread,&c)!=FALSE;
    if(good){c.Dr0=target;c.Dr6=0;c.Dr7=(c.Dr7&~0x000f0003U)|1U;good=SetThreadContext(thread,&c)!=FALSE;}
    CloseHandle(thread);need(good,"Install hardware execution breakpoint");
}
void clearAll(DWORD pid){
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);if(snapshot==INVALID_HANDLE_VALUE)return;
    THREADENTRY32 entry{};entry.dwSize=sizeof(entry);
    for(BOOL found=Thread32First(snapshot,&entry);found;found=Thread32Next(snapshot,&entry))if(entry.th32OwnerProcessID==pid){
        HANDLE thread=OpenThread(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT|THREAD_SET_CONTEXT,FALSE,entry.th32ThreadID);if(!thread)continue;
        if(SuspendThread(thread)!=DWORD(-1)){
            CONTEXT c{};c.ContextFlags=CONTEXT_DEBUG_REGISTERS;
            if(GetThreadContext(thread,&c)){c.Dr0=0;c.Dr6=0;c.Dr7&=~3U;SetThreadContext(thread,&c);}
            ResumeThread(thread);
        }
        CloseHandle(thread);
    }
    CloseHandle(snapshot);
}
}
int wmain(int argc,wchar_t** argv){
    DWORD pid{};HANDLE process{};bool attached=false,pending=false;DWORD pendingTid{};unsigned calls=0,abnormal=0;
    try{
        need(argc==3||argc==4,"Expected owned-native-pid, exact reference executable, optional stop-after-abnormal");pid=std::stoul(argv[1]);
        process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|PROCESS_VM_READ|SYNCHRONIZE,FALSE,pid);need(process!=nullptr,"Open owned diagnostic process");
        wchar_t path[32768]{};DWORD length=32768;need(QueryFullProcessImageNameW(process,0,path,&length)!=FALSE,"Read reference image path");
        need(std::filesystem::equivalent(path,std::filesystem::absolute(argv[2]))&&std::filesystem::path(path).filename()==L"Dobrý farmář.exe","Refuse unrelated process");
        std::array<unsigned char,3> bytes{};need(read(process,target,bytes.data(),bytes.size())&&bytes==std::array<unsigned char,3>{0x55,0x8b,0xec},"Original level entry bytes differ");
        need(DebugActiveProcess(pid)!=FALSE,"Attach owned native diagnostic");attached=true;need(DebugSetProcessKillOnExit(FALSE)!=FALSE,"Keep owned game alive on debugger exit");
        bool ready=false,done=false;const auto started=GetTickCount64();
        while(!done){
            DEBUG_EVENT e{};
            if(!WaitForDebugEvent(&e,1000)){need(GetLastError()==ERROR_SEM_TIMEOUT,"Wait debug event failed");need(GetTickCount64()-started<200000,"Hardware diagnostic deadline");continue;}
            pending=true;pendingTid=e.dwThreadId;DWORD action=DBG_CONTINUE;
            if(e.dwDebugEventCode==CREATE_PROCESS_DEBUG_EVENT){install(e.dwThreadId);if(e.u.CreateProcessInfo.hFile)CloseHandle(e.u.CreateProcessInfo.hFile);CloseHandle(e.u.CreateProcessInfo.hThread);CloseHandle(e.u.CreateProcessInfo.hProcess);}
            else if(e.dwDebugEventCode==CREATE_THREAD_DEBUG_EVENT){install(e.dwThreadId);CloseHandle(e.u.CreateThread.hThread);}
            else if(e.dwDebugEventCode==LOAD_DLL_DEBUG_EVENT){if(e.u.LoadDll.hFile)CloseHandle(e.u.LoadDll.hFile);}
            else if(e.dwDebugEventCode==EXIT_PROCESS_DEBUG_EVENT){done=true;std::cout<<"{\"event\":\"exit\",\"exit_code\":"<<e.u.ExitProcess.dwExitCode<<",\"upgrade_level_calls\":"<<calls<<",\"abnormal_ids\":"<<abnormal<<"}"<<std::endl;}
            else if(e.dwDebugEventCode==EXCEPTION_DEBUG_EVENT){
                const auto& x=e.u.Exception.ExceptionRecord;
                if(x.ExceptionCode==EXCEPTION_SINGLE_STEP&&(DWORD)x.ExceptionAddress==target){
                    HANDLE thread=OpenThread(THREAD_GET_CONTEXT|THREAD_SET_CONTEXT,FALSE,e.dwThreadId);need(thread!=nullptr,"Open breakpoint thread");
                    CONTEXT c{};c.ContextFlags=CONTEXT_FULL|CONTEXT_DEBUG_REGISTERS;need(GetThreadContext(thread,&c)!=FALSE,"Read breakpoint context");
                    DWORD id{};need(read(process,c.Ecx+0x24,&id,4),"Read Upgrade property id");++calls;
                    if(id>34&&abnormal++<32){
                        std::array<DWORD,24> stack{};std::array<DWORD,16> object{};need(read(process,c.Esp,stack.data(),sizeof(stack)),"Read diagnostic stack");need(read(process,c.Ecx,object.data(),sizeof(object)),"Read diagnostic object");
                        std::cout<<"{\"event\":\"abnormal_upgrade_id\",\"thread\":"<<e.dwThreadId<<",\"property_id\":"<<id<<",\"ecx\":"<<c.Ecx<<",\"esp\":"<<c.Esp<<",\"ebp\":"<<c.Ebp<<",\"stack\":[";
                        for(unsigned i=0;i<stack.size();++i)std::cout<<(i?",":"")<<stack[i];std::cout<<"],\"object_words\":[";
                        for(unsigned i=0;i<object.size();++i)std::cout<<(i?",":"")<<object[i];std::cout<<"],\"frames\":[";
                        DWORD frame=c.Ebp;bool comma=false;for(unsigned i=0;i<12&&frame;++i){DWORD next[2]{};if(!read(process,frame,next,8))break;std::cout<<(comma?",":"")<<next[1];comma=true;if(next[0]<=frame||next[0]-frame>1048576)break;frame=next[0];}
                        std::cout<<"]}"<<std::endl;
                    }
                    c.EFlags|=0x10000;c.Dr6=0;if(argc==4&&abnormal){c.Dr0=0;c.Dr7&=~3U;}need(SetThreadContext(thread,&c)!=FALSE,"Resume unchanged instruction with RF");CloseHandle(thread);
                }else if(x.ExceptionCode==EXCEPTION_BREAKPOINT){if(!ready){ready=true;std::cout<<"{\"event\":\"ready\",\"pid\":"<<pid<<",\"hardware_execution_address\":"<<target<<",\"game_memory_written\":false}"<<std::endl;}}
                else action=DBG_EXCEPTION_NOT_HANDLED;
            }
            need(ContinueDebugEvent(e.dwProcessId,e.dwThreadId,action)!=FALSE,"Continue debug event");
            pending=false;
            if(argc==4&&abnormal){clearAll(pid);need(DebugActiveProcessStop(pid)!=FALSE,"Detach after diagnostic hit");attached=false;std::cout<<"{\"event\":\"detached_after_abnormal\",\"upgrade_level_calls\":"<<calls<<",\"abnormal_ids\":"<<abnormal<<"}"<<std::endl;CloseHandle(process);return 0;}
        }
        attached=false;CloseHandle(process);return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';if(attached){clearAll(pid);if(pending)ContinueDebugEvent(pid,pendingTid,DBG_CONTINUE);DebugActiveProcessStop(pid);}if(process)CloseHandle(process);return 1;}
}
