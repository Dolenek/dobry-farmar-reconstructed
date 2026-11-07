// Test supervisor. It never opens graphics devices, sends input, or suspends
// running UI threads. A named Job owns only the launched test process tree.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

namespace fs=std::filesystem;
void need(bool ok,const char* message) {if(!ok) throw std::runtime_error(std::string(message)+" Win32="+std::to_string(GetLastError()));}
std::string utf8(const std::wstring& value) {
    int size=WideCharToMultiByte(CP_UTF8,0,value.c_str(),int(value.size()),nullptr,0,nullptr,nullptr);
    std::string result(size,'\0');WideCharToMultiByte(CP_UTF8,0,value.c_str(),int(value.size()),result.data(),size,nullptr,nullptr);return result;
}
std::string json(const std::wstring& value) {
    std::string out="\"";for(char c:utf8(value)) {if(c=='\\'||c=='"') out+='\\';out+=c;}return out+'"';
}
std::wstring quoted(const std::wstring& arg) {
    std::wstring result=L"\"";unsigned slashes=0;
    for(wchar_t c:arg) {
        if(c==L'\\') {++slashes;continue;}
        if(c==L'"') {result.append(slashes*2+1,L'\\');result+=c;}
        else {result.append(slashes,L'\\');result+=c;}slashes=0;
    }
    result.append(slashes*2,L'\\');return result+L'"';
}
unsigned long long ticks(FILETIME value) {return (static_cast<unsigned long long>(value.dwHighDateTime)<<32)|value.dwLowDateTime;}
unsigned long long written(const fs::path& path) {
    WIN32_FILE_ATTRIBUTE_DATA value{};return GetFileAttributesExW(path.c_str(),GetFileExInfoStandard,&value) ? ticks(value.ftLastWriteTime):0;
}
unsigned liveOwned(HANDLE job) {
    std::vector<ULONG_PTR> storage(1026);
    auto* list=reinterpret_cast<JOBOBJECT_BASIC_PROCESS_ID_LIST*>(storage.data());
    need(QueryInformationJobObject(job,JobObjectBasicProcessIdList,list,DWORD(storage.size()*sizeof(ULONG_PTR)),nullptr),"Read owned process IDs");
    unsigned count=0;
    for(DWORD i=0;i<list->NumberOfProcessIdsInList;++i) {
        HANDLE process=OpenProcess(SYNCHRONIZE|PROCESS_QUERY_LIMITED_INFORMATION,FALSE,DWORD(list->ProcessIdList[i]));
        if(!process) {need(GetLastError()==ERROR_INVALID_PARAMETER,"Verify owned process lifetime");continue;}
        const auto state=WaitForSingleObject(process,0);
        if(state==WAIT_TIMEOUT) {
            wchar_t image[32768]{};DWORD length=32768;
            QueryFullProcessImageNameW(process,0,image,&length);
            std::cerr<<"Live owned PID="<<DWORD(list->ProcessIdList[i])<<" image="<<utf8(image)<<"\n";
        }
        CloseHandle(process);
        need(state==WAIT_OBJECT_0||state==WAIT_TIMEOUT,"Read owned process lifetime");
        if(state==WAIT_TIMEOUT) ++count;
    }
    return count;
}
int wmain(int argc,wchar_t** argv) {
    HANDLE job=nullptr;PROCESS_INFORMATION process{};std::ofstream log;
    try {
        need(argc>=6,"Expected run-directory max-seconds stale-ms executable arguments...");
        const auto directory=fs::absolute(argv[1]);const auto seconds=std::stoul(argv[2]);const auto stale=std::stoul(argv[3]);
        need(seconds>=1&&seconds<=900&&stale>=500&&stale<=30000,"Bounded watchdog limits required");
        fs::create_directories(directory);need(!fs::exists(directory/L"lease.json"),"Refusing to overwrite a prior run lease");
        log.open(directory/L"watchdog.jsonl",std::ios::binary);need(bool(log),"Cannot open watchdog log");
        const auto now=GetTickCount64();const auto deadline=now+seconds*1000;
        const auto name=L"Local\\FarmarHostTest-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(now);
        job=CreateJobObjectW(nullptr,name.c_str());need(job!=nullptr&&GetLastError()!=ERROR_ALREADY_EXISTS,"Create unique test job");
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
        limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE|JOB_OBJECT_LIMIT_JOB_MEMORY;
        limits.JobMemoryLimit=static_cast<SIZE_T>(2ULL*1024*1024*1024);
        need(SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits)),"Set kill-on-close/memory bounds");
        auto* inherited=GetEnvironmentStringsW();need(inherited!=nullptr,"Read environment");std::vector<wchar_t> environment;
        for(auto* entry=inherited;*entry;entry+=wcslen(entry)+1) {
            if(_wcsnicmp(entry,L"FARMAR_HOST_WATCHDOG_LEASE=",wcslen(L"FARMAR_HOST_WATCHDOG_LEASE="))!=0)
                environment.insert(environment.end(),entry,entry+wcslen(entry)+1);
        }
        FreeEnvironmentStringsW(inherited);
        const auto leasePath=directory/L"lease.json";
        const auto leaseVariable=L"FARMAR_HOST_WATCHDOG_LEASE="+leasePath.wstring();
        environment.insert(environment.end(),leaseVariable.c_str(),leaseVariable.c_str()+leaseVariable.size()+1);environment.push_back(0);
        std::wstring command;
        for(int i=4;i<argc;++i) {if(i>4) command+=L' ';command+=quoted(argv[i]);}
        STARTUPINFOW startup{sizeof(startup)};startup.dwFlags=STARTF_USESTDHANDLES;
        SECURITY_ATTRIBUTES attributes{sizeof(attributes),nullptr,TRUE};
        HANDLE out=CreateFileW((directory/L"driver-stdout.log").c_str(),GENERIC_WRITE,FILE_SHARE_READ,&attributes,CREATE_NEW,0,nullptr);
        HANDLE err=CreateFileW((directory/L"driver-stderr.log").c_str(),GENERIC_WRITE,FILE_SHARE_READ,&attributes,CREATE_NEW,0,nullptr);
        HANDLE input=CreateFileW(L"NUL",GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,&attributes,OPEN_EXISTING,0,nullptr);
        need(out!=INVALID_HANDLE_VALUE&&err!=INVALID_HANDLE_VALUE&&input!=INVALID_HANDLE_VALUE,"Open owned child streams");
        startup.hStdOutput=out;startup.hStdError=err;startup.hStdInput=input;
        need(CreateProcessW(argv[4],command.data(),nullptr,nullptr,TRUE,
            CREATE_SUSPENDED|CREATE_NO_WINDOW|CREATE_UNICODE_ENVIRONMENT,environment.data(),nullptr,&startup,&process),"Create test before first instruction");
        CloseHandle(out);CloseHandle(err);CloseHandle(input);
        need(AssignProcessToJobObject(job,process.hProcess),"Assign test to owned job before entry");
        {
            std::ofstream lease(leasePath,std::ios::binary);need(bool(lease),"Write runtime lease");
            lease<<"{\"schema\":1,\"guardian_pid\":"<<GetCurrentProcessId()<<",\"driver_pid\":"<<process.dwProcessId
                <<",\"job_name\":"<<json(name)<<",\"deadline_tick\":"<<deadline
                <<",\"stale_progress_ms\":"<<stale<<",\"job_kill_on_close\":true}";
        }
        log<<"{\"event\":\"started\",\"guardian_pid\":"<<GetCurrentProcessId()<<",\"driver_pid\":"<<process.dwProcessId
            <<",\"maximum_seconds\":"<<seconds<<",\"stale_progress_ms\":"<<stale<<"}\n"<<std::flush;
        need(ResumeThread(process.hThread)!=DWORD(-1),"Start newly created test");CloseHandle(process.hThread);process.hThread=nullptr;
        auto progress=written(directory/L"progress.json");auto lastProgress=GetTickCount64(),lastLog=lastProgress,lowMemorySince=0ULL,highCPUSince=0ULL;
        FILETIME previousIdle{},previousKernel{},previousUser{};need(GetSystemTimes(&previousIdle,&previousKernel,&previousUser),"Read CPU baseline");
        int failure=0;const char* reason=nullptr;
        while(WaitForSingleObject(process.hProcess,100)==WAIT_TIMEOUT) {
            const auto current=GetTickCount64();const auto mark=written(directory/L"progress.json");
            if(mark!=progress) {progress=mark;lastProgress=current;}
            if(current>=deadline) {failure=124;reason="absolute_time_limit";break;}
            if(current-lastProgress>stale) {failure=125;reason="driver_progress_stalled";break;}
            MEMORYSTATUSEX memory{sizeof(memory)};need(GlobalMemoryStatusEx(&memory),"Read memory pressure");
            if(memory.ullAvailPhys<512ULL*1024*1024) {if(!lowMemorySince) lowMemorySince=current;}
            else lowMemorySince=0;
            if(lowMemorySince&&current-lowMemorySince>2000) {failure=126;reason="host_available_memory_low";break;}
            if(current-lastLog>=1000) {
                FILETIME idle{},kernel{},user{};need(GetSystemTimes(&idle,&kernel,&user),"Read CPU use");
                const auto total=ticks(kernel)-ticks(previousKernel)+ticks(user)-ticks(previousUser);
                const auto idleDelta=ticks(idle)-ticks(previousIdle);
                const double cpu=total ? 100.0*double(total-idleDelta)/double(total):0;
                previousIdle=idle;previousKernel=kernel;previousUser=user;
                if(cpu>95) {if(!highCPUSince) highCPUSince=current;}else highCPUSince=0;
                JOBOBJECT_BASIC_ACCOUNTING_INFORMATION accounting{};
                need(QueryInformationJobObject(job,JobObjectBasicAccountingInformation,&accounting,sizeof(accounting),nullptr),"Read owned job accounting");
                JOBOBJECT_EXTENDED_LIMIT_INFORMATION used{};
                need(QueryInformationJobObject(job,JobObjectExtendedLimitInformation,&used,sizeof(used),nullptr),"Read owned job memory");
                log<<"{\"event\":\"health\",\"elapsed_ms\":"<<current-now<<",\"progress_age_ms\":"<<current-lastProgress
                    <<",\"host_cpu_percent\":"<<cpu<<",\"host_available_physical_bytes\":"<<memory.ullAvailPhys
                    <<",\"owned_active_processes\":"<<accounting.ActiveProcesses
                    <<",\"owned_peak_job_memory_bytes\":"<<used.PeakJobMemoryUsed<<"}\n"<<std::flush;
                lastLog=current;
                if(highCPUSince&&current-highCPUSince>5000) {failure=126;reason="host_CPU_saturated";break;}
            }
        }
        if(failure) {
            log<<"{\"event\":\"terminated_owned_job\",\"reason\":\""<<reason<<"\"}\n"<<std::flush;
            TerminateJobObject(job,DWORD(failure));WaitForSingleObject(process.hProcess,5000);
        }
        DWORD code=0;need(GetExitCodeProcess(process.hProcess,&code),"Read test exit");
        JOBOBJECT_BASIC_ACCOUNTING_INFORMATION accounting{};
        need(QueryInformationJobObject(job,JobObjectBasicAccountingInformation,&accounting,sizeof(accounting),nullptr),"Read final owned processes");
        // Job accounting may still include the just-signalled root process.
        // Check actual process lifetimes instead of mistaking that for an orphan.
        auto alive=liveOwned(job);
        for(unsigned settle=0;alive&&settle<20&&!failure;++settle) {Sleep(25);alive=liveOwned(job);}
        if(!failure&&alive) {failure=127;reason="driver_left_owned_processes";TerminateJobObject(job,127);}
        log<<"{\"event\":\"finished\",\"driver_exit_code\":"<<code<<",\"watchdog_exit_code\":"<<failure
            <<",\"owned_processes_before_final_close\":"<<accounting.ActiveProcesses
            <<",\"live_owned_processes_before_final_close\":"<<alive<<"}\n"<<std::flush;
        CloseHandle(process.hProcess);process.hProcess=nullptr;CloseHandle(job);job=nullptr;
        std::cout<<"{\"watchdog_exit_code\":"<<failure<<",\"driver_exit_code\":"<<code<<"}\n";
        return failure ? failure : int(code);
    } catch(const std::exception& error) {
        if(job) TerminateJobObject(job,127);
        if(process.hProcess) {TerminateProcess(process.hProcess,127);CloseHandle(process.hProcess);}
        if(process.hThread) CloseHandle(process.hThread);
        if(job) CloseHandle(job);
        if(log) log<<"{\"event\":\"watchdog_error\"}\n"<<std::flush;
        std::cerr<<error.what()<<"\n";return 127;
    }
}
