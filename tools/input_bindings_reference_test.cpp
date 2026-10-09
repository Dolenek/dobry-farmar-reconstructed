// Original Input handler sets and buffered dispatch reference only. Implementation libraries
// never load the original PE; CRT/device/service fixtures are explicit adapters.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "InputBindings.h"
#include "InputDevices.h"
#include "FileIndex.h"
#include "ResourceManager.h"
#include "Texture.h"
#include "ImageTexture.h"
#include "Settings.h"
#include "StringManager.h"
#include "StringConversion.h"
#include "XmlDocument.h"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <float.h>
#include <iostream>
#include <random>
#include <set>
#include <stdexcept>
#include <vector>
using namespace farmar::recovered;
namespace {
unsigned char* image=nullptr;
unsigned stage=0,trial=0,action=0;
std::set<void*> allocations;
FileIndex* files=nullptr; IDirect3DDevice8* device=nullptr;
template<class T> T& field(void* object,unsigned offset) { return *reinterpret_cast<T*>(static_cast<unsigned char*>(object)+offset); }
template<class T> T function(unsigned rva) { return reinterpret_cast<T>(image+rva); }
void call(unsigned rva,void* object) { function<void(__thiscall*)(void*)>(rva)(object); }
void require(bool result,const char* message) { if(!result) throw std::runtime_error(message); }
void* __cdecl allocate(unsigned bytes) { auto* p=std::malloc(bytes); if(!p) throw std::bad_alloc(); allocations.insert(p); return p; }
void __cdecl release(void* p) { allocations.erase(p); std::free(p); }
LONG CALLBACK diagnose(EXCEPTION_POINTERS* e) {
    if(e->ExceptionRecord->ExceptionCode==EXCEPTION_ACCESS_VIOLATION) {
        std::cerr<<"Text control AV stage "<<stage<<" trial "<<trial<<" action "<<action<<" RVA 0x"<<std::hex
            <<(reinterpret_cast<unsigned>(e->ExceptionRecord->ExceptionAddress)-reinterpret_cast<unsigned>(image))<<" address 0x"<<e->ExceptionRecord->ExceptionInformation[1]<<'\n';
        TerminateProcess(GetCurrentProcess(),1);
    } return EXCEPTION_CONTINUE_SEARCH;
}
void write(unsigned rva,const void* p,unsigned size) { DWORD old,ignored; require(VirtualProtect(image+rva,size,PAGE_EXECUTE_READWRITE,&old)!=0,"Oracle protection failed"); std::memcpy(image+rva,p,size); VirtualProtect(image+rva,size,old,&ignored); }
void redirect(unsigned rva,void* p) { unsigned char instruction[5]={0xe9}; unsigned delta=reinterpret_cast<unsigned>(p)-reinterpret_cast<unsigned>(image+rva+5); std::memcpy(instruction+1,&delta,4); write(rva,instruction,5); }
void putPointer(unsigned rva,void* p) { std::memcpy(image+rva,&p,4); }
struct Node { Node *left,*parent,*right; void* key; unsigned char color,nil,pad[2]; };
void initializeSet(unsigned rva) { auto* h=static_cast<Node*>(allocate(20)); *h={h,h,h,nullptr,1,1,{0,0}}; std::memset(image+rva,0,12); putPointer(rva+4,h); }
struct HandlerNode { HandlerNode *left,*parent,*right; void* handler;void* view;unsigned char color,nil,pad[2]; };
static_assert(sizeof(HandlerNode)==24);
void init(void* object,unsigned offset) {
    auto* head=static_cast<HandlerNode*>(allocate(24));*head={head,head,head,nullptr,nullptr,1,1,{0,0}};
    field<HandlerNode*>(object,offset+4)=head;
}
using Keys=std::vector<std::pair<unsigned,unsigned>>;
Keys nativeKeys(HandlerNode* node) {
    if(node->nil) return {};auto keys=nativeKeys(node->left);keys.emplace_back(reinterpret_cast<unsigned>(node->handler),reinterpret_cast<unsigned>(node->view));
    const auto right=nativeKeys(node->right);keys.insert(keys.end(),right.begin(),right.end());return keys;
}
template<class Set> Keys keys(const Set& set) { Keys result;for(const auto& pair:set)result.emplace_back(reinterpret_cast<unsigned>(pair.function),reinterpret_cast<unsigned>(pair.view));return result; }
void compareSet(const Keys& actual,void* object,unsigned offset) {
    auto* head=field<HandlerNode*>(object,offset+4);
    require(actual==nativeKeys(head->parent),"Original Input handler keys/order differ");
    require(actual.size()==field<unsigned>(object,offset+8),"Original Input binding count differs");
}
std::vector<std::array<unsigned,4>> actualTrace,expectedTrace;
bool actualPhase=true;void* nativeObject=nullptr;InputDevices* recoveredInput=nullptr;
void event(unsigned kind,unsigned a,unsigned b,void* view) { (actualPhase ? actualTrace:expectedTrace).push_back({kind,a,b,reinterpret_cast<unsigned>(view)}); }
void control1(void*);void control2(void*);
void control0(void* view) {
    event(0,0,0,view);
    if(recoveredInput && trial%7==0) {
        auto* target=reinterpret_cast<void*>(0x3020);
        if(actualPhase) {recoveredInput->bindings().unbind(17,&control1,target);recoveredInput->bindings().bind(17,&control2,target);}
        else {function<void(__thiscall*)(void*,unsigned,InputBindings::Handler,void*)>(0x1280f0)(nativeObject,17,&control1,target);function<void(__thiscall*)(void*,unsigned,InputBindings::Handler,void*)>(0x126190)(nativeObject,17,&control2,target);}
    }
}
void control1(void* view) { event(0,1,0,view); } void control2(void* view) { event(0,2,0,view); }
void mouse0(unsigned axis,int delta,void* view) { event(1,axis,unsigned(delta),view); } void mouse1(unsigned axis,int delta,void* view) { event(2,axis,unsigned(delta),view); }
void char0(wchar_t unit,void* view) { event(3,unit,0,view); } void char1(wchar_t unit,void* view) {
    event(4,unit,0,view);
    // Append during live character dispatch without reallocating the fixed
    // 1024-unit buffer; original loop observes the appended unit this pass.
    if(unit==L'!') { if(actualPhase)recoveredInput->character(L'?');else function<void(__thiscall*)(void*,unsigned)>(0x1245e0)(nativeObject,L'?'); }
}
struct Device { void** table;unsigned id; };
std::array<std::vector<DIDEVICEOBJECTDATA>,2> deviceEvents;
std::array<HRESULT,2> dataResults{},acquireResults{},unacquireResults{};
HRESULT stateResult=0;
DIMOUSESTATE2 mouseState{};std::array<unsigned char,256> keyboardState{};
ULONG __stdcall deviceRelease(Device* d) { event(8,d->id,0,nullptr);return 0; }
HRESULT __stdcall acquire(Device* d) { event(5,d->id,0,nullptr);return acquireResults[d->id]; }
HRESULT __stdcall unacquire(Device* d) { event(6,d->id,0,nullptr);return unacquireResults[d->id]; }
HRESULT __stdcall cooperative(Device* d,HWND window,DWORD flags) { event(7,d->id,flags,window);return 0; }
HRESULT __stdcall getData(Device* d,DWORD size,void* output,DWORD* count,DWORD flags) {
    require(size==20 && flags==0 && *count==1024,"Original GetDeviceData arguments differ");event(9,d->id,0,nullptr);
    *count=unsigned(deviceEvents[d->id].size());std::memcpy(output,deviceEvents[d->id].data(),*count*20);return dataResults[d->id];
}
HRESULT __stdcall getState(Device* d,DWORD size,void* output) {
    event(10,d->id,size,nullptr);
    if(d->id==0) {require(size==256,"Original keyboard state size differs");std::memcpy(output,keyboardState.data(),256);return 0;}
    require(size==20,"Original mouse state size differs");std::memcpy(output,&mouseState,20);return stateResult;
}
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=2) return 2;
    HMODULE reference=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES);if(!reference)return 2;
    try {
        image=reinterpret_cast<unsigned char*>(reference);AddVectoredExceptionHandler(1,&diagnose);
        struct Fixup { unsigned rva,address; };const Fixup fixups[]={
#include "input_bindings_oracle_fixups.inc"
        };
        std::set<unsigned> applied;
        for(const auto& f:fixups) { if(!applied.insert(f.rva).second)continue;unsigned address;std::memcpy(&address,image+f.rva,4);require(address==f.address,"Input oracle operand differs");address=reinterpret_cast<unsigned>(image)+address-0x400000;write(f.rva,&address,4); }
        redirect(0x17edb1,reinterpret_cast<void*>(&allocate));redirect(0x1803af,reinterpret_cast<void*>(&release));redirect(0x17ffc0,reinterpret_cast<void*>(&std::memmove));
        FlushInstructionCache(GetCurrentProcess(),image,0x200000);
        unsigned formatObjects=0;
        // SDK data formats are not assumed to match the original static data.
        // Resolve original absolute pointers solely in this read-only oracle.
        for(const auto& format: {std::pair<unsigned,const DIDATAFORMAT*>{0x20c5e4,&c_dfDIKeyboard},{0x20c7ec,&c_dfDIMouse2}}) {
            const auto* source=reinterpret_cast<const DIDATAFORMAT*>(image+format.first);const auto& target=*format.second;
            require(source->dwSize==target.dwSize && source->dwObjSize==target.dwObjSize && source->dwFlags==target.dwFlags && source->dwDataSize==target.dwDataSize && source->dwNumObjs==target.dwNumObjs,"SDK Input format differs from original");
            const auto* objects=reinterpret_cast<const DIOBJECTDATAFORMAT*>(image+reinterpret_cast<unsigned>(source->rgodf)-0x400000);
            for(unsigned i=0;i<source->dwNumObjs;++i) {
                const auto& a=objects[i];const auto& b=target.rgodf[i];
                require(a.dwOfs==b.dwOfs && a.dwType==b.dwType && a.dwFlags==b.dwFlags && bool(a.pguid)==bool(b.pguid),"SDK Input object format differs from original");
                if(a.pguid) require(!std::memcmp(image+reinterpret_cast<unsigned>(a.pguid)-0x400000,b.pguid,16),"SDK Input object GUID differs from original");++formatObjects;
            }
        }
        std::array<unsigned char,0xc240> object{}; auto* f=object.data();InputBindings input;
        for(unsigned control=0;control<532;++control)init(f,0xa818+control*12);init(f,0xc108);init(f,0xc114);
        std::mt19937 random(0x494e5054);const InputBindings::Handler handlers[]={&control0,&control1,&control2,nullptr};
        const InputBindings::MouseHandler mouseHandlers[]={&mouse0,&mouse1,nullptr};const InputBindings::CharHandler charHandlers[]={&char0,&char1,nullptr};
        unsigned comparisons=0;
        for(trial=0;trial<10000;++trial) {
            const unsigned control=random()%532;auto handler=handlers[random()%4];auto mouseHandler=mouseHandlers[random()%3];auto charHandler=charHandlers[random()%3];
            auto* view=reinterpret_cast<void*>(0x1000+(random()%8)*4);action=random()%9;stage=1;
            switch(action) {
            case 0:input.bind(control,handler,view);function<void(__thiscall*)(void*,unsigned,InputBindings::Handler,void*)>(0x126190)(f,control,handler,view);break;
            case 1:input.unbind(control,handler,view);function<void(__thiscall*)(void*,unsigned,InputBindings::Handler,void*)>(0x1280f0)(f,control,handler,view);break;
            case 2:input.bindMouse(mouseHandler,view);function<void(__thiscall*)(void*,InputBindings::MouseHandler,void*)>(0x126970)(f,mouseHandler,view);break;
            case 3:input.unbindMouse(mouseHandler,view);function<void(__thiscall*)(void*,InputBindings::MouseHandler,void*)>(0x1288e0)(f,mouseHandler,view);break;
            case 4:input.bindKeyboard(charHandler,view);function<void(__thiscall*)(void*,InputBindings::CharHandler,void*)>(0x126580)(f,charHandler,view);break;
            case 5:input.unbindKeyboard(charHandler,view);function<void(__thiscall*)(void*,InputBindings::CharHandler,void*)>(0x1284e0)(f,charHandler,view);break;
            case 6:input.unbindControl(control);function<void(__thiscall*)(void*,unsigned)>(0x124640)(f,control);break;
            case 7:input.unbindAll();call(0x124a30,f);break;
            case 8:input.unbindKeyboard();call(0x124e60,f);break;
            }
            stage=2;for(unsigned control=0;control<532;++control) { compareSet(keys(input.controls(control)),f,0xa818+control*12);++comparisons; }
            compareSet(keys(input.mouse()),f,0xc108);compareSet(keys(input.keyboard()),f,0xc114);comparisons+=2;
        }
        stage=3;call(0x124a30,f);call(0x124e60,f);
        for(const auto& binding:input.mouse()) function<void(__thiscall*)(void*,InputBindings::MouseHandler,void*)>(0x1288e0)(f,binding.function,binding.view);
        std::array<void*,17> deviceTable{};deviceTable[2]=reinterpret_cast<void*>(&deviceRelease);deviceTable[7]=reinterpret_cast<void*>(&acquire);deviceTable[8]=reinterpret_cast<void*>(&unacquire);
        deviceTable[9]=reinterpret_cast<void*>(&getState);deviceTable[10]=reinterpret_cast<void*>(&getData);deviceTable[13]=reinterpret_cast<void*>(&cooperative);
        Device keyboard{deviceTable.data(),0},mouse{deviceTable.data(),1};field<Device*>(f,12)=&keyboard;field<Device*>(f,16)=&mouse;
        const auto window=reinterpret_cast<HWND>(0x1234);field<HWND>(f,4)=window;nativeObject=f;
        unsigned frames=0,callbackEvents=0,keyQueries=0;
        {
            InputDevices deviceInput(window,reinterpret_cast<IDirectInputDevice8A*>(&keyboard),reinterpret_cast<IDirectInputDevice8A*>(&mouse));recoveredInput=&deviceInput;
            const auto bindControl=[&](unsigned c,InputBindings::Handler h,void* v) {deviceInput.bindings().bind(c,h,v);function<void(__thiscall*)(void*,unsigned,InputBindings::Handler,void*)>(0x126190)(f,c,h,v);};
            const auto bindCharacter=[&](InputBindings::CharHandler h,void* v) {deviceInput.bindings().bindKeyboard(h,v);function<void(__thiscall*)(void*,InputBindings::CharHandler,void*)>(0x126580)(f,h,v);};
            const auto bindMovement=[&](InputBindings::MouseHandler h,void* v) {deviceInput.bindings().bindMouse(h,v);function<void(__thiscall*)(void*,InputBindings::MouseHandler,void*)>(0x126970)(f,h,v);};
            for(unsigned c=0;c<531;++c) {bindControl(c,handlers[c%3],reinterpret_cast<void*>(0x2000+c%4*4));if(c%4==0)bindControl(c,&control2,reinterpret_cast<void*>(0x3000));}
            bindControl(531,&control1,reinterpret_cast<void*>(0x4000));bindCharacter(&char0,reinterpret_cast<void*>(0x5000));bindCharacter(&char1,reinterpret_cast<void*>(0x6000));
            bindControl(17,&control1,reinterpret_cast<void*>(0x3020));
            bindMovement(&mouse0,reinterpret_cast<void*>(0x7000));bindMovement(&mouse1,reinterpret_cast<void*>(0x8000));
            const HRESULT statuses[]={S_OK,S_FALSE,DIERR_NOTACQUIRED,E_ACCESSDENIED,HRESULT(2)};
            for(trial=0;trial<500;++trial) {
                stage=4;actualTrace.clear();expectedTrace.clear();
                for(unsigned d=0;d<2;++d) {dataResults[d]=statuses[random()%5];acquireResults[d]=statuses[random()%5];unacquireResults[d]=statuses[random()%5];deviceEvents[d].clear();}
                stateResult=statuses[random()%5];mouseState.lX=int(random()%61)-30;mouseState.lY=int(random()%61)-30;mouseState.lZ=int(random()%61)-30;
                for(auto& key:keyboardState)key=static_cast<unsigned char>(random());
                for(unsigned n=0,count=random()%80;n<count;++n) {DIDEVICEOBJECTDATA data{};data.dwOfs=random()%256;data.dwData=random()%2 ? 0x80:0;deviceEvents[0].push_back(data);}
                for(unsigned n=0,count=random()%60;n<count;++n) {DIDEVICEOBJECTDATA data{};data.dwOfs=random()%24;data.dwData=random()%2 ? 0x80:0;deviceEvents[1].push_back(data);}
                for(unsigned n=0,count=random()%1200;n<count;++n) {const unsigned unit=n%111==0 ? L'!':random()%65536;deviceInput.character(unit);function<void(__thiscall*)(void*,unsigned)>(0x1245e0)(f,unit);}
                require(deviceInput.queuedCharacters()==field<unsigned>(f,0xa814),"Original Input char queue capacity differs");
                actualPhase=true;const bool exclusive=bool(random()%2);deviceInput.exclusiveMouse(exclusive);const bool aq=deviceInput.acquire(),uq=deviceInput.unacquire();
                actualPhase=false;function<void(__thiscall*)(void*,bool)>(0x125a00)(f,exclusive);
                require(aq==bool(function<unsigned(__thiscall*)(void*)>(0x123e20)(f)),"Original Input acquire result differs");require(uq==bool(function<unsigned(__thiscall*)(void*)>(0x124200)(f)),"Original Input unacquire result differs");
                for(unsigned q=0;q<10;++q) {const unsigned key=random()%256;actualPhase=true;const bool pressed=deviceInput.keyPressed(key);actualPhase=false;require(pressed==function<bool(__thiscall*)(void*,unsigned)>(0x125620)(f,key),"Original keyboard cache differs");++keyQueries;}
                actualPhase=true;deviceInput.process();actualPhase=false;call(0x127310,f);
                require(actualTrace==expectedTrace,"Original Input device/character/control/mouse dispatch order differs");callbackEvents+=unsigned(actualTrace.size());
                require(deviceInput.queuedCharacters()==0 && field<unsigned>(f,0xa814)==0 && !field<unsigned char>(f,0xc220),"Original Input process reset differs");++frames;
            }
            // Verify the release portion of the native destructor separately:
            // fixture containers remain owned by this test, not a native ctor.
            actualTrace.clear();expectedTrace.clear();actualPhase=false;keyboard.table[2]=reinterpret_cast<void*>(&deviceRelease);deviceRelease(&keyboard);deviceRelease(&mouse);actualPhase=true;
        }
        require(actualTrace==expectedTrace,"Input device release order differs");recoveredInput=nullptr;
        // Input's destructor is a separate pending device-lifecycle test. All
        // owned remaining nodes/heads are diagnostic allocation cleanup here.
        for(auto* p:allocations)std::free(p);allocations.clear();FreeLibrary(reference);
        std::cout<<"{\"operations\":10000,\"set_comparisons\":"<<comparisons<<",\"controls\":532,\"frames\":"<<frames<<",\"dispatch_events\":"<<callbackEvents<<",\"cached_key_queries\":"<<keyQueries<<",\"format_objects\":"<<formatObjects<<"}\n";
    } catch(const std::exception& e) {std::cerr<<e.what()<<" stage "<<stage<<" trial "<<trial<<" action "<<action<<'\n';return 1;}
}
