// Diagnostic only: each real DLL owns its backend, reader, decoder and samples.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include "Squall.h"
#include "SoundEngine/SquallApi.h"
#include <array>
#include <vector>
#include <map>
#include <filesystem>
#include <float.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
namespace {
void need(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
struct FpuObservation {const char* label;unsigned pc,number;};
FpuObservation fpuObservations[128]{};unsigned fpuCount{};
void* pointerPages[3]{};unsigned requestedPrecision{},pointerFpuChecks{};
void observePrecision(const char* label,unsigned number=0){need(fpuCount<128,"FPU observation bound");fpuObservations[fpuCount++]={label,_control87(0,0)&_MCW_PC,number};}
using Slots=std::array<std::uintptr_t,4>;
using Pcm=unsigned(__thiscall*)(void*,void*,unsigned,unsigned,bool);
struct Api {
#define SQUALL_ENTRY(name) decltype(&name) name{};
#include "squall_exports.inc"
#undef SQUALL_ENTRY
 HMODULE module{};std::uintptr_t* slots{};void*(__cdecl* file)(int){};Pcm mono{};Slots defaults{};
 Api(const wchar_t* path,unsigned apiRva,unsigned fileRva,unsigned monoRva){
  module=LoadLibraryW(std::filesystem::absolute(path).c_str());need(module!=nullptr,"Load real DLL");
#define SQUALL_ENTRY(name) name=reinterpret_cast<decltype(name)>(GetProcAddress(module,#name));need(name!=nullptr,#name);
#include "squall_exports.inc"
#undef SQUALL_ENTRY
  const auto base=reinterpret_cast<std::uintptr_t>(module);
  slots=reinterpret_cast<std::uintptr_t*>(base+apiRva+offsetof(squall_api_t,_open));
  file=reinterpret_cast<decltype(file)>(base+fileRva);mono=reinterpret_cast<Pcm>(base+monoRva);
  SQUALL_GetNumDevice();for(unsigned i=0;i<4;++i){defaults[i]=slots[i];need(defaults[i]&&!IsBadCodePtr(reinterpret_cast<FARPROC>(defaults[i])),"Actual default file callback");}
 }
 int set(const Slots& p){return SQUALL_SetFileCallbacks(reinterpret_cast<squall_callback_open_t>(p[0]),reinterpret_cast<squall_callback_seek_t>(p[2]),reinterpret_cast<squall_callback_read_t>(p[1]),reinterpret_cast<squall_callback_close_t>(p[3]));}
 ~Api(){if(module){set(defaults);SQUALL_Sample_UnloadAll();SQUALL_Free();FreeLibrary(module);}}
};
static_assert(sizeof(void*)==4);
static_assert(offsetof(squall_api_t,_open)==0x968);
Api* apis[2]{};
struct Event {unsigned op,generation,serial;int a,b,result;std::uint64_t bytes;bool operator==(const Event& e)const{return op==e.op&&generation==e.generation&&serial==e.serial&&a==e.a&&b==e.b&&result==e.result&&bytes==e.bytes;}};
struct Tracker {std::map<unsigned,unsigned> handles;std::vector<Event> events;unsigned next{};unsigned fail{};} trackers[2];
unsigned results{},pointerCases{},slotChecks{},snapshots{},reads{},loaded{},failed{},phases{};std::uint64_t pcmBytes{};
std::uint64_t hash(const void* p,unsigned n){auto* b=static_cast<const unsigned char*>(p);std::uint64_t h=14695981039346656037ULL;for(unsigned i=0;i<n;++i){h^=b[i];h*=1099511628211ULL;}return h;}
template<unsigned D,unsigned G> unsigned __cdecl openFile(const char* name){
 auto& t=trackers[D];auto fn=reinterpret_cast<squall_callback_open_t>(apis[D]->defaults[0]);const unsigned handle=t.fail==1?~0U:fn(name);unsigned serial=0;
 if(handle!=~0U){serial=++t.next;need(t.handles.emplace(handle,serial).second,"Duplicate owned file handle");}
 t.events.push_back({0,G,serial,0,0,handle==~0U?-1:1,hash(name,unsigned(std::strlen(name)))});return handle;
}
template<unsigned D,unsigned G> int __cdecl readFile(unsigned handle,void* buffer,int size){
 auto& t=trackers[D];need(t.handles.count(handle)==1,"Read unknown file handle");auto fn=reinterpret_cast<squall_callback_read_t>(apis[D]->defaults[1]);const int result=fn(handle,buffer,t.fail==3?0:size);
 t.events.push_back({1,G,t.handles.at(handle),size,0,result,result>0?hash(buffer,unsigned(result)):0});return result;
}
template<unsigned D,unsigned G> int __cdecl seekFile(unsigned handle,int position,int mode){
 auto& t=trackers[D];need(t.handles.count(handle)==1,"Seek unknown file handle");auto fn=reinterpret_cast<squall_callback_seek_t>(apis[D]->defaults[2]);const int result=t.fail==2?-1:fn(handle,position,mode);
 t.events.push_back({2,G,t.handles.at(handle),position,mode,result,0});return result;
}
template<unsigned D,unsigned G> void __cdecl closeFile(unsigned handle){
 auto& t=trackers[D];need(t.handles.count(handle)==1,"Close unknown file handle");const auto serial=t.handles.at(handle);auto fn=reinterpret_cast<squall_callback_close_t>(apis[D]->defaults[3]);fn(handle);t.handles.erase(handle);t.events.push_back({3,G,serial,0,0,0,0});
}
template<unsigned D,unsigned G> Slots callbacks(){return {reinterpret_cast<std::uintptr_t>(openFile<D,G>),reinterpret_cast<std::uintptr_t>(readFile<D,G>),reinterpret_cast<std::uintptr_t>(seekFile<D,G>),reinterpret_cast<std::uintptr_t>(closeFile<D,G>)};}
void same(int a,int b,const char* what){++results;if(a!=b){std::cerr<<what<<" source="<<a<<" native="<<b<<'\n';throw std::runtime_error("File callback result differs");}}
void events(){need(trackers[0].events==trackers[1].events,"Actual file callback event order/arguments/data differ");need(trackers[0].handles.size()==trackers[1].handles.size(),"Open handle ownership differs");++snapshots;}
Slots choose(unsigned d,unsigned mask){const auto a=d?callbacks<1,0>():callbacks<0,0>(),b=d?callbacks<1,1>():callbacks<0,1>();Slots out{};for(unsigned i=0;i<4;++i)out[i]=(mask&(1U<<i))?b[i]:a[i];return out;}
void install(Api& s,Api& n,unsigned mask){same(s.set(choose(1,mask)),n.set(choose(0,mask)),"Set real file callbacks");events();}
void validity(Api& s,Api& n){
 auto* page=pointerPages[0];auto* blocked=pointerPages[1];auto* executable=pointerPages[2];need(page&&blocked&&executable,"Prepared pointer validity pages");
 const auto a=callbacks<0,0>(),b=callbacks<0,1>(),c=callbacks<1,0>(),d=callbacks<1,1>();
 for(unsigned bits=0;bits<4096;++bits){Slots left{},right{};unsigned indices=bits;
  for(unsigned i=0;i<4;++i){const unsigned k=indices&7;indices>>=3;const std::array<std::uintptr_t,8> common={0,0,0,1,~std::uintptr_t(0),reinterpret_cast<std::uintptr_t>(page),reinterpret_cast<std::uintptr_t>(blocked),reinterpret_cast<std::uintptr_t>(executable)};left[i]=k==1?a[i]:k==2?b[i]:common[k];right[i]=k==1?c[i]:k==2?d[i]:common[k];}
  const auto pcBefore=_control87(0,0)&_MCW_PC;
  const auto nr=n.set(left);const auto pcNative=_control87(0,0)&_MCW_PC;
  const auto sr=s.set(right);const auto pcSource=_control87(0,0)&_MCW_PC;same(sr,nr,"Callback pointer combination");
  bool invalid=false;for(auto p:left)invalid=invalid||bool(IsBadCodePtr(reinterpret_cast<FARPROC>(p)));
  const auto pcLocal=_control87(0,0)&_MCW_PC;
  if(pcBefore!=pcNative||pcNative!=pcSource||pcSource!=pcLocal){observePrecision("pointer-before",bits);fpuObservations[fpuCount-1].pc=pcBefore;observePrecision("pointer-native",bits);fpuObservations[fpuCount-1].pc=pcNative;observePrecision("pointer-source",bits);fpuObservations[fpuCount-1].pc=pcSource;observePrecision("pointer-local",bits);}
  need(pcBefore==requestedPrecision&&pcNative==requestedPrecision&&pcSource==requestedPrecision&&pcLocal==requestedPrecision,"Actual file callback pointer test changed FPU precision");++pointerFpuChecks;
  need(nr==(invalid?-4:1),"Native callback pointer validation result");
  for(unsigned i=0;i<4;++i){need(n.slots[i]==left[i]&&s.slots[i]==right[i],"Invalid callback set must still store exact pointers");++slotChecks;}
  ++pointerCases;
 }
 observePrecision("before-default-reset");s.set(s.defaults);observePrecision("after-source-default-reset");n.set(n.defaults);observePrecision("after-native-default-reset");
}
std::array<int,2> load(Api& s,Api& n,const char* name,int memory,bool success=true){
 const int native=n.SQUALL_Sample_LoadFile(const_cast<char*>(name),memory,nullptr),source=s.SQUALL_Sample_LoadFile(const_cast<char*>(name),memory,nullptr);same(source,native,"Actual reader load");events();
 if(success){need(native>0,"Valid reader failed");++loaded;same(s.SQUALL_Sample_GetFileLength(source),n.SQUALL_Sample_GetFileLength(native),"Actual sample frames");need(n.SQUALL_Sample_GetFileLength(native)==64,"Expected own WAV frames");}else{need(native<0,"Controlled callback failure not reported");++failed;}
 return {source,native};
}
void decode(Api& s,Api& n,const std::array<int,2>& id){
 for(unsigned start:{0U,7U,63U,64U})for(unsigned count:{0U,1U,7U,64U,65U})for(bool loop:{false,true}){
  std::vector<unsigned char> a(count*2+32,0x5a),b=a;auto* source=s.file(id[0]);auto* native=n.file(id[1]);need(source&&native,"Actual sample decoder");
  same(s.mono(source,a.data()+16,start,count,loop),n.mono(native,b.data()+16,start,count,loop),"Actual callback-backed PCM read");need(a==b,"Callback PCM or full destination guard differs");++reads;pcmBytes+=count*2;events();
 }
}
void unload(Api& s,Api& n,const std::array<int,2>& id){s.SQUALL_Sample_Unload(id[0]);n.SQUALL_Sample_Unload(id[1]);events();}
void cases(Api& s,Api& n){
 ++phases;observePrecision("phase-start");validity(s,n);observePrecision("after-pointer-validity");
 for(int memory:{0,1,-1,2})for(unsigned mask=0;mask<16;++mask){
  install(s,n,mask);auto first=load(s,n,"sample.wav",memory);
  install(s,n,mask^15);auto second=load(s,n,"sample2.wav",memory);
  // The duplicate keeps the first reader and its callback snapshot.
  auto duplicate=load(s,n,"sample.wav",memory);unload(s,n,first);decode(s,n,duplicate);decode(s,n,second);unload(s,n,second);unload(s,n,duplicate);
  need(trackers[0].handles.empty()&&trackers[1].handles.empty(),"File callback leaked handle");
 }
 observePrecision("after-readers");
 for(unsigned failure:{1U,2U,3U}){install(s,n,0);for(auto& t:trackers)t.fail=failure;load(s,n,"sample.wav",1,false);for(auto& t:trackers)t.fail=0;need(trackers[0].handles.empty()&&trackers[1].handles.empty(),"Failed reader leaked handle");auto recovered=load(s,n,"sample.wav",1);unload(s,n,recovered);}
 install(s,n,0);auto first=load(s,n,"sample.wav",0);install(s,n,15);auto second=load(s,n,"sample2.wav",0);s.SQUALL_Sample_UnloadAll();n.SQUALL_Sample_UnloadAll();events();need(trackers[0].handles.empty()&&trackers[1].handles.empty(),"UnloadAll callback ownership");
 s.set(s.defaults);n.set(n.defaults);observePrecision("phase-end");
}
}
int wmain(int argc,wchar_t** argv){
 if(argc<6||argc>7)return 2;HWND window{};
 try{
  Api native(argv[1],0x75028,0x8300,0x2e020),source(argv[2],std::stoul(argv[3],nullptr,16),std::stoul(argv[4],nullptr,16),std::stoul(argv[5],nullptr,16));apis[0]=&native;apis[1]=&source;
  // Fixture setup can initialize host instrumentation and change the CW.
  // Prepare all pages before selecting the precision being measured. No CW
  // reset occurs between any actual native/source calls below.
  pointerPages[0]=VirtualAlloc(nullptr,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);pointerPages[1]=VirtualAlloc(nullptr,4096,MEM_COMMIT|MEM_RESERVE,PAGE_NOACCESS);pointerPages[2]=VirtualAlloc(nullptr,4096,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READ);need(pointerPages[0]&&pointerPages[1]&&pointerPages[2],"Prepare validity pages");
  const auto precision=argc==7?_PC_24:_PC_53;requestedPrecision=precision;_control87(precision,_MCW_PC);cases(source,native);
  window=CreateWindowExW(0,L"STATIC",L"Squall file callbacks",0,0,0,64,64,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);need(window!=nullptr,"Create DirectSound window");
  squall_parameters_t p{};p.Window=window;p.SampleRate=44100;p.BitPerSample=16;p.Channels=4;p.UseAlg=3;p.BufferSize=200;p.DistanceFactor=1;p.RolloffFactor=.001f;p.DopplerFactor=1;
  observePrecision("before-native-init");const int initialized=native.SQUALL_Init(&p);observePrecision("after-native-init");same(source.SQUALL_Init(&p),initialized,"Actual DirectSound Init");observePrecision("after-source-init");need(initialized>=0,"DirectSound Init failed");
  cases(source,native);source.SQUALL_Free();observePrecision("after-source-free");native.SQUALL_Free();observePrecision("after-native-free");cases(source,native);
  if((_control87(0,0)&_MCW_PC)!=precision)for(unsigned i=0;i<fpuCount;++i)std::cerr<<fpuObservations[i].label<<" pc="<<fpuObservations[i].pc<<" case="<<fpuObservations[i].number<<'\n';
  need((_control87(0,0)&_MCW_PC)==precision,"Precision changed");DestroyWindow(window);window=nullptr;for(auto& page:pointerPages){VirtualFree(page,0,MEM_RELEASE);page=nullptr;}
  std::cout<<"{\"result_comparisons\":"<<results<<",\"pointer_combinations\":"<<pointerCases<<",\"exact_pointer_stores\":"<<slotChecks<<",\"callback_event_snapshots\":"<<snapshots<<",\"actual_pcm_reads\":"<<reads<<",\"guarded_pcm_bytes\":"<<pcmBytes<<",\"loaded_samples_per_dll\":"<<loaded<<",\"forced_failures_per_dll\":"<<failed<<",\"lifecycle_phases\":"<<phases<<",\"callback_events_per_dll\":"<<trackers[0].events.size()<<",\"callback_events\":[";
  for(unsigned i=0;i<trackers[0].events.size();++i){const auto& e=trackers[0].events[i];if(i)std::cout<<',';std::cout<<'['<<e.op<<','<<e.generation<<','<<e.serial<<','<<e.a<<','<<e.b<<','<<e.result<<','<<e.bytes<<']';}
  std::cout<<"],\"pointer_fpu_checks\":"<<pointerFpuChecks<<",\"fpu_observations\":[";for(unsigned i=0;i<fpuCount;++i){if(i)std::cout<<',';std::cout<<"{\"stage\":\""<<fpuObservations[i].label<<"\",\"precision\":"<<fpuObservations[i].pc<<'}';}std::cout<<"]}\n";return 0;
 }catch(const std::exception& e){if(window)DestroyWindow(window);for(auto& page:pointerPages)if(page){VirtualFree(page,0,MEM_RELEASE);page=nullptr;}std::cerr<<e.what()<<'\n';return 1;}
}
