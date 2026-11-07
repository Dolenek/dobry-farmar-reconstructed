// Isolated real public API calls. Fault cases explicitly terminate this owned
// process after recording SEH; no continued use of a half-completed API call.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _USRDLL
#include <windows.h>
#include <float.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include "Squall.h"
struct Api {
#define SQUALL_ENTRY(name) decltype(&name) name{};
#include "squall_exports.inc"
#undef SQUALL_ENTRY
};
struct Outcome {std::uint32_t lead{0x5a5a5a5a};DWORD code{},access{};std::uintptr_t address{},instruction{};int result{-999};float direction[3]{-777,-777,-777};int inside{-777},angle{-777};std::uint32_t tail{0xa5a5a5a5};};
int filter(EXCEPTION_POINTERS* p,Outcome* out) {
    if(p->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION)return EXCEPTION_CONTINUE_SEARCH;
    out->code=p->ExceptionRecord->ExceptionCode;out->access=DWORD(p->ExceptionRecord->ExceptionInformation[0]);
    out->address=p->ExceptionRecord->ExceptionInformation[1];out->instruction=reinterpret_cast<std::uintptr_t>(p->ExceptionRecord->ExceptionAddress);
    return EXCEPTION_EXECUTE_HANDLER;
}
void invoke(Api* api,int method,int id,float* pos,float* vel,int channel,int* outside,Outcome* out) {
    __try {
        switch(method) {
        case 0:out->result=api->SQUALL_Sample_Play3D(id,1,17,0,pos,vel);break;
        case 1:out->result=api->SQUALL_Sample_Play3DEx(id,1,17,0,pos,vel,7,1,16000,1,1000);break;
        case 2:out->result=api->SQUALL_SampleGroup_Play3D(17,1,17,0,pos,vel);break;
        case 3:out->result=api->SQUALL_SampleGroup_Play3DEx(17,1,17,0,pos,vel,7,1,16000,1,1000);break;
        case 4:out->result=api->SQUALL_Channel_GetConeParameters(channel,out->direction,&out->inside,&out->angle,outside);break;
        }
    } __except(filter(GetExceptionInformation(),out)) {}
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=5)return 2;
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX);
    auto module=LoadLibraryW(std::filesystem::absolute(argv[1]).c_str());if(!module)return 3;
    Api api{};
