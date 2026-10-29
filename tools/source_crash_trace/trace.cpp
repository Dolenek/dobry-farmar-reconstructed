// Diagnostic debugger for the exact owned reconstructed executable only.
// No game memory writes, code breakpoints or exception suppression.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>
#include <filesystem>
#include <iostream>
#include <array>
#include <stdexcept>
#include <string>
void need(bool good,const char* what){if(!good)throw std::runtime_error(what);}
int wmain(int argc,wchar_t** argv){
    DWORD pid{},tid{};HANDLE process{};bool attached=false,pending=false,symbols=false;
    try{
        need(argc==3,"Expected owned source pid and exact executable path");pid=std::stoul(argv[1]);
        process=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|SYNCHRONIZE,FALSE,pid);
        need(process!=nullptr,"Open owned source process");
        wchar_t name[32768]{};DWORD length=32768;need(QueryFullProcessImageNameW(process,0,name,&length)!=FALSE,"Query source image");
        need(std::filesystem::equivalent(name,std::filesystem::absolute(argv[2]))&&std::filesystem::path(name).filename()==L"FarmarSourceInterop.exe","Refuse unrelated process");
        need(DebugActiveProcess(pid)!=FALSE,"Attach source debugger");attached=true;
        need(DebugSetProcessKillOnExit(FALSE)!=FALSE,"Keep owned process on debugger exit");
        SymSetOptions(SYMOPT_LOAD_LINES|SYMOPT_UNDNAME|SYMOPT_DEFERRED_LOADS|SYMOPT_FAIL_CRITICAL_ERRORS);
        symbols=SymInitialize(process,nullptr,TRUE)!=FALSE;
        bool ready=false,done=false;auto started=GetTickCount64();
        while(!done){
            DEBUG_EVENT e{};
            if(!WaitForDebugEvent(&e,1000)){need(GetLastError()==ERROR_SEM_TIMEOUT,"Wait debug event");need(GetTickCount64()-started<120000,"Source diagnostic deadline");continue;}
            pending=true;tid=e.dwThreadId;DWORD action=DBG_CONTINUE;
            if(e.dwDebugEventCode==CREATE_PROCESS_DEBUG_EVENT){std::cout<<"{\"event\":\"image\",\"base\":"<<reinterpret_cast<DWORD>(e.u.CreateProcessInfo.lpBaseOfImage)<<"}"<<std::endl;if(e.u.CreateProcessInfo.hFile)CloseHandle(e.u.CreateProcessInfo.hFile);CloseHandle(e.u.CreateProcessInfo.hThread);CloseHandle(e.u.CreateProcessInfo.hProcess);}
            else if(e.dwDebugEventCode==CREATE_THREAD_DEBUG_EVENT)CloseHandle(e.u.CreateThread.hThread);
            else if(e.dwDebugEventCode==LOAD_DLL_DEBUG_EVENT){if(e.u.LoadDll.hFile)CloseHandle(e.u.LoadDll.hFile);}
            else if(e.dwDebugEventCode==EXIT_PROCESS_DEBUG_EVENT){std::cout<<"{\"event\":\"exit\",\"exit_code\":"<<e.u.ExitProcess.dwExitCode<<"}"<<std::endl;done=true;}
            else if(e.dwDebugEventCode==EXCEPTION_DEBUG_EVENT){
                const auto& x=e.u.Exception.ExceptionRecord;
                if(x.ExceptionCode==EXCEPTION_BREAKPOINT&&!ready){ready=true;std::cout<<"{\"event\":\"ready\",\"pid\":"<<pid<<",\"game_memory_written\":false}"<<std::endl;}
                else{
                    action=DBG_EXCEPTION_NOT_HANDLED;
                    if(x.ExceptionCode==EXCEPTION_ACCESS_VIOLATION){
                        HANDLE thread=OpenThread(THREAD_GET_CONTEXT,FALSE,tid);need(thread!=nullptr,"Open exception thread");
                        CONTEXT c{};c.ContextFlags=CONTEXT_FULL;need(GetThreadContext(thread,&c)!=FALSE,"Read exception context");
                        std::cout<<"{\"event\":\"access_violation\",\"first_chance\":"<<e.u.Exception.dwFirstChance<<",\"eip\":"<<c.Eip<<",\"ecx\":"<<c.Ecx<<",\"eax\":"<<c.Eax<<",\"edx\":"<<c.Edx<<",\"esp\":"<<c.Esp<<",\"ebp\":"<<c.Ebp<<",\"access_kind\":"<<x.ExceptionInformation[0]<<",\"access_address\":"<<x.ExceptionInformation[1]<<",\"stack_words\":[";
                        std::array<DWORD,80> words{};SIZE_T bytes{};ReadProcessMemory(process,(void*)c.Esp,words.data(),sizeof(words),&bytes);
                        for(unsigned i=0;i<bytes/4;++i)std::cout<<(i?",":"")<<words[i];
                        std::cout<<"],\"frames\":[";bool comma=false;
                        STACKFRAME64 frame{};frame.AddrPC={c.Eip,0,AddrModeFlat};frame.AddrStack={c.Esp,0,AddrModeFlat};frame.AddrFrame={c.Ebp,0,AddrModeFlat};
                        for(unsigned i=0;symbols&&i<32;++i){
                            if(i&&!StackWalk64(IMAGE_FILE_MACHINE_I386,process,thread,&frame,&c,nullptr,SymFunctionTableAccess64,SymGetModuleBase64,nullptr))break;
                            if(!frame.AddrPC.Offset)break;
                            alignas(SYMBOL_INFO) char buffer[sizeof(SYMBOL_INFO)+2048]{};auto* s=reinterpret_cast<SYMBOL_INFO*>(buffer);s->SizeOfStruct=sizeof(SYMBOL_INFO);s->MaxNameLen=2047;DWORD64 displacement{};
                            std::cout<<(comma?",":"")<<"{\"address\":"<<frame.AddrPC.Offset;comma=true;
                            if(SymFromAddr(process,frame.AddrPC.Offset,&displacement,s))std::cout<<",\"symbol\":\""<<s->Name<<"\",\"displacement\":"<<displacement;
                            IMAGEHLP_LINE64 line{};line.SizeOfStruct=sizeof(line);DWORD delta{};
                            if(SymGetLineFromAddr64(process,frame.AddrPC.Offset,&delta,&line))std::cout<<",\"line\":"<<line.LineNumber;
                            std::cout<<"}";
                        }
                        std::cout<<"]}"<<std::endl;CloseHandle(thread);
                    }
                }
            }
            need(ContinueDebugEvent(e.dwProcessId,e.dwThreadId,action)!=FALSE,"Continue unchanged exception");pending=false;
        }
        attached=false;if(symbols)SymCleanup(process);CloseHandle(process);return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';if(pending)ContinueDebugEvent(pid,tid,DBG_EXCEPTION_NOT_HANDLED);if(attached)DebugActiveProcessStop(pid);if(symbols)SymCleanup(process);if(process)CloseHandle(process);return 1;}
}
