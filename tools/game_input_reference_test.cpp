// Native cGameInput bodies execute only in this diagnostic; game actions are
// explicit boundaries. Production GameInput defaults to actual Game singletons.
#include "GameInput.h"
#include "InputBindings.h"
#include <windows.h>
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
unsigned char* image=nullptr;unsigned cycle=0,step=0;
void require(bool value,const char* text){if(!value)throw std::runtime_error(text);}
template<class T>T fn(unsigned rva){return reinterpret_cast<T>(image+rva);}
template<class T>T& field(void* p,unsigned offset){return *reinterpret_cast<T*>(static_cast<unsigned char*>(p)+offset);}
void write(unsigned rva,const void* bytes,unsigned size){DWORD old,ignored;require(VirtualProtect(image+rva,size,PAGE_EXECUTE_READWRITE,&old)!=0,"Protect reference");std::memcpy(image+rva,bytes,size);VirtualProtect(image+rva,size,old,&ignored);}
void pointer(unsigned rva,void* value){write(rva,&value,4);}
void redirect(unsigned rva,void* target){unsigned char bytes[5]={0xe9};unsigned delta=reinterpret_cast<unsigned>(target)-reinterpret_cast<unsigned>(image+rva+5);std::memcpy(bytes+1,&delta,4);write(rva,bytes,5);}
void* __cdecl allocate(unsigned size){auto* p=std::malloc(size);if(!p)throw std::bad_alloc();return p;}
void __cdecl release(void* p){std::free(p);}
LONG CALLBACK diagnose(EXCEPTION_POINTERS* p){if(p->ExceptionRecord->ExceptionCode==EXCEPTION_ACCESS_VIOLATION){std::cerr<<"GameInput AV cycle "<<cycle<<" step "<<step<<" RVA "<<std::hex<<(reinterpret_cast<unsigned>(p->ExceptionRecord->ExceptionAddress)-reinterpret_cast<unsigned>(image))<<" address "<<p->ExceptionRecord->ExceptionInformation[1]<<'\n';TerminateProcess(GetCurrentProcess(),1);}return EXCEPTION_CONTINUE_SEARCH;}
struct Facade{void** table;InputBindings* input;};
void __fastcall bindCharacters(Facade* self,void*,InputBindings::CharHandler callback,void* object){self->input->bindKeyboard(callback,object);}
void __fastcall clearCharacters(Facade* self,void*){self->input->unbindKeyboard();}
std::array<unsigned char,128> threadData{},locale{};
void* __cdecl currentThread(){return threadData.data();}
std::vector<unsigned> sourceTrace,nativeTrace;
void __cdecl money(unsigned value){require(value==0,"Native money payload");nativeTrace.push_back(1);}
void __cdecl upgrades(unsigned value){require(value==0,"Native upgrade payload");nativeTrace.push_back(2);}
void __cdecl seeds(unsigned value){require(value==0,"Native seed payload");nativeTrace.push_back(3);}
void __cdecl products(unsigned value){require(value==0,"Native product payload");nativeTrace.push_back(4);}
BOOL WINAPI beep(DWORD frequency,DWORD duration){require(frequency==1000&&duration==200,"Native Beep arguments");nativeTrace.push_back(5);return TRUE;}
unsigned unrelatedCalls=0;
void unrelated(wchar_t,void*){++unrelatedCalls;}
}
int wmain(int argc,wchar_t** argv){
 if(argc!=2)return 2;
 auto* preferred=VirtualAlloc(reinterpret_cast<void*>(0x400000),0x10000,MEM_RESERVE,PAGE_NOACCESS);
 auto module=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES);if(preferred)VirtualFree(preferred,0,MEM_RELEASE);if(!module)return 2;
 try {
  image=reinterpret_cast<unsigned char*>(module);require(reinterpret_cast<unsigned>(image)!=0x400000,"Reference relocation required");AddVectoredExceptionHandler(1,&diagnose);
  struct Fixup{unsigned rva,address;};const Fixup fixups[]={
#include "game_input_oracle_fixups.inc"
  };
  std::set<unsigned> patched;for(auto f:fixups){if(!patched.insert(f.rva).second)continue;unsigned value;std::memcpy(&value,image+f.rva,4);require(value==f.address,"Reference operand differs");value=reinterpret_cast<unsigned>(image)+value-0x400000;write(f.rva,&value,4);}
  redirect(0x17edb1,reinterpret_cast<void*>(&allocate));redirect(0x1803af,reinterpret_cast<void*>(&release));redirect(0x17ffc0,reinterpret_cast<void*>(&std::memmove));
  redirect(0x012630,reinterpret_cast<void*>(&money));redirect(0x012650,reinterpret_cast<void*>(&upgrades));redirect(0x0126b0,reinterpret_cast<void*>(&seeds));redirect(0x00c360,reinterpret_cast<void*>(&products));
  pointer(0x1ee058,reinterpret_cast<void*>(&beep));redirect(0x185a13,reinterpret_cast<void*>(&currentThread));field<void*>(threadData.data(),0x64)=locale.data();pointer(0x25da44,locale.data());
  FlushInstructionCache(GetCurrentProcess(),image,0x200000);
  // Entry point is not run: initialize only the original UTF16 global and C
  // locale TLS boundary; actual lowercasing and string bodies still execute.
  auto& nativeBuffer=*new(image+0x25f704) WString;
  for(unsigned unit=0;unit<65536;++unit){auto expected=unit>=65&&unit<=90 ? unit+32:unit;require(fn<unsigned short(__cdecl*)(unsigned short)>(0x180415)(static_cast<unsigned short>(unit))==expected,"Original C-locale UTF16 folding differs");}
  InputBindings sourceInput,nativeInput;std::array<void*,7> table{};table[5]=reinterpret_cast<void*>( &bindCharacters);table[6]=reinterpret_cast<void*>( &clearCharacters);Facade input{table.data(),&nativeInput};pointer(0x25f6e0,&input);
  std::array<unsigned char,224> nativeAwards{};int requested=-1;bool hasGame=false,hasAwards=false;unsigned comparisons=0,units=0,destructions=0;
  const auto availability=[&](bool game,bool award){hasGame=game;hasAwards=award;pointer(0x25f6ec,game ? &input:nullptr);pointer(0x25f6e8,award ? nativeAwards.data():nullptr);};
  const auto compare=[&]{require(sourceTrace==nativeTrace,"Game action order differs");require(GameInput::buffer().view()==nativeBuffer.view(),"Rolling UTF16 buffer differs");require(GameInput::buffer().capacity()==nativeBuffer.capacity(),"Rolling buffer capacity differs");require(requested==field<int>(nativeAwards.data(),220),"Requested award differs");require(sourceInput.keyboard().size()==nativeInput.keyboard().size(),"Keyboard bindings differ");++comparisons;};
  const auto character=[&](wchar_t unit){++step;++units;sourceInput.dispatchCharacter(unit);nativeInput.dispatchCharacter(unit);compare();};
  const auto text=[&](const std::wstring& value){for(auto unit:value)character(unit);};
  std::mt19937 random(0x4030e0);const std::array<std::wstring,12> choices={L"cheatmoney",L"CHEATUPGRADES",L"cHeAtSeEdS",L"cheatproduct",L"CHEATALL",L"a01",L"A21",L"a00",L"a22",L"cheat",L"\b\0x",L"cheatmoneY"};
  for(cycle=0;cycle<6;++cycle){
   availability(false,false);requested=-1;field<int>(nativeAwards.data(),220)=-1;
   std::array<unsigned char,4> nativeObject{};
   auto source=std::make_unique<GameInput>(sourceInput,GameInputServices{[&]{return hasGame;},[]{sourceTrace.push_back(1);},[]{sourceTrace.push_back(2);},[]{sourceTrace.push_back(3);},[]{sourceTrace.push_back(4);},[&]()->int*{return hasAwards ? &requested:nullptr;},[](unsigned frequency,unsigned duration){require(frequency==1000&&duration==200,"Source Beep arguments");sourceTrace.push_back(5);}});
   fn<void*(__thiscall*)(void*)>(0x30e0)(nativeObject.data());compare();
   text(L"CHEATmoneyA01");availability(true,false);text(L"cheatmoneyCHEATUPGRADEScheatseedscheatproductcheatall");
   availability(false,true);
   for(unsigned index=1;index<=21;++index){wchar_t award[]={cycle%2 ? L'A':L'a',wchar_t(L'0'+index/10),wchar_t(L'0'+index%10),0};text(award);require(requested==int(index-1),"Directed award failed");}
   availability(true,true);text(std::wstring(60,L'x')+L"ChEaTmOnEy");character(0);character(0xd800);character(0xffff);text(L"cheatall");
   for(unsigned n=0;n<600;++n){availability(random()%2,random()%2);if(random()%3)text(choices[random()%choices.size()]);else for(unsigned m=0,count=random()%27;m<count;++m)character(static_cast<wchar_t>(random()%65536));}
   sourceInput.bindKeyboard(&unrelated,nullptr);nativeInput.bindKeyboard(&unrelated,nullptr);text(L"retained capacity");
   fn<void(__thiscall*)(void*)>(0x1de0)(nativeObject.data());source.reset();++destructions;compare();require(sourceInput.keyboard().empty()&&nativeInput.keyboard().empty(),"Destructor must clear all char handlers");require(field<void*>(image,0x25f6e4)==nullptr,"Original singleton retained");
  }
  std::cout<<"{\"lifetimes\":6,\"states\":"<<comparisons<<",\"characters\":"<<units<<",\"game_service_events\":"<<sourceTrace.size()<<",\"clear_all_destructions\":"<<destructions<<",\"utf16_c_locale_units\":65536,\"reference_relocated\":true,\"game_action_boundaries\":true}\n";
  nativeBuffer.~WString();
 }catch(const std::exception& e){std::cerr<<e.what()<<" cycle "<<cycle<<" step "<<step<<'\n';return 1;}catch(const char* e){std::cerr<<e<<'\n';return 1;}
 FreeLibrary(module);return 0;
}