#define SQUALL_ENTRY(name) api.name=reinterpret_cast<decltype(api.name)>(GetProcAddress(module,#name));if(!api.name)return 4;
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    int method=_wtoi(argv[2]),test=_wtoi(argv[3]),precision=_wtoi(argv[4]);
    if(method<0||method>4||test<0||test>(method==4?4:9)||(precision!=24&&precision!=53))return 5;
    _control87(precision==24?_PC_24:_PC_53,_MCW_PC);
    SYSTEM_INFO si{};GetSystemInfo(&si);const auto page=si.dwPageSize;
    auto region=static_cast<unsigned char*>(VirtualAlloc(nullptr,page*2,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));if(!region)return 6;
    DWORD old{};if(!VirtualProtect(region+page,page,PAGE_NOACCESS,&old))return 7;
    float position[3]={1,2,3},velocity[3]={4,5,6};float* pos=position;float* vel=velocity;
    auto edge=reinterpret_cast<float*>(region+page-12);for(unsigned i=0;i<3;++i)edge[i]=position[i];
    if(method<4)switch(test) {
    case 0:pos=nullptr;break;
    case 1:pos=reinterpret_cast<float*>(1);break;
    case 2:pos=reinterpret_cast<float*>(region+page);break;
    case 3:pos=reinterpret_cast<float*>(region+page-11);break;
    case 4:pos=edge;if(!VirtualProtect(region,page,PAGE_READONLY,&old))return 8;break;
    case 5:pos=edge;break;
    case 6:vel=nullptr;break;
    case 7:vel=reinterpret_cast<float*>(1);break;
    case 8:vel=reinterpret_cast<float*>(region+page);break;
    }
    int outsideValue=-777;int* outside=&outsideValue;
    if(method==4)switch(test) {
    case 0:outside=nullptr;break;
    case 1:outside=reinterpret_cast<int*>(1);break;
    case 2:outside=reinterpret_cast<int*>(region+page);break;
    case 3:outside=reinterpret_cast<int*>(region);if(!VirtualProtect(region,page,PAGE_READONLY,&old))return 9;break;
    }
    auto window=CreateWindowExW(WS_EX_NOACTIVATE,L"STATIC",L"Squall isolated pointer probe",0,0,0,64,64,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);if(!window)return 10;
    squall_sample_default_t def{};def.SampleGroupID=17;def.Priority=7;def.Frequency=16000;def.Volume=1;def.Pan=50;def.MinDist=1;def.MaxDist=1000;
    char filename[]="sample.wav";const auto sample=api.SQUALL_Sample_LoadFile(filename,1,&def);if(sample<=0)return 11;
    squall_parameters_t parameters{};parameters.Window=window;parameters.SampleRate=44100;parameters.BitPerSample=16;parameters.Channels=4;parameters.UseAlg=1;parameters.BufferSize=200;parameters.DistanceFactor=1;parameters.RolloffFactor=.001f;parameters.DopplerFactor=1;
    auto initialized=api.SQUALL_Init(&parameters);if(initialized<0)return 12;
    int channel=-1;if(method==4){channel=api.SQUALL_Sample_Play3DEx(sample,1,17,0,position,velocity,7,1,16000,1,1000);if(channel<0)return 13;}
    const auto requestedPrecision=precision==24?_PC_24:_PC_53;
    const auto beforePrecision=_control87(0,0)&_MCW_PC;if(beforePrecision!=requestedPrecision)return 15;
    Outcome out{};invoke(&api,method,sample,pos,vel,channel,outside,&out);
    const auto afterPrecision=_control87(0,0)&_MCW_PC;if(afterPrecision!=requestedPrecision)return 16;
    std::uint32_t cachedPos[3]{},cachedVel[3]{};int positionResult=-999,velocityResult=-999;
    if(!out.code&&method<4&&out.result>=0){positionResult=api.SQUALL_Channel_Get3DPosition(out.result,reinterpret_cast<float*>(cachedPos));velocityResult=api.SQUALL_Channel_GetVelocity(out.result,reinterpret_cast<float*>(cachedVel));}
    const bool fault=out.code!=0;
    std::uint32_t direction[3]{};std::memcpy(direction,out.direction,12);
    std::printf("{\"method\":%d,\"case\":%d,\"precision\":%d,\"sample\":%d,\"initialized\":%d,\"exception\":%lu,\"access\":%lu,\"fault_address\":%llu,\"instruction_rva\":%llu,\"result\":%d,\"outside\":%d,\"position_result\":%d,\"velocity_result\":%d,\"position_words\":[%u,%u,%u],\"velocity_words\":[%u,%u,%u],\"direction_words\":[%u,%u,%u],\"inside_angle\":%d,\"outside_angle\":%d,\"lead_guard\":%u,\"tail_guard\":%u,\"before_precision_bits\":%u,\"after_precision_bits\":%u,\"explicit_fault_process_termination\":%s}\n",
        method,test,precision,sample,initialized,out.code,out.access,static_cast<unsigned long long>(out.address),
        static_cast<unsigned long long>(fault?out.instruction-reinterpret_cast<std::uintptr_t>(module):0),out.result,outsideValue,
        positionResult,velocityResult,cachedPos[0],cachedPos[1],cachedPos[2],cachedVel[0],cachedVel[1],cachedVel[2],direction[0],direction[1],direction[2],out.inside,out.angle,out.lead,out.tail,beforePrecision,afterPrecision,fault?"true":"false");
    std::fflush(stdout);
    if(fault){TerminateProcess(GetCurrentProcess(),0);return 14;}
    api.SQUALL_Free();api.SQUALL_Sample_UnloadAll();DestroyWindow(window);VirtualFree(region,0,MEM_RELEASE);FreeLibrary(module);return 0;
}
