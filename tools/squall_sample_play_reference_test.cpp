// Real exported sample/group playback; no private implementation substitution.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "Squall.h"
#include <array>
#include <vector>
#include <filesystem>
#include <float.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
namespace {
void need(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
struct Api {
#define SQUALL_ENTRY(name) decltype(&name) name{};
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    HMODULE module{};
    explicit Api(const wchar_t* path){module=LoadLibraryW(std::filesystem::absolute(path).c_str());need(module!=nullptr,"Load DLL");
#define SQUALL_ENTRY(name) name=reinterpret_cast<decltype(name)>(GetProcAddress(module,#name));need(name!=nullptr,#name);
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    }
    ~Api(){if(module){SQUALL_Free();SQUALL_Sample_UnloadAll();FreeLibrary(module);}}
};
unsigned results{},snapshots{},playCases{},retirementPolls{},pauseCases{},emptyCases{},methodIndex{},caseIndex{};
void same(int a,int b,const char* label){++results;if(a!=b){std::cerr<<label<<" method="<<methodIndex<<" case="<<caseIndex<<" source="<<a<<" native="<<b<<'\n';throw std::runtime_error("Sample playback result differs");}}
struct Pair {int source,native;};
Pair samples[3];const int groups[3]={17,-7,0};
std::array<float,3> position{1.25f,-2.5f,3.75f},velocity{-4.5f,5.25f,6.75f};
int play(Api& api,unsigned method,int sample,int group,int loop,int start){
    switch(method){
    case 0:return api.SQUALL_Sample_Play(sample,loop,group,start);
    case 1:return api.SQUALL_Sample_PlayEx(sample,loop,group,start,19,1,0,57);
    case 2:return api.SQUALL_Sample_Play3D(sample,loop,group,start,position.data(),velocity.data());
    case 3:return api.SQUALL_Sample_Play3DEx(sample,loop,group,start,position.data(),velocity.data(),19,1,0,3,1000);
    case 4:return api.SQUALL_SampleGroup_Play(sample,loop,group,start);
    case 5:return api.SQUALL_SampleGroup_PlayEx(sample,loop,group,start,19,1,0,57);
    case 6:return api.SQUALL_SampleGroup_Play3D(sample,loop,group,start,position.data(),velocity.data());
    default:return api.SQUALL_SampleGroup_Play3DEx(sample,loop,group,start,position.data(),velocity.data(),19,1,0,3,1000);
    }
}
Pair create(Api& s,Api& n,unsigned method,unsigned sample,int loop,int start){
    methodIndex=method;const auto b=play(n,method,method<4?samples[sample].native:groups[sample],-91,loop,start);
    const auto a=play(s,method,method<4?samples[sample].source:groups[sample],-91,loop,start);
    same(a,b,"Actual sample/group create");need(a>=0,"Actual sample/group channel failed");++playCases;return {a,b};
}
void snapshot(Api& s,Api& n,Pair c){
#define SCALAR(name) same(s.SQUALL_Channel_##name(c.source),n.SQUALL_Channel_##name(c.native),#name)
    SCALAR(Status);SCALAR(GetVolume);SCALAR(GetFrequency);SCALAR(GetPriority);SCALAR(GetLoop);SCALAR(GetPan);SCALAR(GetLength);SCALAR(GetLengthMs);
#undef SCALAR
    std::array<std::uint32_t,15>a,b;a.fill(0x5a5a5a5a);b=a;
    auto out=[&](Api& api,int channel,auto& words){std::array<int,3> r{};
        r[0]=api.SQUALL_Channel_Get3DPosition(channel,reinterpret_cast<float*>(words.data()+1));
        r[1]=api.SQUALL_Channel_GetVelocity(channel,reinterpret_cast<float*>(words.data()+5));
        r[2]=api.SQUALL_Channel_GetMinMaxDistance(channel,reinterpret_cast<float*>(words.data()+10),reinterpret_cast<float*>(words.data()+12));return r;};
    const auto ar=out(s,c.source,a),br=out(n,c.native,b);for(unsigned i=0;i<3;++i)same(ar[i],br[i],"Guarded cached3D getter");
    need(a==b,"Cached3D words/guards differ");++snapshots;
}
void retired(Api& s,Api& n,const std::vector<Pair>& channels){
    const auto deadline=GetTickCount64()+2000;
    for(;;){bool done=true;for(auto c:channels){const auto a=s.SQUALL_Channel_Status(c.source),b=n.SQUALL_Channel_Status(c.native);
        need((a==0||a==-24)&&(b==0||b==-24),"Unexpected worker retirement status");done&=a==0&&b==0;}
        if(done)break;need(GetTickCount64()<deadline,"Workers did not retire");++retirementPolls;Sleep(1);}
    for(auto c:channels)same(s.SQUALL_Channel_Status(c.source),n.SQUALL_Channel_Status(c.native),"Retired channel status");
}
void stop(Api& s,Api& n,Pair c){same(s.SQUALL_Channel_Stop(c.source),n.SQUALL_Channel_Stop(c.native),"Stop case channel");retired(s,n,{c});}
void empty(Api& s,Api& n){
    for(int id:{0,-1,0x7fffffff,0x10000}){
        for(int pause:{0,1,-1,2}){same(s.SQUALL_Sample_Pause(id,pause),n.SQUALL_Sample_Pause(id,pause),"Empty sample pause");++emptyCases;}
        same(s.SQUALL_Sample_Stop(id),n.SQUALL_Sample_Stop(id),"Empty sample stop");++emptyCases;
        same(s.SQUALL_Sample_PlayEx(id,1,7,0,7,1,0,50),n.SQUALL_Sample_PlayEx(id,1,7,0,7,1,0,50),"Invalid sample PlayEx");++emptyCases;
        same(s.SQUALL_SampleGroup_PlayEx(id,1,7,0,7,1,0,50),n.SQUALL_SampleGroup_PlayEx(id,1,7,0,7,1,0,50),"Missing sample group PlayEx");++emptyCases;
    }
}
void matrix(Api& s,Api& n){
    for(unsigned method=0;method<8;++method)for(unsigned sample=0;sample<3;++sample)for(int loop:{0,1,-1,2})for(int start:{0,1,-1,2}){
        ++caseIndex;auto c=create(s,n,method,sample,loop,start);
        // A started nonlooping one-second buffer is paused immediately so that
        // scheduler differences cannot masquerade as cached parameter changes.
        if(start){same(s.SQUALL_Channel_Pause(c.source,1),n.SQUALL_Channel_Pause(c.native,1),"Pause freshly started channel");}
        snapshot(s,n,c);stop(s,n,c);
    }
}
void sampleControls(Api& s,Api& n){
    auto a=create(s,n,0,0,1,0),b=create(s,n,2,0,1,0),other=create(s,n,0,2,1,0);
    // Same source file, different sample ID: pause/stop must match the ID.
    for(int pause:{0,1,-1,2}){
        same(s.SQUALL_Sample_Pause(samples[0].source,pause),n.SQUALL_Sample_Pause(samples[0].native,pause),"Prepared sample pause");
        for(auto c:{a,b,other}){snapshot(s,n,c);need(n.SQUALL_Channel_Status(c.native)==3,"Sample pause started prepared channel");}++pauseCases;
    }
    for(auto c:{a,b,other})same(s.SQUALL_Channel_Start(c.source),n.SQUALL_Channel_Start(c.native),"Start sample-control buffer");
    for(int pause:{1,0,-1,0,2,0,1}){
        same(s.SQUALL_Sample_Pause(samples[0].source,pause),n.SQUALL_Sample_Pause(samples[0].native,pause),"Live sample pause");
        for(auto c:{a,b}){snapshot(s,n,c);need(n.SQUALL_Channel_Status(c.native)==(pause?2:1),"Sample pause boolean mismatch");}
        snapshot(s,n,other);need(n.SQUALL_Channel_Status(other.native)==1,"Pause touched duplicate-file different sample ID");++pauseCases;
    }
    same(s.SQUALL_Sample_Stop(samples[0].source),n.SQUALL_Sample_Stop(samples[0].native),"Stop same-sample2D/3D");retired(s,n,{a,b});
    snapshot(s,n,other);need(n.SQUALL_Channel_Status(other.native)==1,"Sample Stop touched duplicate-file different sample ID");
    same(s.SQUALL_Sample_Stop(samples[2].source),n.SQUALL_Sample_Stop(samples[2].native),"Stop other sample");retired(s,n,{other});
}
}
int wmain(int argc,wchar_t** argv){
    if(argc<3||argc>4)return 2;HWND window{};
    try{
        Api native(argv[1]),source(argv[2]);const auto precision=argc==4?_PC_24:_PC_53;_control87(precision,_MCW_PC);empty(source,native);
        for(unsigned i=0;i<3;++i){squall_sample_default_t def{};def.SampleGroupID=groups[i];def.Priority=7+int(i)*2;def.Frequency=16000+int(i)*4000;def.Volume=1;def.Pan=39+int(i)*10;def.MinDist=2;def.MaxDist=100;
            char file1[]="sample.wav",file2[]="sample2.wav";auto* file=i==1?file2:file1;
            samples[i].native=native.SQUALL_Sample_LoadFile(file,1,&def);samples[i].source=source.SQUALL_Sample_LoadFile(file,1,&def);same(samples[i].source,samples[i].native,"Load singleton-group sample");need(samples[i].native>0,"Load PCM file failed");}
        window=CreateWindowExW(0,L"STATIC",L"Squall sample playback",0,0,0,64,64,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);need(window!=nullptr,"Create audio window");
        squall_parameters_t p{};p.Window=window;p.SampleRate=44100;p.BitPerSample=16;p.Channels=8;p.UseAlg=1;p.BufferSize=200;p.DistanceFactor=1;p.RolloffFactor=.001f;p.DopplerFactor=1;
        const auto initialized=native.SQUALL_Init(&p);same(source.SQUALL_Init(&p),initialized,"Actual Init");need(initialized>=0,"Actual DirectSound Init failed");
        matrix(source,native);sampleControls(source,native);
        source.SQUALL_Free();native.SQUALL_Free();empty(source,native);
        need((_control87(0,0)&_MCW_PC)==precision,"Precision changed");DestroyWindow(window);window=nullptr;
        std::cout<<"{\"result_comparisons\":"<<results<<",\"guarded_channel_snapshots\":"<<snapshots<<",\"actual_play_cases\":"<<playCases<<",\"sample_pause_cases\":"<<pauseCases<<",\"empty_lifecycle_cases\":"<<emptyCases<<",\"retirement_polls\":"<<retirementPolls<<",\"export_lookups\":194,\"actual_directsound\":true,\"sample_groups\":[17,-7,0],\"multiple_members_per_group_verified\":false,\"audible_output_compared\":false}\n";return 0;
    }catch(const std::exception& error){if(window)DestroyWindow(window);std::cerr<<error.what()<<'\n';return 1;}
}
