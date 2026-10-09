// Original ButtonPrototype/AnimationButton/Button reference only. Implementation libraries
// never load the original PE; CRT/device/service fixtures are explicit adapters.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Button.h"
#include "Resource.h"
#include "FileIndex.h"
#include "ResourceManager.h"
#include "StringConversion.h"
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
float elapsed=0;
struct Fixture { alignas(4) unsigned char data[0x280]; unsigned id=0; };
template<class T> T& field(Fixture* f,unsigned offset) { return *reinterpret_cast<T*>(f->data+offset); }
template<class T> T function(unsigned rva) { return reinterpret_cast<T>(image+rva); }
void call(unsigned rva,void* f) { function<void(__thiscall*)(void*)>(rva)(f); }
std::set<void*> allocations;
void* __cdecl allocate(unsigned bytes) { auto* p=std::malloc(bytes); if(!p) throw std::bad_alloc(); allocations.insert(p); return p; }
void __cdecl release(void* p) { allocations.erase(p); std::free(p); }
LONG CALLBACK diagnose(EXCEPTION_POINTERS* e) {
    if(e->ExceptionRecord->ExceptionCode==EXCEPTION_ACCESS_VIOLATION) {
        std::cerr<<"Control oracle AV stage "<<stage<<" trial "<<trial<<" action "<<action<<" RVA 0x"<<std::hex
            <<(reinterpret_cast<unsigned>(e->ExceptionRecord->ExceptionAddress)-reinterpret_cast<unsigned>(image))<<" address 0x"<<e->ExceptionRecord->ExceptionInformation[1]<<'\n';
        TerminateProcess(GetCurrentProcess(),1);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
void protectWrite(unsigned rva,const void* p,unsigned size) {
    DWORD old,ignored; if(!VirtualProtect(image+rva,size,PAGE_EXECUTE_READWRITE,&old)) throw std::runtime_error("Oracle protection failed");
    std::memcpy(image+rva,p,size); VirtualProtect(image+rva,size,old,&ignored);
}
void redirect(unsigned rva,void* p) {
    unsigned char instruction[5]={0xe9}; unsigned delta=reinterpret_cast<unsigned>(p)-reinterpret_cast<unsigned>(image+rva+5);
    std::memcpy(instruction+1,&delta,4); protectWrite(rva,instruction,5);
}
void putPointer(unsigned rva,void* p) { std::memcpy(image+rva,&p,4); }
struct Node { Node *left,*parent,*right; Fixture* key; unsigned char color,nil,padding[2]; };
static_assert(sizeof(Node)==20);
void initializeSet(unsigned rva) { auto* h=static_cast<Node*>(allocate(20)); *h={h,h,h,nullptr,1,1,{0,0}}; std::memset(image+rva,0,12); putPointer(rva+4,h); }
std::vector<unsigned> treeIds(Node* n) {
    std::vector<unsigned> result; if(n->nil) return result;
    result=treeIds(n->left); result.push_back(n->key->id); auto right=treeIds(n->right); result.insert(result.end(),right.begin(),right.end()); return result;
}
std::vector<unsigned> originalSet(unsigned rva) { Node* head; std::memcpy(&head,image+rva+4,4); return treeIds(head->parent); }
std::vector<std::pair<unsigned,unsigned>> actualTrace,expectedTrace;
std::vector<ButtonPrototype*> recovered;
unsigned id(const ButtonPrototype* p) { const auto it=std::find(recovered.begin(),recovered.end(),p); if(it==recovered.end()) throw std::logic_error("Button not owned"); return unsigned(it-recovered.begin()); }
void event(ButtonPrototype* p,unsigned kind) { actualTrace.emplace_back(id(p),kind); if(kind==1 && (trial+id(p))%13==0) p->setState(ButtonState::Normal); }
void click(ButtonPrototype* p) { event(p,0); } void hover(ButtonPrototype* p) { event(p,1); }
void unhover(ButtonPrototype* p) { event(p,2); } void press(ButtonPrototype* p) { event(p,3); }
void __cdecl nativeEvent(Fixture* p,unsigned kind) { expectedTrace.emplace_back(p->id,kind); if(kind==1 && (trial+p->id)%13==0) function<void(__thiscall*)(void*,unsigned)>(0xf30e0)(p,1); }
void __cdecl nativeClick(Fixture* p) { nativeEvent(p,0); } void __cdecl nativeHover(Fixture* p) { nativeEvent(p,1); }
void __cdecl nativeUnhover(Fixture* p) { nativeEvent(p,2); } void __cdecl nativePress(Fixture* p) { nativeEvent(p,3); }
float __fastcall timer(void*,void*) { return elapsed; }
Point2 cursorPosition{0,0};
const Point2* __fastcall cursor(void*,void*) { return &cursorPosition; }
struct InputAdapter { void** table; InputBindings* bindings; };
void __fastcall bind(InputAdapter* p,void*,unsigned control,InputBindings::Handler handler,void* view) { p->bindings->bind(control,handler,view); }
void __fastcall unbind(InputAdapter* p,void*,unsigned control,InputBindings::Handler handler,void* view) { p->bindings->unbind(control,handler,view); }
void __fastcall bindMouse(InputAdapter* p,void*,InputBindings::MouseHandler handler,void* view) { p->bindings->bindMouse(handler,view); }
void __fastcall unbindMouse(InputAdapter* p,void*,InputBindings::MouseHandler handler,void* view) { p->bindings->unbindMouse(handler,view); }
class Sound final : public cResource {
public:
    explicit Sound(unsigned i):cResource(String("Diagnostic sound")),id(i) {}
    unsigned id; unsigned resourceType() const override { return 4; }
};
void __fastcall playHover(void*,void*,Sound* sound,bool loop,unsigned value) { if(loop || value!=1) throw std::logic_error("Original hover arguments differ"); expectedTrace.emplace_back(sound->id,4); }
void __fastcall playClick(void*,void*,Sound* sound) { expectedTrace.emplace_back(sound->id,5); }
template<class Base> class Probe final : public Base {
public:
    using Base::Base;
    void configure(unsigned frames,float length) { this->animation_.frames=frames; this->animation_.length=length; this->geometry_.columns=frames; this->geometry_.rows=1; this->dirty_=true; }
    void sounds(cResource* click,cResource* hover) { this->clickSound_=click;this->hoverSound_=hover; }
};
using ProbeAnimation=Probe<AnimationButton>;
void compareFloat(float a,float e,const char* label) { if(std::memcmp(&a,&e,4)) { std::cerr<<label<<" actual "<<std::hexfloat<<a<<" expected "<<e<<'\n'; throw std::runtime_error("Original button float differs"); } }
void compare(ButtonPrototype* p,Fixture* f) {
    const auto& s=p->state();
    if(s.shown!=bool(field<unsigned char>(f,0x24)) || s.parentShown!=bool(field<unsigned char>(f,0x25)) || s.enabled!=bool(field<unsigned char>(f,0x26)) || s.parentEnabled!=bool(field<unsigned char>(f,0x27))) throw std::runtime_error("Original Button visibility/enable differs");
    if(unsigned(p->buttonState())!=field<unsigned>(f,0x238)) throw std::runtime_error("Original button state differs");
    compareFloat(p->animation().time,field<float>(f,0x1ac),"button time");
    if(p->dirty()!=bool(field<unsigned char>(f,0x1b0))) throw std::runtime_error("Original button dirty differs");
    if(p->callbackCount()!=field<unsigned>(f,0x1c0)) throw std::runtime_error("Original Button callback queue differs");
}
template<class Set> std::vector<unsigned> recoveredSet(const Set& values) { std::vector<unsigned> result; for(auto* p:values) result.push_back(id(static_cast<const ButtonPrototype*>(p))); return result; }
void compareBindings(const InputBindings& a,const InputBindings& e) {
    for(const unsigned control:{0x200U,0x208U}) {
        std::vector<unsigned> ai,ei;
        for(const auto& b:a.controls(control)) ai.push_back(id(static_cast<ButtonPrototype*>(b.view)));
        for(const auto& b:e.controls(control)) ei.push_back(static_cast<Fixture*>(b.view)->id);
        if(ai!=ei) throw std::runtime_error("Original Button control binding differs");
    }
    std::vector<unsigned> ai,ei;
    for(const auto& b:a.mouse()) ai.push_back(id(static_cast<ButtonPrototype*>(b.view)));
    for(const auto& b:e.mouse()) ei.push_back(static_cast<Fixture*>(b.view)->id);
    if(ai!=ei) throw std::runtime_error("Original Button mouse binding differs");
}
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=3 && argc!=4) return 2;
    HMODULE module=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES); if(!module) return 2;
    try {
        image=reinterpret_cast<unsigned char*>(module); AddVectoredExceptionHandler(1,&diagnose); if(argc==4) _control87(_PC_24,_MCW_PC);
        struct Fixup { unsigned rva,address; }; const Fixup fixups[]={
#include "button_state_oracle_fixups.inc"
        };
        std::set<unsigned> applied;
        for(const auto& f:fixups) { if(!applied.insert(f.rva).second) continue; unsigned address; std::memcpy(&address,image+f.rva,4); if(address!=f.address) throw std::runtime_error("Button oracle operand differs"); address=reinterpret_cast<unsigned>(image)+address-0x400000; protectWrite(f.rva,&address,4); }
        redirect(0x17edb1,reinterpret_cast<void*>(&allocate)); redirect(0x1803af,reinterpret_cast<void*>(&release));
        for(unsigned rva:{0x26025c,0x2604fc,0x260508,0x260514,0x260520,0x26052c,0x260538,0x260544,0x260550}) initializeSet(rva);
        auto* discarded=static_cast<unsigned char*>(allocate(48)); std::memset(discarded,0,48); for(unsigned offset:{0U,4U,8U}) std::memcpy(discarded+offset,&discarded,4); discarded[44]=discarded[45]=1; putPointer(0x2604e8,discarded);
        std::array<void*,3> timerTable{}; timerTable[2]=reinterpret_cast<void*>(&timer); auto* tp=timerTable.data(); putPointer(0x25ff50,&tp);
        std::array<void*,5> cursorTable{}; cursorTable[4]=reinterpret_cast<void*>(&cursor); auto* cp=cursorTable.data(); putPointer(0x25ff60,&cp);
        InputBindings input,nativeInput; std::array<void*,10> inputTable{};
        inputTable[1]=reinterpret_cast<void*>(&bind); inputTable[2]=reinterpret_cast<void*>(&unbind); inputTable[8]=reinterpret_cast<void*>(&bindMouse); inputTable[9]=reinterpret_cast<void*>(&unbindMouse);
        InputAdapter adapter{inputTable.data(),&nativeInput}; putPointer(0x25f6e0,&adapter);
        std::array<void*,3> soundTable{}; soundTable[1]=reinterpret_cast<void*>(&playHover); soundTable[2]=reinterpret_cast<void*>(&playClick); auto* sp=soundTable.data(); putPointer(0x25f734,&sp);
        FlushInstructionCache(GetCurrentProcess(),image,0x200000);
        FileIndex files(toNarrow(WString(argv[2])),String("pack")); ResourceManager resources(files); ViewRegistry views; ControlRegistry controls;
        ButtonInteraction interaction; interaction.cursor=[] { return cursorPosition; };
        interaction.playHover=[](cResource* sound,bool loop,unsigned value) { if(loop || value!=1) throw std::logic_error("Recovered sound arguments"); actualTrace.emplace_back(static_cast<Sound*>(sound)->id,4); };
        interaction.playClick=[](cResource* sound) { actualTrace.emplace_back(static_cast<Sound*>(sound)->id,5); };
        uiElapsedTime()=[] { return elapsed; };
        std::vector<Fixture*> original;
        std::vector<Sound*> soundResources;
        stage=1;
        for(unsigned i=0;i<12;++i) {
            if(i<4) recovered.push_back(new Probe<ButtonPrototype>(nullptr,resources,input,interaction,views,controls));
            else if(i<8) recovered.push_back(new ProbeAnimation(nullptr,resources,input,interaction,views,controls));
            else recovered.push_back(new Probe<Button>(nullptr,resources,input,interaction,views,controls));
            original.push_back(new Fixture{}); function<void*(__thiscall*)(void*,void*)>(i<4 ? 0xf53b0:i<8 ? 0xf1c30:0x107ec0)(original.back(),nullptr);
        }
        // Matching pointer ranks preserve the original tie ordering of bindings
        // and registries. Track dynamic type independently after sorting.
        std::sort(recovered.begin(),recovered.end()); std::sort(original.begin(),original.end());
        for(unsigned i=0;i<12;++i) {
            auto* p=recovered[i]; auto* f=original[i]; f->id=i;
            const unsigned constructor=dynamic_cast<ProbeAnimation*>(p) ? 0xf1c30:dynamic_cast<Button*>(p) ? 0x107ec0:0xf53b0;
            // Reconstruct the matching native dynamic type in its already
            // sorted owned storage after destroying the initial fixture.
            call(0xf5480,f); std::memset(f->data,0,sizeof(f->data)); function<void*(__thiscall*)(void*,void*)>(constructor)(f,nullptr);
            p->clickEvent=&click; p->hoverEvent=&hover; p->unhoverEvent=&unhover; p->pressEvent=&press;
            field<void*>(f,0x23c)=reinterpret_cast<void*>(&nativeClick); field<void*>(f,0x240)=reinterpret_cast<void*>(&nativeHover); field<void*>(f,0x244)=reinterpret_cast<void*>(&nativeUnhover); field<void*>(f,0x248)=reinterpret_cast<void*>(&nativePress);
            if(i%2==0) {
                auto* clickSound=new Sound(i); auto* hoverSound=new Sound(i); auto* nativeClickSound=new Sound(i); auto* nativeHoverSound=new Sound(i);
                for(auto* resource:{clickSound,hoverSound,nativeClickSound,nativeHoverSound}) soundResources.push_back(resource);
                if(auto* a=dynamic_cast<ProbeAnimation*>(p)) a->sounds(clickSound,hoverSound);
                else if(auto* a=dynamic_cast<Probe<Button>*>(p)) a->sounds(clickSound,hoverSound);
                else static_cast<Probe<ButtonPrototype>*>(p)->sounds(clickSound,hoverSound);
                field<cResource*>(f,0x250)=nativeClickSound;field<cResource*>(f,0x254)=nativeHoverSound;
            }
            if(auto* a=dynamic_cast<ProbeAnimation*>(p)) {
                a->configure(4,1.7f); a->normalFrame=0; a->pressedFrame=1; a->hoverFrame=2; a->disabledFrame=3;
                field<unsigned>(f,0x34)=4; field<float>(f,0x38)=1.7f; field<unsigned>(f,0x40)=4; field<unsigned>(f,0x44)=1;
                field<unsigned>(f,0x26c)=0; field<unsigned>(f,0x270)=1; field<unsigned>(f,0x274)=2; field<unsigned>(f,0x278)=3;
            }
            const Point2 position{float(i%4)*25,float(i/4)*20},size{100,80};
            p->setPosition(position); function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(f,&position);
            p->setSize(size); function<void(__thiscall*)(void*,const Point2*)>(0x13b80)(f,&size);
            const float depth=float(i%3); p->setDeep(depth); function<void(__thiscall*)(void*,const float*)>(0xf1080)(f,&depth);
            if(i%3==0) {
                struct NativeRow { unsigned bitCount,allocator; unsigned* words; unsigned size,capacity; };
                static_assert(sizeof(NativeRow)==20);
                auto* nativeRows=static_cast<NativeRow*>(allocate(7*sizeof(NativeRow))); std::vector<ButtonPrototype::MaskRow> rows;
                for(unsigned row=0;row<7;++row) {
                    const unsigned bits=(0x24dU<<row)|(0x63U>>(row%3)); auto* words=static_cast<unsigned*>(allocate(4));*words=bits;
                    nativeRows[row]={11,0,words,1,1}; rows.push_back({11,{bits}});
                }
                p->setMask(std::move(rows));field<unsigned char>(f,0x268)=1;
                field<NativeRow*>(f,0x25c)=nativeRows;field<NativeRow*>(f,0x260)=field<NativeRow*>(f,0x264)=nativeRows+7;
            }
            compare(p,f);
        }
        std::mt19937 random(0x42544e53); unsigned states=12,callbacks=0,spriteCount=0,inputEvents=0,maskHits=0;
        for(trial=0;trial<20000;++trial) {
            const unsigned index=random()%12; action=random()%16; auto* p=recovered[index]; auto* f=original[index]; stage=2;
            actualTrace.clear(); expectedTrace.clear();
            switch(action) {
            case 0:p->show();call(0xd37d0,f);break;
            case 1:p->hide();call(0xd3720,f);break;
            case 2:p->enable();call(0xd3bb0,f);break;
            case 3:p->disable();call(0xd3750,f);break;
            case 4: { const auto state=ButtonState(random()%6); p->setState(state); function<void(__thiscall*)(void*,unsigned)>(0xf30e0)(f,unsigned(state));break; }
            case 5:p->hovered();call(0xf3a00,f);break;
            case 6:p->unhovered();call(0xf3e00,f);break;
            case 7:p->clicked();call(0xf41f0,f);break;
            case 8:p->unclicked();call(0xf4620,f);break;
            case 9: { cursorPosition={float(int(random()%160)-40),float(int(random()%140)-40)}; const std::array<int,3> delta{1,-1,int(random()%3)-1}; input.dispatchMouse(delta);nativeInput.dispatchMouse(delta);break; }
            case 10: { const unsigned event=random()%2 ? 0x208:0x200; input.dispatchControls({event,event});nativeInput.dispatchControls({event,event});inputEvents+=2;break; }
            case 11: { p->deepProcessing=bool(random()%2); field<unsigned char>(f,0x26a)=p->deepProcessing;break; }
            case 12: { const float depth=float(random()%4);p->setDeep(depth);function<void(__thiscall*)(void*,const float*)>(0xf1080)(f,&depth);break; }
            case 13:elapsed=float(random()%500)/1000;p->enabledStep();call(0xf28c0,f);break;
            case 14: { p->soundEnabled=bool(random()%2);p->soundOnUnclick=bool(random()%2);field<unsigned char>(f,0x269)=p->soundEnabled;field<unsigned char>(f,0x26b)=p->soundOnUnclick;break; }
            case 15: { p->updateTransform();call(0xf0b70,f);if(std::memcmp(&p->sprite(),f->data+0x70,76))throw std::runtime_error("Button Sprite differs");++spriteCount;break; }
            }
            if(actualTrace!=expectedTrace) throw std::runtime_error("Original Button event order differs"); callbacks+=unsigned(actualTrace.size());
            auto* active=*reinterpret_cast<Fixture**>(image+0x2604dc);
            if((interaction.active ? id(interaction.active):999)!=(active ? active->id:999) || interaction.pending!=*reinterpret_cast<unsigned*>(image+0x2604e0)) throw std::runtime_error("Original Button deep selection differs");
            stage=3; for(unsigned i=0;i<12;++i) {
                compare(recovered[i],original[i]); ++states;
                if(recovered[i]->pointInRect(cursorPosition)!=function<bool(__thiscall*)(void*,const Point2*)>(0xf34c0)(original[i],&cursorPosition)) throw std::runtime_error("Original Button mask hit differs");++maskHits;
            } compareBindings(input,nativeInput);
            if(recoveredSet(controls.visible())!=originalSet(0x2604fc)) throw std::runtime_error("Button drawable registry differs");
        }
        stage=4;
        for(unsigned i=0;i<12;++i) { const bool textures=dynamic_cast<Button*>(recovered[i])!=nullptr; delete recovered[i]; call(textures ? 0x108120:0xf5480,original[i]); delete original[i]; }
        recovered.clear(); if(!input.mouse().empty() || !nativeInput.mouse().empty()) throw std::runtime_error("Button destructor left mouse bindings");
        for(auto* sound:soundResources) { if(sound->referenceCount()!=0) throw std::runtime_error("Button sound lifetime differs");delete sound; }
        for(auto* p:allocations) std::free(p); allocations.clear(); uiElapsedTime()={}; FreeLibrary(module);
        std::cout<<"{\"operations\":20000,\"state_comparisons\":"<<states<<",\"callback_events\":"<<callbacks<<",\"sprites\":"<<spriteCount<<",\"control_events\":"<<inputEvents<<",\"mask_hit_tests\":"<<maskHits<<",\"buttons_destroyed\":12}\n";
    } catch(const std::exception& e) { std::cerr<<e.what()<<" stage "<<stage<<" trial "<<trial<<" action "<<action<<'\n'; FreeLibrary(module);return 1; }
}
