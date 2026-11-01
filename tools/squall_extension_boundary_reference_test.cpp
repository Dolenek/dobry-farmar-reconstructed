// Diagnostic only: restore private DLL context bytes after every owned fixture.
// Production DLLs are unchanged. The reference is never an implementation dependency.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include "Squall.h"
#include "SoundEngine/SquallApi.h"
#include <array>
#include <algorithm>
#include <vector>
#include <filesystem>
#include <float.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
namespace {
void need(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
static_assert(sizeof(SSample)==300 && offsetof(SSample,DS3D_PropSet)==0x10,"Native channel layout");
static_assert(offsetof(SSample,Param)+offsetof(SChannelParam,EAXBP)==0x94,"Native EAX cache");
static_assert(offsetof(SSample,Param)+offsetof(SChannelParam,ZOOMFXBP)==0xdc,"Native ZOOMFX cache");
static_assert(offsetof(context_t,_useEax)==4 && offsetof(context_t,_eaxSupport)==8 && offsetof(context_t,_eaxListener)==0x1c && offsetof(context_t,_eaxLP)==0x20 && offsetof(context_t,_zoomfxSupport)==0x90 && offsetof(context_t,_channelsArray)==0x100,"Native context instruction offsets");
struct Api {
#define SQUALL_ENTRY(name) decltype(&name) name{};
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    HMODULE module{};context_t* context{};
    explicit Api(const wchar_t* path,std::uintptr_t contextRva){module=LoadLibraryW(std::filesystem::absolute(path).c_str());need(module!=nullptr,"Load DLL");
#define SQUALL_ENTRY(name) name=reinterpret_cast<decltype(name)>(GetProcAddress(module,#name));need(name!=nullptr,#name);
#include "squall_exports.inc"
#undef SQUALL_ENTRY
        need(SQUALL_GetNumDevice()>0,"Actual device enumeration failed");context=reinterpret_cast<context_t*>(reinterpret_cast<std::uintptr_t>(module)+contextRva);
        MEMORY_BASIC_INFORMATION info{};need(VirtualQuery(context,&info,sizeof(info))==sizeof(info)&&info.AllocationBase==module&&(info.Protect&PAGE_READWRITE),"Context outside owned writable DLL");
    }
    ~Api(){if(module){SQUALL_Free();SQUALL_Sample_UnloadAll();FreeLibrary(module);}}
};
struct Preserve {context_t* pointer;std::array<unsigned char,sizeof(context_t)> original;
    explicit Preserve(context_t* p):pointer(p){std::memcpy(original.data(),p,original.size());}
    ~Preserve(){std::memcpy(pointer,original.data(),original.size());}
};
struct Event {
    unsigned operation,id,size;GUID guid;std::vector<unsigned char> bytes;
    bool operator==(const Event& b)const{return operation==b.operation&&id==b.id&&size==b.size&&!std::memcmp(&guid,&b.guid,sizeof(guid))&&bytes==b.bytes;}
};
struct Boundary:IKsPropertySet {
    std::vector<Event> events;HRESULT setResult=S_OK,getResult=S_OK;unsigned written=~0U,reported=~0U;
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID,void**)override{return E_NOINTERFACE;}
    ULONG STDMETHODCALLTYPE AddRef()override{return 1;}
    ULONG STDMETHODCALLTYPE Release()override{return 1;}
    HRESULT STDMETHODCALLTYPE Get(REFGUID guid,ULONG id,LPVOID instance,ULONG instanceBytes,LPVOID data,ULONG bytes,PULONG returned)override{
        need(!instance&&!instanceBytes&&data&&returned&&bytes<=112,"Unexpected Get COM arguments");
        auto* begin=static_cast<unsigned char*>(data);events.push_back({0,id,bytes,guid,std::vector<unsigned char>(begin,begin+bytes)});
        for(unsigned i=0;i<(std::min)(unsigned(bytes),written);++i)begin[i]=static_cast<unsigned char>(0x31+i*7);
        *returned=reported==~0U?bytes:reported;return getResult;
    }
    HRESULT STDMETHODCALLTYPE Set(REFGUID guid,ULONG id,LPVOID instance,ULONG instanceBytes,LPVOID data,ULONG bytes)override{
        need(!instance&&!instanceBytes&&data&&bytes<=112,"Unexpected Set COM arguments");
        auto* begin=static_cast<unsigned char*>(data);events.push_back({1,id,bytes,guid,std::vector<unsigned char>(begin,begin+bytes)});return setResult;
    }
    HRESULT STDMETHODCALLTYPE QuerySupport(REFGUID,ULONG,PULONG)override{throw std::runtime_error("Unexpected QuerySupport in exported property method");}
};
struct Fixture {
    context_t context{};std::array<SSample,2> channels{};Boundary boundary;
    void configure(int support,unsigned boundaryMode,int soundType=TYPE_POSITIONED){
        context={};channels={};boundary.events.clear();boundary.setResult=boundaryMode==1?E_FAIL:S_OK;boundary.getResult=boundaryMode==2?E_FAIL:S_OK;
        boundary.written=boundaryMode==3?3:~0U;boundary.reported=boundaryMode==3?0:~0U;
        context._threadID=GetCurrentThreadId();context._useEax=1;for(auto& supported:context._eaxSupport)supported=support;
        context._zoomfxSupport=support;context._channels=2;context._channelsArray=channels.data();context._eaxListener=boundaryMode==4?nullptr:&boundary;
        auto fill=[](void* p,unsigned size){auto* b=static_cast<unsigned char*>(p);for(unsigned i=0;i<size;++i)b[i]=static_cast<unsigned char>(0xa3+i*13);};
        fill(&context._eaxLP,sizeof(context._eaxLP));
        for(unsigned i=0;i<2;++i){auto& c=channels[i];c._count=1;c.ChannelID=256+int(i);c.Status.STAGE=PREPARE;c.Status.SOUND_TYPE=soundType;c.DS3D_PropSet=boundaryMode==4?nullptr:&boundary;c.DS_PropSet=c.DS3D_PropSet;fill(&c.Param.EAXBP,sizeof(c.Param.EAXBP));fill(&c.Param.ZOOMFXBP,sizeof(c.Param.ZOOMFXBP));}
    }
    void install(Api& api){std::memcpy(api.context,&context,sizeof(context));}
};
unsigned cases{},results{},snapshots{},comSnapshots{},presets{},method{},caseId{};
std::array<unsigned,7> methods{},comCalls{};
void same(int a,int b){++results;if(a!=b){std::cerr<<"Result method="<<method<<" case="<<caseId<<" source="<<a<<" native="<<b<<'\n';throw std::runtime_error("Extension boundary result differs");}}
int call(Api& api,unsigned method,int version,int channel,void* input){
    switch(method){
    case 0:return api.SQUALL_Listener_EAX_SetProperties(version,static_cast<squall_eax_listener_t*>(input));
    case 1:return api.SQUALL_Listener_EAX_GetProperties(version,static_cast<squall_eax_listener_t*>(input));
    case 2:return api.SQUALL_Channel_EAX_SetProperties(channel,version,static_cast<squall_eax_channel_t*>(input));
    case 3:return api.SQUALL_Channel_EAX_GetProperties(channel,version,static_cast<squall_eax_channel_t*>(input));
    case 4:return api.SQUALL_Channel_ZOOMFX_SetProperties(channel,static_cast<squall_zoomfx_channel_t*>(input));
    case 5:return api.SQUALL_Channel_ZOOMFX_GetProperties(channel,static_cast<squall_zoomfx_channel_t*>(input));
    default:return api.SQUALL_Listener_EAX_SetPreset(version);
    }
}
void inspect(Api& s,Api& n,Fixture& own,Fixture& native){
    need(!std::memcmp(&s.context->_eaxLP,&n.context->_eaxLP,sizeof(squall_eax_listener_t)),"Listener cached words differ");
    for(unsigned i=0;i<2;++i){need(!std::memcmp(&own.channels[i].Param.EAXBP,&native.channels[i].Param.EAXBP,sizeof(squall_eax_channel_t)),"Channel EAX cached words differ");need(!std::memcmp(&own.channels[i].Param.ZOOMFXBP,&native.channels[i].Param.ZOOMFXBP,sizeof(squall_zoomfx_channel_t)),"Channel ZOOMFX cached words differ");}
    ++snapshots;
    if(own.boundary.events!=native.boundary.events){std::cerr<<"COM method="<<method<<" case="<<caseId<<" source="<<own.boundary.events.size()<<" native="<<native.boundary.events.size()<<'\n';throw std::runtime_error("COM GUID/property/size/payload differs");}
    comCalls[method]+=unsigned(native.boundary.events.size());++comSnapshots;
}
void one(Api& s,Api& n,Fixture& own,Fixture& native,int support,unsigned mode,int version,int channel,int soundType,int useEax=1){
    ++cases;++caseId;++methods[method];own.configure(support,mode,soundType);native.configure(support,mode,soundType);own.context._useEax=native.context._useEax=useEax;
    own.install(s);native.install(n);
    std::array<unsigned char,160>a,b;for(unsigned i=0;i<a.size();++i)a[i]=static_cast<unsigned char>(i*29+0x17);b=a;
    const auto nr=call(n,method,version,channel,b.data()+16),sr=call(s,method,version,channel,a.data()+16);same(sr,nr);need(a==b,"Caller property bytes/guards differ");
    inspect(s,n,own,native);
}
}
int wmain(int argc,wchar_t** argv){
    if(argc<4||argc>5)return 2;
    try{
        const auto sourceRva=std::stoul(argv[3],nullptr,16);Api native(argv[1],0x75034),source(argv[2],sourceRva);
        Preserve originalNative(native.context),originalSource(source.context);Fixture own,original;
        const auto precision=argc==5?_PC_24:_PC_53;_control87(precision,_MCW_PC);
        for(method=0;method<6;++method)for(int support:{0,1,-1})for(unsigned mode=0;mode<5;++mode)for(int version:{INT_MIN,-1,0,1,2,3,4,INT_MAX}){
            if(method>=4&&version!=0)continue;
            for(int channel:{256,257,-1,0,0x7fffffff}){
                if(method<2&&channel!=256)continue;
                for(int soundType:{TYPE_POSITIONED,TYPE_AMBIENT}){
                    if(method<2&&soundType==TYPE_AMBIENT)continue;
                    one(source,native,own,original,support,mode,version,channel,soundType);
                }
            }
        }
        method=6;
        for(int useEax:{0,1,2,3,4,-1})for(unsigned mode=0;mode<5;++mode)for(int preset:{INT_MIN,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,INT_MAX}){
            one(source,native,own,original,1,mode,preset,256,TYPE_POSITIONED,useEax);++presets;
        }
        need((_control87(0,0)&_MCW_PC)==precision,"Precision changed");
        std::cout<<"{\"fixture_cases\":"<<cases<<",\"result_comparisons\":"<<results<<",\"cached_and_caller_snapshots\":"<<snapshots<<",\"com_event_snapshots\":"<<comSnapshots<<",\"preset_cases\":"<<presets<<",\"export_lookups\":194,\"original_entry_point_executed\":true,\"actual_hardware_extension_compared\":false,\"private_context_fixture_only\":true,\"source_context_rva\":"<<sourceRva<<",\"context_bytes\":"<<sizeof(context_t)<<",\"method_cases\":[";
        for(unsigned i=0;i<7;++i){if(i)std::cout<<',';std::cout<<methods[i];}std::cout<<"],\"com_calls\":[";for(unsigned i=0;i<7;++i){if(i)std::cout<<',';std::cout<<comCalls[i];}std::cout<<"]}\n";return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
