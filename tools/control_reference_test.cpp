// Original cPicture/cControl reference only. The implementation library never
// loads this PE. Original allocators and timer are diagnostic adapters.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Control.h"
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
struct Fixture { alignas(4) unsigned char data[0x238]; unsigned id=0; };
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
unsigned actualEnded=0,expectedEnded=0;
bool removeCallback(unsigned id,unsigned kind) { return ((trial+id+kind)%4)!=0; }
class Probe final : public Picture {
public:
    using Picture::Picture; unsigned id=0;
    void configure(unsigned cols,unsigned rows,float length,bool loop,bool doubleRender,bool playing) {
        geometry_.columns=cols; geometry_.rows=rows; geometry_.invertX=(cols%2)==0; geometry_.invertY=(rows%2)==0;
        animation_.frames=cols*rows; animation_.length=length; animation_.loop=loop; animation_.doubleRender=doubleRender; playing_=playing; dirty_=true;
    }
    void onAnimationEnded() override { ++actualEnded; }
};
bool callback(Control* control,unsigned kind) { auto* p=static_cast<Probe*>(control); actualTrace.emplace_back(p->id,kind); return removeCallback(p->id,kind); }
bool shown(Control* p) { return callback(p,1); } bool hidden(Control* p) { return callback(p,2); } bool update(Control* p) { return callback(p,3); } bool queued(Control* p) { return callback(p,4); }
bool __cdecl originalCallback(Fixture* f,unsigned kind) { expectedTrace.emplace_back(f->id,kind); return removeCallback(f->id,kind); }
bool __cdecl originalShown(Fixture* p) { return originalCallback(p,1); } bool __cdecl originalHidden(Fixture* p) { return originalCallback(p,2); }
bool __cdecl originalUpdate(Fixture* p) { return originalCallback(p,3); } bool __cdecl originalQueued(Fixture* p) { return originalCallback(p,4); }
void __fastcall ended(Fixture*,void*) { ++expectedEnded; }
float __fastcall timer(void*,void*) { return elapsed; }
void floats(float actual,float expected,const char* label) {
    if(std::memcmp(&actual,&expected,4)) { std::cerr<<label<<" actual "<<actual<<" expected "<<expected<<" trial "<<trial<<" action "<<action<<'\n'; throw std::runtime_error("Original control float differs"); }
}
void compare(Probe* p,Fixture* f) {
    const auto& s=p->state(); floats(s.transparency,field<float>(f,8),"transparency");
    if(s.shown!=bool(field<unsigned char>(f,0x24)) || s.parentShown!=bool(field<unsigned char>(f,0x25))
        || s.enabled!=bool(field<unsigned char>(f,0x26)) || s.parentEnabled!=bool(field<unsigned char>(f,0x27))
        || s.absoluteEnabled!=bool(field<unsigned char>(f,0x28)) || s.parentAbsoluteEnabled!=bool(field<unsigned char>(f,0x29))) throw std::runtime_error("Original control View state differs");
    const auto& g=p->geometry(); const auto& a=p->animation();
    floats(g.x,field<float>(f,0x5c),"x"); floats(g.y,field<float>(f,0x60),"y"); floats(g.width,field<float>(f,0x64),"width"); floats(g.height,field<float>(f,0x68),"height");
    floats(g.angle,field<float>(f,0x54),"angle"); floats(g.color.a,field<float>(f,0x224),"alpha"); floats(p->deep(),field<float>(f,0x228),"depth");
    floats(a.time,field<float>(f,0x1ac),"time"); floats(a.length,field<float>(f,0x38),"length");
    if(p->dirty()!=bool(field<unsigned char>(f,0x1b0)) || p->callbackCount()!=field<unsigned>(f,0x1c0)
        || p->frame()!=function<unsigned(__thiscall*)(void*)>(0x13a10)(f)) throw std::runtime_error("Original control dirty/callback/frame differs");
    // sprite() is deliberately not called here: observation must not normalize
    // animation time or update dirty state. Explicit transform actions compare.
}
template<class Set> std::vector<unsigned> recoveredSet(const Set& values) { std::vector<unsigned> ids; for(auto* p:values) ids.push_back(static_cast<const Probe*>(p)->id); return ids; }
void compareSets(const ViewRegistry& views,const ControlRegistry& controls) {
    const std::pair<unsigned,const ViewRegistry::Views*> entries[]={ {0x260508,&views.enabled()},{0x260514,&views.absoluteEnabled()},
        {0x26052c,&views.pendingEnabledAdds()},{0x260538,&views.pendingEnabledRemoves()},{0x260544,&views.pendingAbsoluteAdds()},{0x260550,&views.pendingAbsoluteRemoves()} };
    for(const auto& e:entries) if(originalSet(e.first)!=recoveredSet(*e.second)) throw std::runtime_error("Original control enabled registry differs");
    if(originalSet(0x2604fc)!=recoveredSet(controls.visible())) throw std::runtime_error("Original control depth registry differs");
}
void configure(Probe* p,Fixture* f,std::mt19937& random) {
    const unsigned cols=1+random()%8,rows=1+random()%6; const float length=float(1+random()%100)/20;
    const bool loop=random()%2,doubleRender=random()%2,playing=random()%2;
    p->configure(cols,rows,length,loop,doubleRender,playing);
    field<unsigned>(f,0x34)=cols*rows; field<float>(f,0x38)=length; field<unsigned char>(f,0x3c)=loop;
    field<unsigned>(f,0x40)=cols; field<unsigned>(f,0x44)=rows; field<unsigned char>(f,0x48)=playing;
    field<unsigned char>(f,0x1c4)=(cols%2)==0; field<unsigned char>(f,0x1c5)=(rows%2)==0; field<unsigned char>(f,0x1c6)=doubleRender; field<unsigned char>(f,0x1b0)=1;
}
void __cdecl visitOriginal(Fixture* f) { expectedTrace.emplace_back(f->id,5); }
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=3 && argc!=4) return 2;
    HMODULE module=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES); if(!module) return 2;
    try {
        image=reinterpret_cast<unsigned char*>(module); AddVectoredExceptionHandler(1,&diagnose);
        if(argc==4) _control87(_PC_24,_MCW_PC);
        struct Fixup { unsigned rva,address; }; const Fixup fixups[]={
#include "control_state_oracle_fixups.inc"
        };
        for(const auto& f:fixups) { unsigned address; std::memcpy(&address,image+f.rva,4); if(address!=f.address) throw std::runtime_error("Control oracle operand differs");
            address=reinterpret_cast<unsigned>(image)+address-0x400000; protectWrite(f.rva,&address,4); }
        redirect(0x17edb1,reinterpret_cast<void*>(&allocate)); redirect(0x1803af,reinterpret_cast<void*>(&release));
        for(unsigned rva:{0x26025c,0x2604fc,0x260508,0x260514,0x260520,0x26052c,0x260538,0x260544,0x260550}) initializeSet(rva);
        auto* discarded=static_cast<unsigned char*>(allocate(48)); std::memset(discarded,0,48);
        for(unsigned offset:{0U,4U,8U}) std::memcpy(discarded+offset,&discarded,4); discarded[44]=discarded[45]=1; putPointer(0x2604e8,discarded);
        std::array<void*,3> timerTable{}; timerTable[2]=reinterpret_cast<void*>(&timer); auto* tp=timerTable.data(); putPointer(0x25ff50,&tp);
        std::array<void*,64> table{}; std::memcpy(table.data(),image+0x1fc388,sizeof(table)); table[0xdc/4]=reinterpret_cast<void*>(&ended);
        FlushInstructionCache(GetCurrentProcess(),image,0x200000);
        FileIndex files(toNarrow(WString(argv[2])),String(".pack")); ResourceManager resources(files); ViewRegistry views; ControlRegistry controls;
        uiElapsedTime()=[] { return elapsed; };
        std::vector<Probe*> recovered; std::vector<Fixture*> original;
        stage=1;
        for(unsigned i=0;i<10;++i) {
            recovered.push_back(new Probe(nullptr,resources,views,controls)); original.push_back(new Fixture{});
            function<void*(__thiscall*)(void*,void*)>(0xf16f0)(original.back(),nullptr); field<void**>(original.back(),0)=table.data();
        }
        std::sort(recovered.begin(),recovered.end()); std::sort(original.begin(),original.end());
        for(unsigned i=0;i<10;++i) {
            auto* p=recovered[i]; auto* f=original[i]; p->id=f->id=i; compare(p,f);
            p->shownCallback=&shown; p->hiddenCallback=&hidden; p->updateCallback=&update;
            field<void*>(f,0x230)=reinterpret_cast<void*>(&originalShown); field<void*>(f,0x22c)=reinterpret_cast<void*>(&originalHidden); field<void*>(f,0x234)=reinterpret_cast<void*>(&originalUpdate);
        }
        std::mt19937 random(0x50494354); unsigned snapshots=10,callbacks=0,sprites=0,hits=0,drawTraversals=0;
        for(trial=0;trial<20000;++trial) {
            const unsigned index=random()%10; action=random()%22; auto* p=recovered[index]; auto* f=original[index]; stage=2;
            actualTrace.clear(); expectedTrace.clear();
            const float value=float(int(random()%20001)-10000)/32;
            switch(action) {
            case 0: p->show(); call(0xd37d0,f); break;
            case 1: p->hide(); call(0xd3720,f); break;
            case 2: p->enable(); call(0xd3bb0,f); break;
            case 3: p->disable(); call(0xd3750,f); break;
            case 4: { const Point2 point{value,value*0.375f}; p->setPosition(point); function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(f,&point); break; }
            case 5: p->setDeep(value); function<void(__thiscall*)(void*,const float*)>(0xf1080)(f,&value); break;
            case 6: { const Point2 size{value,value*0.625f}; p->setSize(size); function<void(__thiscall*)(void*,const Point2*)>(0x13b80)(f,&size); break; }
            case 7: p->setAngle(value); function<void(__thiscall*)(void*,const float*)>(0x13b20)(f,&value); break;
            case 8: { const float alpha=value/320; p->setAlpha(alpha); function<void(__thiscall*)(void*,const float*)>(0x13b50)(f,&alpha); break; }
            case 9: { const float time=value/64; p->setTime(time); function<void(__thiscall*)(void*,const float*)>(0x13bc0)(f,&time); break; }
            case 10: { const unsigned frame=random()%3==0 ? random():random()%64; p->setFrame(frame); function<void(__thiscall*)(void*,const unsigned*)>(0xeebb0)(f,&frame); break; }
            case 11: configure(p,f,random); break;
            case 12: p->addCallback(&queued); function<void(__thiscall*)(void*,void*)>(0xf1680)(f,reinterpret_cast<void*>(&originalQueued)); break;
            case 13: elapsed=float(random()%301)/1000; p->enabledStep(); call(0xf1810,f); break;
            case 14: {
                const unsigned parent=index ? random()%index:0; const bool attached=index && random()%3;
                p->setParent(attached ? recovered[parent]:nullptr); function<void(__thiscall*)(void*,void*)>(0xd2e50)(f,attached ? original[parent]:nullptr); break;
            }
            case 15: {
                p->updateTransform(); call(0xf0b70,f);
                if(std::memcmp(&p->sprite(),f->data+0x70,76) || std::memcmp(&p->secondSprite(),f->data+0xbc,76)) throw std::runtime_error("Original controller sprite differs"); ++sprites; break;
            }
            case 16: {
                const auto& g=p->geometry(); const Point2 points[]={{value,value},{g.x,g.y},{g.x-g.width/2,g.y},{g.x+g.width/2,g.y},{g.x,g.y-g.height/2},{g.x,g.y+g.height/2}};
                for(const auto& point:points) { if(p->hitTest(point)!=function<bool(__thiscall*)(void*,const Point2*)>(0x13a50)(f,&point)) throw std::runtime_error("Original control hit test differs"); ++hits; } break;
            }
            case 17: views.beginDispatch(); function<void(__cdecl*)()>(0x1069d0)(); break;
            case 18: views.finishEnabledDispatch(); function<void(__cdecl*)()>(0x106b90)(); break;
            case 19: p->absoluteEnable(); call(0xd3fb0,f); break;
            case 20: p->process(); call(0xed590,f); break;
            case 21: controls.forEachVisible([](Control& c){ actualTrace.emplace_back(static_cast<Probe&>(c).id,5); }); function<void(__cdecl*)(void*)>(0x106a10)(reinterpret_cast<void*>(&visitOriginal)); ++drawTraversals; break;
            }
            if(actualTrace!=expectedTrace || actualEnded!=expectedEnded) throw std::runtime_error("Original Picture callback order differs"); callbacks+=unsigned(actualTrace.size());
            stage=3; for(unsigned i=0;i<10;++i) { compare(recovered[i],original[i]); ++snapshots; } compareSets(views,controls);
        }
        stage=4;
        // Child ownership is separately checked by the cView oracle. Detach
        // here to exercise each real cPicture/cControl destructor exactly once.
        for(unsigned i=0;i<10;++i) { recovered[i]->setParent(nullptr); function<void(__thiscall*)(void*,void*)>(0xd2e50)(original[i],nullptr); }
        for(unsigned i=0;i<10;++i) { delete recovered[i]; call(0xf1780,original[i]); delete original[i]; }
        if(!controls.visible().empty() || !originalSet(0x2604fc).empty() || !originalSet(0x26025c).empty()) throw std::runtime_error("Original Control destructor left registry entries");
        for(auto* p:allocations) std::free(p); allocations.clear(); uiElapsedTime()={}; FreeLibrary(module);
        std::cout<<"{\"operations\":20000,\"state_comparisons\":"<<snapshots<<",\"callback_events\":"<<callbacks<<",\"animation_ended_events\":"<<actualEnded<<",\"sprite_pairs\":"<<sprites<<",\"hit_tests\":"<<hits<<",\"draw_traversals\":"<<drawTraversals<<",\"controls_destroyed\":10}\n";
    } catch(const std::exception& e) { std::cerr<<e.what()<<" stage "<<stage<<" trial "<<trial<<" action "<<action<<'\n'; FreeLibrary(module); return 1; }
}
