// Actual original cLogic modifier binding/lifetime/callback bodies. Only the
// Input/Settings/Timer/Graphics/Resource service boundaries are diagnostic adapters.
#include "LogicShortcuts.h"
#include "GraphicsScreenshot.h"
#include "InputBindings.h"
#include "GameTimer.h"
#include "Settings.h"
#include "FileIndex.h"
#include "StringConversion.h"
#include <array>
#include <vector>
#include <set>
#include <random>
#include <memory>
#include <iostream>
#include <cstring>
#include <cstdlib>
using namespace farmar::recovered;
namespace {
unsigned char* image=nullptr;
unsigned cycle=0,step=0;
void require(bool condition,const char* message) {if(!condition)throw std::runtime_error(message);}
template<class T> T function(unsigned rva){return reinterpret_cast<T>(image+rva);}
template<class T> T& field(void* p,unsigned offset){return *reinterpret_cast<T*>(static_cast<unsigned char*>(p)+offset);}
void write(unsigned rva,const void* bytes,unsigned size){DWORD old,ignored;require(VirtualProtect(image+rva,size,PAGE_EXECUTE_READWRITE,&old)!=0,"Protect reference operand");std::memcpy(image+rva,bytes,size);VirtualProtect(image+rva,size,old,&ignored);}
void redirect(unsigned rva,void* target){unsigned char code[5]={0xe9};unsigned delta=reinterpret_cast<unsigned>(target)-reinterpret_cast<unsigned>(image+rva+5);std::memcpy(code+1,&delta,4);write(rva,code,5);}
void pointer(unsigned rva,void* p){write(rva,&p,4);}
void* __cdecl allocate(unsigned n){auto* p=std::malloc(n);if(!p)throw std::bad_alloc();return p;}
void __cdecl release(void* p){std::free(p);}
LONG CALLBACK diagnose(EXCEPTION_POINTERS* p){if(p->ExceptionRecord->ExceptionCode==EXCEPTION_ACCESS_VIOLATION){std::cerr<<"Logic AV cycle "<<cycle<<" step "<<step<<" RVA "<<std::hex<<(reinterpret_cast<unsigned>(p->ExceptionRecord->ExceptionAddress)-reinterpret_cast<unsigned>(image))<<" address "<<p->ExceptionRecord->ExceptionInformation[1]<<'\n';TerminateProcess(GetCurrentProcess(),1);}return EXCEPTION_CONTINUE_SEARCH;}
using Event=std::array<unsigned,2>;
std::vector<Event> sourceTrace,nativeTrace;
struct Facade {void** table;void* object;};
void __fastcall bind(Facade* self,void*,unsigned code,InputBindings::Handler callback,void* view){static_cast<InputBindings*>(self->object)->bind(code,callback,view);}
void __fastcall unbind(Facade* self,void*,unsigned code,InputBindings::Handler callback,void* view){static_cast<InputBindings*>(self->object)->unbind(code,callback,view);}
bool __fastcall getBool(Facade* self,void*,const String& group,const String& name,bool& value){require(group.view()=="graphics"&&name.view()=="fullscreen","Native fullscreen property differs");return static_cast<Settings*>(self->object)->get(group,name,value);}
void __fastcall setBool(Facade* self,void*,const String& group,const String& name,const bool& value){require(group.view()=="graphics"&&name.view()=="fullscreen","Native fullscreen property write differs");static_cast<Settings*>(self->object)->set(group,name,value);}
float expectedMultiplier=1;bool expectedWireframe=false;
void __fastcall multiplier(Facade*,void*,const float& value){expectedMultiplier=value;}
void __fastcall reset(Facade*,void*,unsigned char value){nativeTrace.push_back({1,value});}
void __fastcall wireframe(Facade*,void*,unsigned char value){expectedWireframe=value!=0;nativeTrace.push_back({2,value});}
void __fastcall refresh(Facade*,void*){nativeTrace.push_back({3,0});}
void __fastcall screenshot(Facade*,void*){nativeTrace.push_back({4,0});}
unsigned screenshotBufferCalls=0,screenshotBufferReleases=0;bool screenshotBufferFailure=false;
struct Surface {void** table;};
ULONG __stdcall releaseSurface(Surface*) {++screenshotBufferReleases;return 0;}
std::array<void*,3> surfaceTable{};Surface surface{surfaceTable.data()};
HRESULT __stdcall getBackBuffer(void*,UINT index,D3DBACKBUFFER_TYPE type,IDirect3DSurface8** result) {
 require(index==0 && type==D3DBACKBUFFER_TYPE_MONO,"Screenshot back-buffer arguments differ");++screenshotBufferCalls;
 *result=screenshotBufferFailure ? nullptr:reinterpret_cast<IDirect3DSurface8*>(&surface);return screenshotBufferFailure ? E_FAIL:S_OK;
}
DWORD WINAPI clockTime(){return 0;}
void WINAPI sleepTime(DWORD){}
MMRESULT WINAPI period(UINT){return 0;}
String ansi(const wchar_t* p){return toNarrow(WString(p));}
}
int wmain(int argc,wchar_t** argv){
 if(argc!=3)return 2;
 // Only occupy the preferred allocation granule. A three-megabyte range
 // can collide with an unrelated ASLR module and fail despite 00400000
 // being free, accidentally allowing the reference back at its fixed base.
 void* preferred=VirtualAlloc(reinterpret_cast<void*>(0x400000),0x10000,MEM_RESERVE,PAGE_NOACCESS);
 HMODULE reference=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES);if(preferred)VirtualFree(preferred,0,MEM_RELEASE);if(!reference)return 2;
 try {
  image=reinterpret_cast<unsigned char*>(reference);require(reinterpret_cast<unsigned>(image)!=0x400000,"Reference must be relocated for this comparison");AddVectoredExceptionHandler(1,&diagnose);
  struct Fixup{unsigned rva,address;};const Fixup fixups[]={
#include "logic_shortcuts_oracle_fixups.inc"
  };
  std::set<unsigned> patched;for(const auto& f:fixups){if(!patched.insert(f.rva).second)continue;unsigned target;std::memcpy(&target,image+f.rva,4);require(target==f.address,"Logic reference operand differs");target=reinterpret_cast<unsigned>(image)+target-0x400000;write(f.rva,&target,4);}
  redirect(0x17edb1,reinterpret_cast<void*>(&allocate));redirect(0x1803af,reinterpret_cast<void*>(&release));redirect(0x17ffc0,reinterpret_cast<void*>(&std::memmove));FlushInstructionCache(GetCurrentProcess(),image,0x200000);
  FileIndex files(ansi(argv[2]),String("pack"));Settings sourceSettings(files),nativeSettings(files);
  sourceSettings.set(String("graphics"),String("fullscreen"),false);nativeSettings.set(String("graphics"),String("fullscreen"),false);
  TimerServices timerServices{&clockTime,&sleepTime,&period,&period};GameTimer timer(sourceSettings,timerServices);
  InputBindings sourceInput,nativeInput;
  std::array<void*,3> inputTable{};inputTable[1]=reinterpret_cast<void*>(&bind);inputTable[2]=reinterpret_cast<void*>(&unbind);
  std::array<void*,21> settingsTable{};settingsTable[10]=reinterpret_cast<void*>(&getBool);settingsTable[20]=reinterpret_cast<void*>(&setBool);
  std::array<void*,16> graphicsTable{};graphicsTable[5]=reinterpret_cast<void*>(&reset);graphicsTable[15]=reinterpret_cast<void*>(&wireframe);graphicsTable[9]=reinterpret_cast<void*>(&screenshot);
  std::array<void*,6> timerTable{};timerTable[5]=reinterpret_cast<void*>(&multiplier);
  std::array<void*,13> resourceTable{};resourceTable[12]=reinterpret_cast<void*>(&refresh);
  Facade input{inputTable.data(),&nativeInput},settings{settingsTable.data(),&nativeSettings},graphics{graphicsTable.data(),nullptr},nativeTimer{timerTable.data(),nullptr},resources{resourceTable.data(),nullptr};
  pointer(0x25f6e0,&input);pointer(0x25f728,&settings);pointer(0x25ff8c,&graphics);pointer(0x25ff50,&nativeTimer);pointer(0x25f730,&resources);
  for(unsigned n=0;n<4;++n)require(image[0x25d368+n]==1,"Original shortcut initial flag is not1");
  bool sourcePaused=false,sourceWireframe=false;unsigned comparisons=0,frames=0,events=0,lifetimes=0,retainedTemporary=0;
  std::array<unsigned char,24> nativeLogic{};
  const auto compare=[&](bool alive){
   require(sourceTrace==nativeTrace,"Native shortcut service event sequence differs");
   require(std::memcmp(&timer.state().multiplier,&expectedMultiplier,4)==0,"Native shortcut timer multiplier differs");
   require(sourceWireframe==expectedWireframe,"Native wireframe state differs");
   if(alive)require(sourcePaused==bool(nativeLogic[12]),"Native Logic pause state differs");
   bool a=false,b=false;sourceSettings.get(String("graphics"),String("fullscreen"),a);nativeSettings.get(String("graphics"),String("fullscreen"),b);require(a==b,"Native fullscreen setting differs");
   for(unsigned code=0;code<InputBindings::ControlCount;++code){const auto& actual=sourceInput.controls(code);const auto& expected=nativeInput.controls(code);require(actual.size()==expected.size(),"Native modifier binding count differs");for(const auto& value:actual)require(value.view==nullptr,"Source shortcut payload is not native null");for(const auto& value:expected)require(value.view==nullptr,"Native shortcut payload differs");}
   ++comparisons;
  };
  const auto dispatch=[&](std::vector<unsigned> list){++step;events+=unsigned(list.size());sourceInput.dispatchControls(list);nativeInput.dispatchControls(list);compare(true);++frames;};
  std::mt19937 random(0x5315b0);
  const std::array<unsigned,13> choices={0x138,0x1b8,0x38,0xb8,0x1c,0x14,0xc7,0xc5,0xd2,0x3f,0xb7,0x13f,0x114};
  for(cycle=0;cycle<6;++cycle){
   nativeLogic.fill(0);sourcePaused=false;sourceWireframe=expectedWireframe;
   LogicShortcuts logic(sourceInput,sourceSettings,timer,LogicShortcutServices{
    [&](bool value){sourcePaused=value;},[&](bool value){sourceTrace.push_back({1,unsigned(value)});},
    [&](bool value){sourceWireframe=value;sourceTrace.push_back({2,unsigned(value)});},
    [&]{sourceTrace.push_back({3,0});},[&]{sourceTrace.push_back({4,0});}});
   function<void*(__thiscall*)(void*)>(0x1315b0)(nativeLogic.data());++lifetimes;compare(true);
   dispatch({0x38});dispatch({0x1c,0x14,0xc7,0xc5,0xd2,0x3f}); // no modifier: no action
   dispatch({0x138,0x1c,0x14}); // all control snapshots precede binding mutation
   dispatch({0x1c,0x14,0xc7,0xc5,0xd2,0x3f,0xb7});
   dispatch({0xc7,0xc7,0xd2,0xc7,0xc5,0xc5}); // independent process toggles
   dispatch({0x1b8});dispatch({0x38});dispatch({0x1c,0x3f}); // releasing either Alt removes temporary handlers
   for(unsigned n=0;n<600;++n){std::vector<unsigned> list;for(unsigned k=0,count=random()%8;k<count;++k)list.push_back(choices[random()%choices.size()]);dispatch(std::move(list));}
   if(cycle%2)dispatch({0x138});else dispatch({0x38});
   function<void(__thiscall*)(void*)>(0x131a70)(nativeLogic.data());require(field<void*>(image,0x25ff60)==nullptr,"Native Logic destructor retained singleton");
   // Source destructor runs at the same lexical lifetime boundary below; the
   // next iteration compares retained temporary handlers after both destruct.
   if(cycle%2)++retainedTemporary;
  }
  compare(false);require(sourceInput.controls(0x138).empty()&&nativeInput.controls(0x138).empty(),"Permanent Alt handlers retained after destructor");
  require(sourceInput.controls(0x1c).size()==1&&nativeInput.controls(0x1c).size()==1,"Original Alt-held temporary binding residue differs");
  surfaceTable[2]=reinterpret_cast<void*>(&releaseSurface);std::array<void*,17> screenshotDeviceTable{};screenshotDeviceTable[16]=reinterpret_cast<void*>(&getBackBuffer);
  Facade screenshotDevice{screenshotDeviceTable.data(),nullptr};
  {GraphicsScreenshot shot(reinterpret_cast<IDirect3DDevice8*>(&screenshotDevice));shot.prepare();shot.prepare();require(screenshotBufferCalls==1 && screenshotBufferReleases==0,"Screenshot cache did not retain the first back buffer");
   shot.release();require(screenshotBufferReleases==1,"Screenshot pre-reset did not release buffer");screenshotBufferFailure=true;shot.prepare();shot.prepare();screenshotBufferFailure=false;shot.prepare();}
  require(screenshotBufferCalls==4 && screenshotBufferReleases==2,"Screenshot cache/failure/destruction ownership differs");
  std::cout<<"{\"logic_lifetimes\":"<<lifetimes<<",\"logic_states\":"<<comparisons<<",\"input_frames\":"<<frames<<",\"input_events\":"<<events<<",\"service_events\":"<<sourceTrace.size()<<",\"alt_held_destructions\":"<<retainedTemporary<<",\"source_screenshot_api_calls\":"<<screenshotBufferCalls<<",\"source_screenshot_releases\":"<<screenshotBufferReleases<<",\"native_service_boundaries\":true,\"reference_relocated\":true}\n";
 }catch(const std::exception& e){std::cerr<<e.what()<<" cycle "<<cycle<<" step "<<step<<'\n';return 1;}catch(const char* e){std::cerr<<e<<'\n';return 1;}
 FreeLibrary(reference);return 0;
}
