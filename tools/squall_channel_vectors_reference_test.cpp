// Actual DLL channel 3D comparison; original is a diagnostic oracle only.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include "Squall.h"
#include <array>
#include <vector>
#include <filesystem>
#include <float.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstddef>
#include "SoundEngine/Channels.h"
namespace {
void require(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
struct Api {
#define SQUALL_ENTRY(name) decltype(&name) name{};
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    HMODULE module{};
    explicit Api(const wchar_t* path){
        module=LoadLibraryW(std::filesystem::absolute(path).c_str());require(module!=nullptr,"LoadLibrary failed");
#define SQUALL_ENTRY(name) name=reinterpret_cast<decltype(name)>(GetProcAddress(module,#name));require(name!=nullptr,#name);
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    }
    ~Api(){if(module){SQUALL_Free();FreeLibrary(module);}}
};
unsigned comparisons=0,snapshots=0,setters=0,contextId=0,caseId=0,distanceCases=0,coneCases=0,pointerCases=0;
void same(int source,int native,const char* name){
    ++comparisons;if(source!=native){std::cerr<<name<<" context="<<contextId<<" case="<<caseId<<" source="<<source<<" native="<<native<<'\n';throw std::runtime_error("Actual channel 3D result differs");}
}
std::vector<unsigned char> wave;unsigned cursor=0,opens=0,closes=0;
unsigned __cdecl open(const char*){cursor=0;++opens;return 1;}
int __cdecl seek(unsigned,int offset,int origin){
    const auto base=origin==0 ? 0:origin==1 ? int(cursor):int(wave.size());
    const auto next=static_cast<std::int64_t>(base)+offset;if(next<0||next>int(wave.size()))return -1;
    cursor=unsigned(next);return int(cursor);
}
int __cdecl read(unsigned,void* out,int size){
    if(size<0)return 0;const auto count=(std::min)(unsigned(size),unsigned(wave.size())-cursor);
    std::memcpy(out,wave.data()+cursor,count);cursor+=count;return int(count);
}
void __cdecl close(unsigned){++closes;}
void createWave(){
    const unsigned rate=16000,bytes=rate*2;wave.assign(44+bytes,0);
    auto put=[&](unsigned at,unsigned value,unsigned size){std::memcpy(wave.data()+at,&value,size);};
    std::memcpy(wave.data(),"RIFF",4);put(4,36+bytes,4);std::memcpy(wave.data()+8,"WAVEfmt ",8);
    put(16,16,4);put(20,1,2);put(22,1,2);put(24,rate,4);put(28,rate*2,4);put(32,2,2);put(34,16,2);
    std::memcpy(wave.data()+36,"data",4);put(40,bytes,4);
}
void snapshot(Api& source,Api& native,int sc,int nc){
    std::array<std::uint32_t,24> a,b;a.fill(0x5a5a5a5a);b=a;
    auto get=[&](Api& api,int channel,std::array<std::uint32_t,24>& out){
        std::array<int,4> result{};auto f=[&](unsigned n){return reinterpret_cast<float*>(out.data()+n);};
        result[0]=api.SQUALL_Channel_Get3DPosition(channel,f(1));result[1]=api.SQUALL_Channel_GetVelocity(channel,f(5));
        result[2]=api.SQUALL_Channel_GetConeParameters(channel,f(9),reinterpret_cast<int*>(out.data()+13),reinterpret_cast<int*>(out.data()+15),reinterpret_cast<int*>(out.data()+17));
        result[3]=api.SQUALL_Channel_GetMinMaxDistance(channel,f(19),f(21));return result;
    };
    const auto ar=get(source,sc,a),br=get(native,nc,b);for(unsigned i=0;i<4;++i)same(ar[i],br[i],"3D getter snapshot");
    if(a!=b){for(unsigned i=0;i<a.size();++i)if(a[i]!=b[i])std::cerr<<"word="<<i<<" source="<<std::hex<<a[i]<<" native="<<b[i]<<std::dec<<'\n';throw std::runtime_error("Cached channel 3D words or guards differ");}
    ++snapshots;
}
void vectors(Api& source,Api& native,int sc,int nc){
    std::vector<std::array<std::uint32_t,3>> patterns{
        {0,0,0},{0x80000000,0,0x80000000},{0x3f800000,0x40000000,0xc0400000},
        {0x3f800000,0x40000000,0xc0400000},{1,0x80000001,0x007fffff},
        {0x7f800000,0xff800000,0},{0x7fc00001,0xffc00123,0x3f800000},
        {0x7fc00001,0xffc00123,0x3f800000},{0x7f800001,0xff800123,0x3f800000},
        {0x7f7fffff,0xff7fffff,0},{0,0,0x3f800000},{0,0x3f800000,0}};
    for(unsigned axis=0;axis<3;++axis)for(unsigned payload:{1U,17U,0x1234U,0x3fffffU})for(unsigned mask:{0x7f800000U,0xff800000U,0x7fc00000U,0xffc00000U}) {
        std::array<std::uint32_t,3> words{0,0,0x3f800000};words[axis]=mask|payload;patterns.push_back(words);patterns.push_back(words);
    }
    for(unsigned n=0;n<patterns.size();++n){
        caseId=n;std::array<float,3> value{};std::memcpy(value.data(),patterns[n].data(),12);
        for(unsigned which=0;which<3;++which){
            int a,b;
            if(which==0){a=source.SQUALL_Channel_Set3DPosition(sc,value.data());b=native.SQUALL_Channel_Set3DPosition(nc,value.data());}
            else if(which==1){a=source.SQUALL_Channel_SetVelocity(sc,value.data());b=native.SQUALL_Channel_SetVelocity(nc,value.data());}
            else {a=source.SQUALL_Channel_SetConeParameters(sc,value.data(),360,360,100);b=native.SQUALL_Channel_SetConeParameters(nc,value.data(),360,360,100);}
            std::cerr<<"vector context="<<contextId<<" case="<<n<<" operation="<<which<<" source="<<a<<" native="<<b<<'\n';
            same(a,b,"3D vector setter");++setters;snapshot(source,native,sc,nc);
        }
        same(source.SQUALL_Listener_Update(),native.SQUALL_Listener_Update(),"Deferred listener commit");snapshot(source,native,sc,nc);
    }
}
void scalars(Api& source,Api& native,int sc,int nc){
    const std::vector<std::uint32_t> words{0,0x80000000,1,0x80000001,0x007fffff,0x00800000,0x3c23d70a,0x3f800000,0x40000000,0x4e6e6b28,0x4e6e6b29,0x7f7fffff,0xff7fffff,0x7f800000,0xff800000,0x7fc00001,0xffc00123,0x7f800001,0xff800123};
    for(auto lo:words)for(auto hi:words){
        caseId=distanceCases;float a,b;std::memcpy(&a,&lo,4);std::memcpy(&b,&hi,4);
        same(source.SQUALL_Channel_SetMinMaxDistance(sc,a,b),native.SQUALL_Channel_SetMinMaxDistance(nc,a,b),"Distance raw pair");++setters;++distanceCases;snapshot(source,native,sc,nc);
    }
    std::array<float,3> orientation{0,0,1};
    const std::array<int,8> angles{-1,0,1,359,360,361,INT_MIN,INT_MAX},volumes{-1,0,1,99,100,101,INT_MIN,INT_MAX};
    for(auto inside:angles)for(auto outside:angles)for(auto volume:volumes){
        caseId=coneCases;same(source.SQUALL_Channel_SetConeParameters(sc,orientation.data(),inside,outside,volume),native.SQUALL_Channel_SetConeParameters(nc,orientation.data(),inside,outside,volume),"Cone integer boundaries");++setters;++coneCases;snapshot(source,native,sc,nc);
    }
}
struct Pages {
    unsigned char* data=nullptr;DWORD page=0;
    Pages(){SYSTEM_INFO info{};GetSystemInfo(&info);page=info.dwPageSize;data=static_cast<unsigned char*>(VirtualAlloc(nullptr,page*3,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));require(data!=nullptr,"Guard allocation failed");float input[3]{1,2,3};std::memcpy(data,input,12);DWORD previous{};require(VirtualProtect(data,page,PAGE_READONLY,&previous)!=0,"Read-only protection failed");require(VirtualProtect(data+page*2,page,PAGE_NOACCESS,&previous)!=0,"No-access protection failed");}
    ~Pages(){if(data)VirtualFree(data,0,MEM_RELEASE);}
    float* readonly(){return reinterpret_cast<float*>(data);}
    float* cross(){return reinterpret_cast<float*>(data+page*2-2);}
};
void pointers(Api& source,Api& native,int sc,int nc,Pages& pages){
    for(float* input:{static_cast<float*>(nullptr),reinterpret_cast<float*>(1),pages.readonly(),pages.cross()}){
        same(source.SQUALL_Channel_Set3DPosition(sc,input),native.SQUALL_Channel_Set3DPosition(nc,input),"Position input page");++setters;snapshot(source,native,sc,nc);
        same(source.SQUALL_Channel_SetVelocity(sc,input),native.SQUALL_Channel_SetVelocity(nc,input),"Velocity input page");++setters;snapshot(source,native,sc,nc);
        same(source.SQUALL_Channel_SetConeParameters(sc,input,360,360,100),native.SQUALL_Channel_SetConeParameters(nc,input,360,360,100),"Cone writable-input page");++setters;snapshot(source,native,sc,nc);
    }
    for(unsigned mode=0;mode<11;++mode){
        std::array<std::uint32_t,24> a,b;a.fill(0x5a5a5a5a);b=a;
        float *oa=reinterpret_cast<float*>(a.data()+1),*ob=reinterpret_cast<float*>(b.data()+1);
        if(mode==1)oa=ob=nullptr;if(mode==2)oa=ob=reinterpret_cast<float*>(1);if(mode==3)oa=ob=pages.readonly();if(mode==4)oa=ob=pages.cross();
        same(source.SQUALL_Channel_Get3DPosition(sc,oa),native.SQUALL_Channel_Get3DPosition(nc,ob),"Position output guard");require(a==b,"Position output words differ");++pointerCases;
        same(source.SQUALL_Channel_GetVelocity(sc,oa),native.SQUALL_Channel_GetVelocity(nc,ob),"Velocity output guard");require(a==b,"Velocity output words differ");++pointerCases;
        float *la=reinterpret_cast<float*>(a.data()+9),*lb=reinterpret_cast<float*>(b.data()+9),*ha=reinterpret_cast<float*>(a.data()+13),*hb=reinterpret_cast<float*>(b.data()+13);
        if(mode<4){if(!(mode&1))la=lb=nullptr;if(!(mode&2))ha=hb=nullptr;}
        if(mode==4)la=lb=pages.cross();if(mode==5)ha=hb=pages.readonly();if(mode==6)la=ha,lb=hb;
        same(source.SQUALL_Channel_GetMinMaxDistance(sc,la,ha),native.SQUALL_Channel_GetMinMaxDistance(nc,lb,hb),"Distance output guard");require(a==b,"Distance output words differ");++pointerCases;
        // Original cone getter writes OutsideVolume unconditionally on success.
        // Keep that output writable; a separate-process fault test remains pending.
        int *ia=reinterpret_cast<int*>(a.data()+5),*ib=reinterpret_cast<int*>(b.data()+5),*ea=reinterpret_cast<int*>(a.data()+7),*eb=reinterpret_cast<int*>(b.data()+7);
        if(mode<8){if(!(mode&1))oa=ob=nullptr;if(!(mode&2))ia=ib=nullptr;if(!(mode&4))ea=eb=nullptr;}
        if(mode==8)ia=ib=reinterpret_cast<int*>(pages.readonly());if(mode==9)ea=eb=reinterpret_cast<int*>(pages.cross());
        if(mode==10)ia=ea=reinterpret_cast<int*>(a.data()+1),ib=eb=reinterpret_cast<int*>(b.data()+1);
        same(source.SQUALL_Channel_GetConeParameters(sc,oa,ia,ea,reinterpret_cast<int*>(a.data()+21)),native.SQUALL_Channel_GetConeParameters(nc,ob,ib,eb,reinterpret_cast<int*>(b.data()+21)),"Cone partial output guard");require(a==b,"Cone output words differ");++pointerCases;
    }
}
unsigned sourceSymbol(const Api& api,const std::filesystem::path& path,const char* name){
    std::ifstream map(path.parent_path()/"Squall.map");require(bool(map),"Matching source linker map required");
    auto base=reinterpret_cast<unsigned char*>(api.module);auto pe=reinterpret_cast<IMAGE_NT_HEADERS32*>(base+reinterpret_cast<IMAGE_DOS_HEADER*>(base)->e_lfanew);
    unsigned preferred=0,timestamp=0;std::string line;
    while(std::getline(map,line)){
        if(line.find("Preferred load address is ")!=std::string::npos)preferred=std::stoul(line.substr(line.find_last_of(' ')+1),nullptr,16);
        if(line.find("Timestamp is ")!=std::string::npos)timestamp=std::stoul(line.substr(line.find("Timestamp is ")+13),nullptr,16);
        if(line.find(name)!=std::string::npos){std::istringstream fields(line);std::string segment,symbol,address;fields>>segment>>symbol>>address;const auto flat=std::stoul(address,nullptr,16);require(preferred&&flat>=preferred&&flat-preferred<pe->OptionalHeader.SizeOfImage&&timestamp==pe->FileHeader.TimeDateStamp,"Source map does not match loaded DLL");return unsigned(flat-preferred);}
    }
    throw std::runtime_error(name);
}
struct Mock3D {void** table;DS3DBUFFER forwarded{};DWORD deferred=0;unsigned calls=0;HRESULT result=S_OK;};
HRESULT WINAPI recordParameters(void* object,const DS3DBUFFER* parameters,DWORD deferred){
    auto& mock=*static_cast<Mock3D*>(object);std::memcpy(&mock.forwarded,parameters,sizeof(*parameters));mock.deferred=deferred;++mock.calls;return mock.result;
}
void fixtureCompare(Api& source,Api& native,const std::filesystem::path& candidate){
    static_assert(sizeof(SSample)==300&&offsetof(SSample,DS3D_Buffer)==12&&offsetof(SSample,Param)==84,"Original method byte layout");
    using Set=int(__thiscall*)(SSample*,D3DVECTOR*,D3DVECTOR*,D3DVECTOR*,unsigned,unsigned,long,float,float,DWORD);
    using Get=int(__thiscall*)(SSample*,D3DVECTOR*,D3DVECTOR*,D3DVECTOR*,int*,int*,long*,float*,float*);
    auto sourceBase=reinterpret_cast<unsigned char*>(source.module),nativeBase=reinterpret_cast<unsigned char*>(native.module);
    auto ownSet=reinterpret_cast<Set>(sourceBase+sourceSymbol(source,candidate,"?Set3DParameters@SSample@@"));auto originalSet=reinterpret_cast<Set>(nativeBase+0x1a70);
    auto ownGet=reinterpret_cast<Get>(sourceBase+sourceSymbol(source,candidate,"?Get3DParameters@SSample@@"));auto originalGet=reinterpret_cast<Get>(nativeBase+0x1c10);
    const std::array<std::uint32_t,16> patterns{0,0x80000000,1,0x80000001,0x007fffff,0x3f800000,0xbf800000,0x7f7fffff,0xff7fffff,0x7f800000,0xff800000,0x7fc00001,0xffc00123,0x7f800001,0xff800123,0x3f000001};
    std::array<void*,13> table{};table[12]=reinterpret_cast<void*>(&recordParameters);Mock3D mock{};mock.table=table.data();
    unsigned setCases=0,getCases=0;std::uint32_t random=0x57a291e3;
    auto next=[&](){random^=random<<13;random^=random>>17;random^=random<<5;return random;};
    for(unsigned seed=0;seed<patterns.size();++seed)for(unsigned presence=0;presence<8;++presence)for(unsigned variant=0;variant<16;++variant){
        SSample initial{};auto bytes=reinterpret_cast<unsigned char*>(&initial);for(unsigned n=0;n<sizeof(initial);++n)bytes[n]=static_cast<unsigned char>(next());
        initial.DS3D_Buffer=(variant&1) ? nullptr:reinterpret_cast<LPDIRECTSOUND3DBUFFER>(&mock);
        auto cached=reinterpret_cast<unsigned char*>(&initial.Param.ds3d);
        for(unsigned n=0;n<16;++n){auto word=patterns[(seed+n)%patterns.size()];std::memcpy(cached+n*4,&word,4);}
        std::array<std::uint32_t,9> input{};for(unsigned n=0;n<input.size();++n)input[n]=patterns[(seed+variant+n)%patterns.size()];
        SSample object;std::array<unsigned char,300> afterSource{};Mock3D sourceMock{};int sourceResult=0;
        for(unsigned pass=0;pass<2;++pass){
            object=initial;mock.forwarded={};mock.deferred=0;mock.calls=0;mock.result=(variant&2) ? E_FAIL:S_OK;
            auto field=reinterpret_cast<unsigned char*>(&object.Param.ds3d);
            std::array<D3DVECTOR*,3> pointers{};
            for(unsigned n=0;n<3;++n)if(presence&(1U<<n))pointers[n]=reinterpret_cast<D3DVECTOR*>((variant&4) ? field+4*(n+1):reinterpret_cast<unsigned char*>(input.data()+n*3));
            float min,max;auto lo=patterns[seed],hi=patterns[(seed+variant)%patterns.size()];std::memcpy(&min,&lo,4);std::memcpy(&max,&hi,4);
            const auto set=pass ? originalSet:ownSet;const auto result=set(&object,pointers[0],pointers[1],pointers[2],variant&8 ? 0:seed*37,variant&8 ? seed*61:0,variant&8 ? -10000:0,min,max,0x80000000U|variant);
            if(!pass){sourceResult=result;std::memcpy(afterSource.data(),&object,sizeof(object));sourceMock=mock;}
            else {
                require(sourceResult==result,"Fixture Set3DParameters result differs");require(std::memcmp(afterSource.data(),&object,sizeof(object))==0,"Fixture Set3DParameters cached words differ");
                require(sourceMock.calls==mock.calls&&sourceMock.deferred==mock.deferred&&std::memcmp(&sourceMock.forwarded,&mock.forwarded,sizeof(mock.forwarded))==0,"Fixture forwarded COM call differs");
            }
        }
        ++setCases;
    }
    for(unsigned seed=0;seed<patterns.size();++seed)for(unsigned presence=0;presence<256;++presence)for(unsigned variant=0;variant<4;++variant){
        SSample initial{};initial.DS3D_Buffer=(variant&1) ? nullptr:reinterpret_cast<LPDIRECTSOUND3DBUFFER>(&mock);
        auto cached=reinterpret_cast<unsigned char*>(&initial.Param.ds3d);for(unsigned n=0;n<16;++n){auto word=patterns[(seed+n)%patterns.size()];std::memcpy(cached+n*4,&word,4);}
        SSample object;std::array<std::uint32_t,40> output,sourceOutput{};std::array<unsigned char,300> sourceObject{};int sourceResult=0;
        for(unsigned pass=0;pass<2;++pass){
            object=initial;output.fill(0x5a5a5a5a);auto field=reinterpret_cast<unsigned char*>(&object.Param.ds3d);std::array<void*,8> target{};
            for(unsigned n=0;n<8;++n)if(presence&(1U<<n))target[n]=(variant&2) ? field+4*(n+1):static_cast<void*>(output.data()+1+n*4);
            const auto get=pass ? originalGet:ownGet;const auto result=get(&object,static_cast<D3DVECTOR*>(target[0]),static_cast<D3DVECTOR*>(target[1]),static_cast<D3DVECTOR*>(target[2]),static_cast<int*>(target[3]),static_cast<int*>(target[4]),static_cast<long*>(target[5]),static_cast<float*>(target[6]),static_cast<float*>(target[7]));
            if(!pass){sourceResult=result;sourceOutput=output;std::memcpy(sourceObject.data(),&object,sizeof(object));}
            else {require(sourceResult==result&&sourceOutput==output,"Fixture Get3DParameters result or output words differ");require(std::memcmp(sourceObject.data(),&object,sizeof(object))==0,"Fixture Get3DParameters alias writes differ");}
        }
        ++getCases;
    }
    std::cout<<"{\"setter_fixtures\":"<<setCases<<",\"getter_fixtures\":"<<getCases<<",\"same_object_address\":true,\"com_boundary_recorded\":true,\"actual_directsound\":false}\n";
}
}
int wmain(int argc,wchar_t** argv){
    if(argc<3||argc>5)return 2;HWND window=nullptr;bool pc24=false,fixtures=false;
    for(int n=3;n<argc;++n){if(std::wstring(argv[n])==L"pc24")pc24=true;else if(std::wstring(argv[n])==L"fixture")fixtures=true;else return 2;}
    try {
        Api native(argv[1]),source(argv[2]);_control87(pc24 ? _PC_24:_PC_53,_MCW_PC);
        if(fixtures){fixtureCompare(source,native,std::filesystem::absolute(argv[2]));return 0;}
        createWave();Pages pages;
        const int devices=native.SQUALL_GetNumDevice();same(source.SQUALL_GetNumDevice(),devices,"Device count");require(devices>0,"Actual audio device required");
        window=CreateWindowExW(0,L"STATIC",L"Squall actual positioned channel vectors",0,0,0,64,64,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);require(window!=nullptr,"CreateWindow failed");
        for(unsigned deferred=0;deferred<2;++deferred){
            contextId=deferred;squall_parameters_t params{};params.Window=window;params.SampleRate=44100;params.BitPerSample=16;params.Channels=16;params.UseAlg=1;params.BufferSize=200;params.ListenerMode=deferred;params.DistanceFactor=1;params.RolloffFactor=.001f;params.DopplerFactor=1;
            const int initialized=native.SQUALL_Init(&params);same(source.SQUALL_Init(&params),initialized,"Init");require(initialized>=0,"Real DirectSound init failed");
            same(source.SQUALL_SetFileCallbacks(open,seek,read,close),native.SQUALL_SetFileCallbacks(open,seek,read,close),"PCM fixture callbacks");
            char filename[]="channel-vectors-mono.wav";const int ns=native.SQUALL_Sample_LoadFile(filename,1,nullptr),ss=source.SQUALL_Sample_LoadFile(filename,1,nullptr);same(ss,ns,"Mono PCM sample load");require(ns>0,"Mono PCM sample failed");
            std::array<float,3> zero{};const int nc=native.SQUALL_Sample_Play3DEx(ns,1,17,0,zero.data(),zero.data(),7,0,0,1,1000000),sc=source.SQUALL_Sample_Play3DEx(ss,1,17,0,zero.data(),zero.data(),7,0,0,1,1000000);
            std::cerr<<"Prepared positioned channel source="<<sc<<" native="<<nc<<'\n';
            same(sc,nc,"Actual prepared positioned channel");require(nc>=0,"Positioned DirectSound channel failed");snapshot(source,native,sc,nc);vectors(source,native,sc,nc);scalars(source,native,sc,nc);pointers(source,native,sc,nc,pages);
            source.SQUALL_Free();native.SQUALL_Free();
        }
        require(opens==closes,"Diagnostic PCM callback handle leaked");require((_control87(0,0)&_MCW_PC)==(pc24 ? _PC_24:_PC_53),"x87 precision changed");
        DestroyWindow(window);window=nullptr;
        std::cout<<"{\"api_comparisons\":"<<comparisons<<",\"snapshots\":"<<snapshots<<",\"setter_calls\":"<<setters<<",\"distance_pairs\":"<<distanceCases<<",\"cone_integer_cases\":"<<coneCases<<",\"output_pointer_cases\":"<<pointerCases<<",\"actual_directsound\":true,\"prepared_positioned_channels\":2,\"export_lookups\":194,\"waveform_compared\":false}\n";return 0;
    }catch(const std::exception& e){if(window)DestroyWindow(window);std::cerr<<e.what()<<'\n';return 1;}
}
