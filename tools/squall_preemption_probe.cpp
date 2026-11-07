// Real exported channel exhaustion/preemption in one isolated owned process.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _USRDLL
#include <windows.h>
#include <float.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include "Squall.h"
struct Api {
#define SQUALL_ENTRY(name) decltype(&name) name{};
#include "squall_exports.inc"
#undef SQUALL_ENTRY
};
struct Outcome {DWORD code{},access{};std::uintptr_t address{},instruction{};int result{-999};};
int filter(EXCEPTION_POINTERS* p,Outcome* out) {
    if(p->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
    out->code=p->ExceptionRecord->ExceptionCode;out->access=DWORD(p->ExceptionRecord->ExceptionInformation[0]);
    out->address=p->ExceptionRecord->ExceptionInformation[1];out->instruction=reinterpret_cast<std::uintptr_t>(p->ExceptionRecord->ExceptionAddress);
    return EXCEPTION_EXECUTE_HANDLER;
}
void invoke(Api* api,int method,int id,int priority,Outcome* out) {
    float position[3]={1,2,3},velocity[3]={4,5,6};
    __try {
        switch(method) {
        case 0:out->result=api->SQUALL_Sample_Play(id,1,17,0);break;
        case 1:out->result=api->SQUALL_Sample_PlayEx(id,1,17,0,priority,1,16000,50);break;
        case 2:out->result=api->SQUALL_Sample_Play3D(id,1,17,0,position,velocity);break;
        case 3:out->result=api->SQUALL_Sample_Play3DEx(id,1,17,0,position,velocity,priority,1,16000,1,1000);break;
        case 4:out->result=api->SQUALL_SampleGroup_Play(17,1,17,0);break;
        case 5:out->result=api->SQUALL_SampleGroup_PlayEx(17,1,17,0,priority,1,16000,50);break;
        case 6:out->result=api->SQUALL_SampleGroup_Play3D(17,1,17,0,position,velocity);break;
        case 7:out->result=api->SQUALL_SampleGroup_Play3DEx(17,1,17,0,position,velocity,priority,1,16000,1,1000);break;
        }
    } __except(filter(GetExceptionInformation(),out)) {}
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=6)return 2;
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX);
    const int method=_wtoi(argv[2]),priority=_wtoi(argv[3]),precision=_wtoi(argv[4]),pattern=_wtoi(argv[5]);
    if(method<0||method>7||priority<0||(precision!=24&&precision!=53)||pattern<0||pattern>2)return 3;
    const auto requested=precision==24?_PC_24:_PC_53;_control87(requested,_MCW_PC);
    auto module=LoadLibraryW(std::filesystem::absolute(argv[1]).c_str());if(!module)return 4;
    const auto loadedPC=_control87(0,0)&_MCW_PC;
    Api api{};
#define SQUALL_ENTRY(name) api.name=reinterpret_cast<decltype(api.name)>(GetProcAddress(module,#name));if(!api.name)return 5;
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    // Loading a CRT-bearing DLL can reset the caller's FP environment. Set
    // the tested precision after loading, then measure it at the actual call.
    _control87(requested,_MCW_PC);
    auto window=CreateWindowExW(WS_EX_NOACTIVATE,L"STATIC",L"Squall isolated preemption probe",0,0,0,64,64,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);if(!window)return 6;
    squall_sample_default_t def{};def.SampleGroupID=17;def.Priority=7;def.Frequency=16000;def.Volume=1;def.Pan=50;def.MinDist=1;def.MaxDist=1000;
    char filename[]="sample.wav";const auto sample=api.SQUALL_Sample_LoadFile(filename,1,&def);if(sample<=0)return 7;
    squall_parameters_t p{};p.Window=window;p.SampleRate=44100;p.BitPerSample=16;p.Channels=4;p.UseAlg=1;p.BufferSize=200;p.DistanceFactor=1;p.RolloffFactor=.001f;p.DopplerFactor=1;
    if(api.SQUALL_Init(&p)!=1)return 8;
    const int ascending[4]={20,30,40,50},ties[4]={20,20,20,20},mixed[4]={50,20,20,40};
    const int* priorities=pattern==0?ascending:pattern==1?ties:mixed;
    int ids[4]{},beforeStatus[4]{},beforePriority[4]{},afterStatus[4]={-999,-999,-999,-999},afterPriority[4]={-999,-999,-999,-999};
    for(int i=0;i<4;++i) {
        ids[i]=api.SQUALL_Sample_PlayEx(sample,1,17,0,priorities[i],1,16000,50);if(ids[i]<0)return 9;
        beforeStatus[i]=api.SQUALL_Channel_Status(ids[i]);beforePriority[i]=api.SQUALL_Channel_GetPriority(ids[i]);
        if(beforeStatus[i]!=3||beforePriority[i]!=priorities[i])return 10;
    }
    def.Priority=priority;if(api.SQUALL_Sample_SetDefault(sample,&def)!=1)return 11;
    const auto beforePC=_control87(0,0)&_MCW_PC;if(beforePC!=requested)return 12;
    Outcome out{};invoke(&api,method,sample,priority,&out);
    const auto afterPC=_control87(0,0)&_MCW_PC;if(afterPC!=requested)return 13;
    int newStatus=-999,newPriority=-999;
    if(!out.code) {
        for(int i=0;i<4;++i){afterStatus[i]=api.SQUALL_Channel_Status(ids[i]);afterPriority[i]=api.SQUALL_Channel_GetPriority(ids[i]);}
        newStatus=api.SQUALL_Channel_Status(out.result);newPriority=api.SQUALL_Channel_GetPriority(out.result);
    }
    std::printf("{\"method\":%d,\"priority\":%d,\"precision\":%d,\"pattern\":%d,\"exception\":%lu,\"access\":%lu,\"fault_address\":%llu,\"instruction_rva\":%llu,\"result\":%d,\"new_status\":%d,\"new_priority\":%d,\"loaded_precision_bits\":%u,\"before_precision_bits\":%u,\"after_precision_bits\":%u,\"channels\":[",
        method,priority,precision,pattern,out.code,out.access,static_cast<unsigned long long>(out.address),
        static_cast<unsigned long long>(out.code?out.instruction-reinterpret_cast<std::uintptr_t>(module):0),out.result,newStatus,newPriority,loadedPC,beforePC,afterPC);
    for(int i=0;i<4;++i)std::printf("%s{\"id\":%d,\"before_status\":%d,\"before_priority\":%d,\"after_status\":%d,\"after_priority\":%d}",i?",":"",ids[i],beforeStatus[i],beforePriority[i],afterStatus[i],afterPriority[i]);
    std::printf("],\"explicit_fault_process_termination\":%s}\n",out.code?"true":"false");std::fflush(stdout);
    if(out.code){TerminateProcess(GetCurrentProcess(),0);return 14;}
    api.SQUALL_Free();api.SQUALL_Sample_UnloadAll();DestroyWindow(window);FreeLibrary(module);return 0;
}
