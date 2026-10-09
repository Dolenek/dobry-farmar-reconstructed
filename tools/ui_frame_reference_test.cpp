// Test-only original cView methods and original set mutation code. Only the
// allocation/free boundary and timer are adapted to this diagnostic process.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "View.h"
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
unsigned stage=0,currentTrial=0,currentAction=0;
LONG CALLBACK diagnose(EXCEPTION_POINTERS* exception) {
    if(exception->ExceptionRecord->ExceptionCode==EXCEPTION_ACCESS_VIOLATION) {
        std::cerr<<"View oracle AV stage "<<stage<<" trial "<<currentTrial<<" action "<<currentAction
            <<" instruction RVA 0x"<<std::hex<<(reinterpret_cast<unsigned>(exception->ExceptionRecord->ExceptionAddress)-reinterpret_cast<unsigned>(image))
            <<" address 0x"<<exception->ExceptionRecord->ExceptionInformation[1]<<'\n'; TerminateProcess(GetCurrentProcess(),1);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
std::set<void*> allocations;
void* __cdecl allocate(unsigned bytes) { auto* p=std::malloc(bytes); if(!p) throw std::bad_alloc(); allocations.insert(p); return p; }
void __cdecl release(void* p) { allocations.erase(p); std::free(p); }
struct Node { Node *left,*parent,*right; void* key; unsigned char color,nil,padding[2]; };
static_assert(sizeof(Node)==20);
struct Fixture { void** table; unsigned unknown; float transparency,speed; unsigned transition,allocator; Node* children; unsigned count; Fixture* parent; unsigned char shown,parentShown,enabled,parentEnabled,absolute,parentAbsolute,pad[2]; unsigned id; bool destroyed; };
static_assert(offsetof(Fixture,id)==44);
using Method=void(__thiscall*)(void*);
using Constructor=void*(__thiscall*)(void*,void*);
using ParentMethod=void(__thiscall*)(void*,void*);
std::vector<std::pair<unsigned,unsigned>> actualTrace,expectedTrace;
std::vector<Fixture*> fixtures;
std::function<void(unsigned,bool,bool)> stepMutation;
template<class T> T function(unsigned rva) { return reinterpret_cast<T>(image+rva); }
void call(unsigned rva,void* object) { function<Method>(rva)(object); }
void __fastcall shown(Fixture* f,void*) { expectedTrace.emplace_back(f->id,1); call(0xd2ec0,f); }
void __fastcall hidden(Fixture* f,void*) { expectedTrace.emplace_back(f->id,2); call(0xd2f50,f); }
void __fastcall process(Fixture* f,void*) { expectedTrace.emplace_back(f->id,3); call(0xd2fc0,f); }
void __fastcall enabled(Fixture* f,void*) { expectedTrace.emplace_back(f->id,4); function<void(__cdecl*)(void*)>(0x106f90)(f); }
void __fastcall disabled(Fixture* f,void*) { expectedTrace.emplace_back(f->id,5); function<void(__cdecl*)(void*)>(0x107030)(f); }
void __fastcall absolute(Fixture* f,void*) { expectedTrace.emplace_back(f->id,6); function<void(__cdecl*)(void*)>(0x106fe0)(f); }
void __fastcall absoluteDisabled(Fixture* f,void*) { expectedTrace.emplace_back(f->id,7); function<void(__cdecl*)(void*)>(0x107080)(f); }
void* __fastcall destroy(Fixture* f,void*,unsigned) { expectedTrace.emplace_back(f->id,8); call(0xd58a0,f); f->destroyed=true; return f; }
void __fastcall normalStep(Fixture* f,void*) { expectedTrace.emplace_back(f->id,9);if(stepMutation) stepMutation(f->id,false,true); }
void __fastcall absoluteStep(Fixture* f,void*) { expectedTrace.emplace_back(f->id,10);if(stepMutation) stepMutation(f->id,true,true); }
void __fastcall service(void*,void*) {}
class TraceView final : public View {
public:
    TraceView(View* parent,ViewRegistry& registry) : View(parent,registry) {}
    unsigned id=0;
    ~TraceView() override { actualTrace.emplace_back(id,8); }
    void assign(const ViewState& state) { state_=state; }
    void enabledStep() override { actualTrace.emplace_back(id,9);if(stepMutation) stepMutation(id,false,false); }
    void absoluteStep() override { actualTrace.emplace_back(id,10);if(stepMutation) stepMutation(id,true,false); }
    void onShown() override { actualTrace.emplace_back(id,1); View::onShown(); }
    void onHidden() override { actualTrace.emplace_back(id,2); View::onHidden(); }
    void process() override { actualTrace.emplace_back(id,3); View::process(); }
    void onEnabled() override { actualTrace.emplace_back(id,4); View::onEnabled(); }
    void onDisabled() override { actualTrace.emplace_back(id,5); View::onDisabled(); }
    void onAbsoluteEnabled() override { actualTrace.emplace_back(id,6); View::onAbsoluteEnabled(); }
    void onAbsoluteDisabled() override { actualTrace.emplace_back(id,7); View::onAbsoluteDisabled(); }
};
float elapsed=0;
float __fastcall timer(void*,void*) { return elapsed; }
void redirect(unsigned rva,void* target) {
    auto* address=image+rva; DWORD old; if(!VirtualProtect(address,5,PAGE_EXECUTE_READWRITE,&old)) throw std::runtime_error("View oracle protection failed");
    const DWORD offset=reinterpret_cast<DWORD>(target)-reinterpret_cast<DWORD>(address+5);
    address[0]=0xe9; std::memcpy(address+1,&offset,4); DWORD ignored; VirtualProtect(address,5,old,&ignored);
}
void putPointer(unsigned rva,void* p) { std::memcpy(image+rva,&p,4); }
void initializeSet(unsigned rva) {
    auto* head=static_cast<Node*>(allocate(20)); *head={head,head,head,nullptr,1,1,{0,0}};
    std::memset(image+rva,0,12); putPointer(rva+4,head);
}
std::vector<unsigned> treeIds(Node* node) {
    std::vector<unsigned> result;
    if(!node->nil) {
        result=treeIds(node->left); result.push_back(static_cast<Fixture*>(node->key)->id);
        const auto right=treeIds(node->right); result.insert(result.end(),right.begin(),right.end());
    }
    return result;
}
std::vector<unsigned> setIds(unsigned rva) { Node* head; std::memcpy(&head,image+rva+4,4); return treeIds(head->parent); }
std::vector<unsigned> setIds(const ViewRegistry::Views& set) { std::vector<unsigned> result; for(auto* p:set) result.push_back(static_cast<TraceView*>(p)->id); return result; }
void compareRegistry(const ViewRegistry& registry) {
    const std::pair<unsigned,const ViewRegistry::Views*> sets[]={
        {0x260508,&registry.enabled()},{0x260514,&registry.absoluteEnabled()},
        {0x26052c,&registry.pendingEnabledAdds()},{0x260538,&registry.pendingEnabledRemoves()},
        {0x260544,&registry.pendingAbsoluteAdds()},{0x260550,&registry.pendingAbsoluteRemoves()},
        {0x260520,&registry.transitions()}};
    for(const auto& entry:sets) if(setIds(entry.first)!=setIds(*entry.second)) throw std::runtime_error("Original view registry differs");
}
void compareState(const TraceView& view,const Fixture& f) {
    const auto& s=view.state();
    if(std::memcmp(&s.transparency,&f.transparency,4) || std::memcmp(&s.transparencySpeed,&f.speed,4) || s.transition!=f.transition
        || s.shown!=bool(f.shown) || s.parentShown!=bool(f.parentShown) || s.enabled!=bool(f.enabled) || s.parentEnabled!=bool(f.parentEnabled)
        || s.absoluteEnabled!=bool(f.absolute) || s.parentAbsoluteEnabled!=bool(f.parentAbsolute)) throw std::runtime_error("Original view state differs");
    const unsigned actualParent=view.parent() ? static_cast<TraceView*>(view.parent())->id:~0U;
    const unsigned expectedParent=f.parent ? f.parent->id:~0U;
    if(actualParent!=expectedParent) throw std::runtime_error("Original view parent differs");
    if(view.rootTransparency()!=function<float(__thiscall*)(void*)>(0xed4f0)(const_cast<Fixture*>(&f))) throw std::runtime_error("Original root transparency differs");
}
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=2 && argc!=3) return 2;
    auto module=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES); if(!module) return 2;
    try {
        image=reinterpret_cast<unsigned char*>(module);
        if(argc==3) _control87(_PC_24,_MCW_PC);
        AddVectoredExceptionHandler(1,&diagnose);
        struct Fixup { unsigned rva,address; }; const Fixup fixups[]={
#include "ui_frame_oracle_fixups.inc"
        };
        for(const auto& f:fixups) {
            unsigned address; std::memcpy(&address,image+f.rva,4); if(address!=f.address) throw std::runtime_error("View oracle operand differs");
            DWORD old; VirtualProtect(image+f.rva,4,PAGE_EXECUTE_READWRITE,&old);
            address=reinterpret_cast<unsigned>(image)+(address-0x400000); std::memcpy(image+f.rva,&address,4);
            DWORD ignored; VirtualProtect(image+f.rva,4,old,&ignored);
        }
        redirect(0x17edb1,reinterpret_cast<void*>(&allocate)); redirect(0x1803af,reinterpret_cast<void*>(&release));
        const unsigned globals[]={0x26025c,0x260508,0x260514,0x260520,0x26052c,0x260538,0x260544,0x260550};
        for(auto rva:globals) initializeSet(rva);
        auto* discardedHead=static_cast<unsigned char*>(allocate(48)); std::memset(discardedHead,0,48);
        for(unsigned offset:{0U,4U,8U}) std::memcpy(discardedHead+offset,&discardedHead,4);
        discardedHead[44]=discardedHead[45]=1; putPointer(0x2604e8,discardedHead);
        std::array<void*,3> timerTable{}; timerTable[2]=reinterpret_cast<void*>(&timer); auto* tp=timerTable.data(); putPointer(0x25ff50,&tp);
        std::array<void*,14> table{};
        table[0]=reinterpret_cast<void*>(&destroy);table[2]=reinterpret_cast<void*>(&normalStep);table[3]=reinterpret_cast<void*>(&absoluteStep); table[4]=image+0xd3700; table[5]=image+0xd55a0;
        table[7]=reinterpret_cast<void*>(&shown); table[8]=reinterpret_cast<void*>(&hidden); table[9]=reinterpret_cast<void*>(&process);
        table[10]=reinterpret_cast<void*>(&enabled); table[11]=reinterpret_cast<void*>(&disabled);
        table[12]=reinterpret_cast<void*>(&absolute); table[13]=reinterpret_cast<void*>(&absoluteDisabled);
        initializeSet(0x2604fc);initializeSet(0x2605fc);
        std::array<void*,12> inputTable{};inputTable[11]=reinterpret_cast<void*>(&service);auto* inputPointer=inputTable.data();putPointer(0x25f6e0,&inputPointer);
        std::array<void*,3> graphicsTable{};graphicsTable[1]=graphicsTable[2]=reinterpret_cast<void*>(&service);auto* graphicsPointer=graphicsTable.data();putPointer(0x25ff8c,&graphicsPointer);
        timerTable[1]=reinterpret_cast<void*>(&service);redirect(0x130290,reinterpret_cast<void*>(&service));
        FlushInstructionCache(GetCurrentProcess(),image,0x200000);
        ViewRegistry registry; std::vector<TraceView*> views;
        stage=1;
        for(unsigned i=0;i<12;++i) {
            views.push_back(new TraceView(nullptr,registry));
            auto* f=static_cast<Fixture*>(std::calloc(1,sizeof(Fixture))); fixtures.push_back(f);
            function<Constructor>(0xd55b0)(f,nullptr); f->table=table.data();
        }
        stage=2; std::sort(views.begin(),views.end()); std::sort(fixtures.begin(),fixtures.end());
        for(unsigned i=0;i<12;++i) { views[i]->id=fixtures[i]->id=i; compareState(*views[i],*fixtures[i]); }
        unsigned stateComparisons=12,traceEvents=0,registryComparisons=0;
        std::mt19937 random(0x56494557);
        for(unsigned trial=0;trial<16000;++trial) {
            const unsigned index=random()%12,action=random()%12; auto* v=views[index]; auto* f=fixtures[index];
            stage=3; currentTrial=trial; currentAction=action;
            actualTrace.clear(); expectedTrace.clear();
            switch(action) {
            case 0: v->show(); call(0xd37d0,f); break;
            case 1: v->hide(); call(0xd3720,f); break;
            case 2: v->enable(); call(0xd3bb0,f); break;
            case 3: v->absoluteEnable(); call(0xd3fb0,f); break;
            case 4: v->showTransparently(); call(0xd4590,f); break;
            case 5: v->hideTransparently(); call(0xd4980,f); break;
            case 6: v->discardTransparently(); call(0xd4d80,f); break;
            case 7:
                elapsed=float(random()%1001)/3000.f;
                // The original diagnostic String construction at a completed
                // discard is a separate, explicitly unverified boundary.
                if(f->transition!=3 || f->parent || double(f->transparency)-double(elapsed)*f->speed>0) {
                    v->changeTransparency(elapsed); call(0xd59c0,f);
                }
                break;
            case 8: {
                const unsigned p=index ? random()%index:0; const bool hasParent=index && random()%3;
                v->setParent(hasParent ? views[p]:nullptr); function<ParentMethod>(0xd2e50)(f,hasParent ? fixtures[p]:nullptr); break;
            }
            case 9: registry.beginDispatch(); function<void(__cdecl*)()>(0x1069d0)(); break;
            case 10: registry.finishEnabledDispatch(); function<void(__cdecl*)()>(0x106b90)(); break;
            case 11: registry.finishAbsoluteDispatch(); function<void(__cdecl*)()>(0x106d90)(); break;
            }
            if(actualTrace!=expectedTrace) { std::cerr<<"Trace trial "<<trial<<" action "<<action<<" node "<<index<<" actual "<<actualTrace.size()<<" expected "<<expectedTrace.size()<<'\n'; throw std::runtime_error("Original view callback order differs"); }
            traceEvents+=unsigned(actualTrace.size());
            stage=4; for(unsigned i=0;i<12;++i) { compareState(*views[i],*fixtures[i]); ++stateComparisons; }
            compareRegistry(registry); ++registryComparisons;
        }
        unsigned precisionComparisons=0;
        stage=7;auto* v=views[0];auto* f=fixtures[0];v->setParent(nullptr);function<ParentMethod>(0xd2e50)(f,nullptr);
        for(currentTrial=0;currentTrial<10000;++currentTrial) {
            auto state=v->state();unsigned bits=(random()&0x807fffffU)|((100U+random()%50)<<23);float alpha;std::memcpy(&alpha,&bits,4);
            state.transparency=f->transparency=alpha;state.transparencySpeed=f->speed=float(int(random()%100001)-50000)/17;
            state.transition=f->transition=1+random()%2;v->assign(state);elapsed=float(int(random()%100001)-50000)/10000;
            actualTrace.clear();expectedTrace.clear();v->changeTransparency(elapsed);call(0xd59c0,f);
            compareState(*v,*f);if(actualTrace!=expectedTrace) throw std::runtime_error("Transparency precision callback order differs");++precisionComparisons;
        }

        stage=8;unsigned frameComparisons=0;
        stepMutation=[&](unsigned id,bool absolute,bool native) {
            const unsigned target=(id+1+currentTrial%3)%12;
            if(absolute) { if(native) function<void(__cdecl*)(void*)>(0x107030)(fixtures[target]);else registry.unregisterEnabled(views[target]); }
            else if(currentTrial%2) { if(native) function<void(__cdecl*)(void*)>(0x106f90)(fixtures[target]);else registry.registerEnabled(views[target]); }
            else { if(native) function<void(__cdecl*)(void*)>(0x107080)(fixtures[target]);else registry.unregisterAbsolute(views[target]); }
        };
        for(currentTrial=0;currentTrial<512;++currentTrial) {
            // Completed discard diagnostic String construction remains a
            // separate boundary. Frame fixtures convert that mode to hide.
            for(unsigned i=0;i<12;++i) if(fixtures[i]->transition==3) { auto state=views[i]->state();state.transition=fixtures[i]->transition=2;views[i]->assign(state); }
            actualTrace.clear();expectedTrace.clear();elapsed=float(random()%1001)/3000;
            if(currentTrial%5==0) { registry.registerEnabled(views[currentTrial%12]);function<void(__cdecl*)(void*)>(0x106f90)(fixtures[currentTrial%12]); }
            if(currentTrial%7==0) { registry.registerAbsolute(views[currentTrial%12]);function<void(__cdecl*)(void*)>(0x106fe0)(fixtures[currentTrial%12]); }
            // The original check is the transition-set count. Do not overwrite
            // this STL size word with an independent synthetic pause flag.
            if(*reinterpret_cast<unsigned*>(image+0x260528)!=registry.transitions().size()) throw std::runtime_error("Original transition count differs");
            registry.dispatchSteps();registry.updateTransitions([] { return elapsed; });
            alignas(4) std::array<unsigned char,24> frame{};frame[4]=1;
            call(0x131b20,frame.data());
            if(actualTrace!=expectedTrace) throw std::runtime_error("Native main-frame UI traversal/callback order differs");
            compareRegistry(registry);for(unsigned i=0;i<12;++i) compareState(*views[i],*fixtures[i]);++frameComparisons;
        }
        stepMutation={};
        stage=5; registry.finishEnabledDispatch(); function<void(__cdecl*)()>(0x106b90)();
        registry.finishAbsoluteDispatch(); function<void(__cdecl*)()>(0x106d90)();
        actualTrace.clear(); expectedTrace.clear();
        // Delete owned forests, using the original base destructor with owned
        // deleting-destructor callbacks for the diagnostic Fixture objects.
        std::vector<unsigned> roots; for(unsigned i=0;i<12;++i) if(!views[i]->parent()) roots.push_back(i);
        stage=6; for(auto i:roots) { delete views[i]; destroy(fixtures[i],nullptr,1); }
        if(actualTrace!=expectedTrace) throw std::runtime_error("Original owned view deletion order differs");
        compareRegistry(registry);
        for(auto* f:fixtures) { if(!f->destroyed) throw std::runtime_error("Original view remained live"); std::free(f); }
        fixtures.clear();
        // Empty original global sentinels belong to the diagnostic, not the PE.
        for(auto* p:allocations) std::free(p); allocations.clear();
        FreeLibrary(module);
        std::cout<<"{\"operations\":16000,\"state_comparisons\":"<<stateComparisons<<",\"callback_events\":"<<traceEvents<<",\"registry_comparisons\":"<<registryComparisons<<",\"precision_comparisons\":"<<precisionComparisons<<",\"frame_comparisons\":"<<frameComparisons<<",\"owned_views_deleted\":12}\n";
    } catch(const std::exception& error) {
        std::cerr<<error.what()<<'\n'; for(auto* f:fixtures) std::free(f); for(auto* p:allocations) std::free(p); FreeLibrary(module); return 1;
    }
}
