// Full original TextLine/Letter control methods are a test oracle only.
// Settings, localization, resource factories and string allocation boundaries
// bind to recovered services; original child objects/registries remain native.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "TextLine.h"
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
ResourceManager* resources=nullptr; StringManager* localization=nullptr; bool rtl=false;
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
struct FontNode { FontNode *left,*parent,*right; unsigned short key,first,last,index; unsigned char color,nil,pad[2]; };
static_assert(sizeof(FontNode)==24);
FontNode* tree(std::vector<FontNode>& nodes,unsigned first,unsigned end,FontNode* parent,FontNode* head) {
    if(first==end) return head; auto* node=&nodes[(first+end)/2]; node->parent=parent; node->left=tree(nodes,first,(first+end)/2,node,head); node->right=tree(nodes,(first+end)/2+1,end,node,head); return node;
}
bool __fastcall getBool(void*,void*,const String*,const String*,bool* output) { *output=rtl; return true; }
const WString* __fastcall getString(void*,void*,const WString* source) { return &localization->getString(*source); }
cTexture* __fastcall getTexture(void*,void*,const String* name) { return resources->getTexture(*name); }
bool __fastcall releaseFont(void* font,void*) { return --field<unsigned>(font,4)==0; }
String* __fastcall assignNarrow(String* self,void*,const char* source,unsigned count) { return &self->assign(source,count); }
WString* __fastcall assignWide(WString* self,void*,const WString* source,unsigned start,unsigned count) { return &self->assign(*source,start,count); }
WString* __fastcall assignUnits(WString* self,void*,const wchar_t* source,unsigned count) { return &self->assign(source,count); }
WString* __fastcall appendString(WString* self,void*,const WString* source,unsigned start,unsigned count) { return &self->append(*source,start,count); }
WString* __fastcall appendUnits(WString* self,void*,unsigned count,wchar_t unit) { for(unsigned i=0;i<count;++i) self->append(&unit,1); return self; }
WString* __fastcall replaceUnits(WString* self,void*,unsigned position,unsigned count,const wchar_t* source,unsigned length) {
    std::wstring text(self->view()); text.replace(position,count,source,length); return &self->assign(text.data(),unsigned(text.size()));
}
struct Link { Link *next,*previous; void* control; };
std::vector<void*> originalLetters(void* object) { auto* head=field<Link*>(object,0x24c); std::vector<void*> result; for(auto* node=head->next;node!=head;node=node->next) result.push_back(node->control); return result; }
void compareFloat(float a,float e,const char* message) { if(std::memcmp(&a,&e,4)) { std::cerr<<message<<" actual "<<std::hexfloat<<a<<" expected "<<e<<'\n'; throw std::runtime_error("Original TextLine float differs"); } }
unsigned compare(TextLine& line,void* object) {
    auto expected=originalLetters(object); require(expected.size()==line.letters().size(),"Original TextLine letter pool differs");
    compareFloat(line.textWidth(),field<float>(object,0x29c),"text width"); compareFloat(line.textHeight(),field<float>(object,0x2a0),"text height");
    require(line.textDirty()==bool(field<unsigned char>(object,0x254)),"Original TextLine dirty differs");
    require(line.string().view()==field<WString>(object,0x22c).view(),"Original TextLine string differs");
    unsigned at=0;
    for(auto* letter:line.letters()) {
        auto* f=expected[at++]; const auto& geometry=letter->geometry();
        compareFloat(geometry.x,field<float>(f,0x5c),"letter x"); compareFloat(geometry.y,field<float>(f,0x60),"letter y");
        compareFloat(geometry.width,field<float>(f,0x64),"letter width"); compareFloat(geometry.height,field<float>(f,0x68),"letter height");
        compareFloat(letter->deep(),field<float>(f,0x228),"letter depth"); compareFloat(letter->alpha(),*field<float*>(f,0x58),"letter alpha");
        require(letter->renderFlags()==field<unsigned>(f,0x6c) && letter->packedColor()==field<unsigned>(f,0x248),"Original letter flags/color differs");
        require(!std::memcmp(letter->uv().data(),static_cast<unsigned char*>(f)+0x238,16),"Original letter UV differs");
        call(0x10c500,f); const auto& sprite=letter->sprite(); require(!std::memcmp(&sprite,static_cast<unsigned char*>(f)+0x70,76),"Original owned letter sprite differs");
        require(letter->state().shown==bool(field<unsigned char>(f,0x24)) && letter->state().enabled==bool(field<unsigned char>(f,0x26)),"Original letter activation differs");
        require(letter->texture()==field<cTexture*>(f,0x30),"Original letter texture factory binding differs");
    }
    return at;
}
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=3 && argc!=4) return 2;
    HMODULE reference=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES),runtime=nullptr; HWND window=nullptr;
    IDirect3D8* d3d=nullptr; IDirect3DDevice8* device=nullptr;
    if(!reference) return 2;
    try {
        image=reinterpret_cast<unsigned char*>(reference); AddVectoredExceptionHandler(1,&diagnose);
        struct Fixup { unsigned rva,address; }; const Fixup fixups[]={
#include "text_state_oracle_fixups.inc"
        };
        std::set<unsigned> applied;
        for(const auto& f:fixups) { if(!applied.insert(f.rva).second) continue; unsigned address; std::memcpy(&address,image+f.rva,4); require(address==f.address,"Text control oracle operand differs"); address=reinterpret_cast<unsigned>(image)+address-0x400000; write(f.rva,&address,4); }
        redirect(0x17edb1,reinterpret_cast<void*>(&allocate)); redirect(0x1803af,reinterpret_cast<void*>(&release));
        redirect(0x19c0,reinterpret_cast<void*>(&assignNarrow)); redirect(0x17c0,reinterpret_cast<void*>(&assignWide)); redirect(0x1ab0,reinterpret_cast<void*>(&assignUnits));
        redirect(0xd9dc0,reinterpret_cast<void*>(&appendString)); redirect(0x1600,reinterpret_cast<void*>(&appendUnits)); redirect(0xda980,reinterpret_cast<void*>(&replaceUnits));
        for(unsigned rva:{0x26025c,0x2604fc,0x260508,0x260514,0x260520,0x26052c,0x260538,0x260544,0x260550}) initializeSet(rva);
        auto* discarded=static_cast<unsigned char*>(allocate(48)); std::memset(discarded,0,48); for(unsigned offset:{0U,4U,8U}) std::memcpy(discarded+offset,&discarded,4); discarded[44]=discarded[45]=1; putPointer(0x2604e8,discarded);
        std::array<void*,11> settingsTable{}; settingsTable[10]=reinterpret_cast<void*>(&getBool); auto* settingsPointer=settingsTable.data(); putPointer(0x25f728,&settingsPointer);
        std::array<void*,3> managerTable{}; managerTable[1]=reinterpret_cast<void*>(&getTexture); auto* managerPointer=managerTable.data(); putPointer(0x25f730,&managerPointer);
        std::array<void*,2> stringTable{}; stringTable[1]=reinterpret_cast<void*>(&getString); auto* stringPointer=stringTable.data(); putPointer(0x26026c,&stringPointer);
        // Bypass only static string initialization/atexit; original direction
        // algorithm and original reverse routine execute on ABI String objects.
        new(image+0x260270) WString(); *reinterpret_cast<unsigned*>(image+0x26028c)=1;
        FlushInstructionCache(GetCurrentProcess(),image,0x200000);
        WNDCLASSW cls{}; cls.lpfnWndProc=DefWindowProcW; cls.hInstance=GetModuleHandleW(nullptr); cls.lpszClassName=L"Text control oracle"; RegisterClassW(&cls);
        window=CreateWindowW(cls.lpszClassName,cls.lpszClassName,WS_OVERLAPPEDWINDOW,0,0,800,600,nullptr,nullptr,cls.hInstance,nullptr); require(window!=nullptr,"Diagnostic window failed");
        wchar_t system[MAX_PATH]; GetSystemDirectoryW(system,MAX_PATH); runtime=LoadLibraryW((std::wstring(system)+L"\\d3d8.dll").c_str()); require(runtime!=nullptr,"D3D8 unavailable");
        auto factory=reinterpret_cast<IDirect3D8*(WINAPI*)(UINT)>(GetProcAddress(runtime,"Direct3DCreate8")); d3d=factory(120); require(d3d!=nullptr,"D3D8 SDK120 failed");
        D3DPRESENT_PARAMETERS parameters{}; parameters.BackBufferWidth=800; parameters.BackBufferHeight=600; parameters.BackBufferFormat=D3DFMT_X8R8G8B8; parameters.BackBufferCount=1;
        parameters.SwapEffect=D3DSWAPEFFECT_DISCARD; parameters.Windowed=TRUE; parameters.hDeviceWindow=window;
        const auto controlBefore=_control87(0,0);
        const bool preserve=argc==3;
        require(SUCCEEDED(d3d->CreateDevice(0,D3DDEVTYPE_HAL,window,D3DCREATE_SOFTWARE_VERTEXPROCESSING|(preserve ? D3DCREATE_FPU_PRESERVE:0),&parameters,&device)),"D3D8 device failed");
        if(preserve) require(_control87(0,0)==controlBefore,"Diagnostic FPU precision changed");
        unsigned comparisons=0,directionComparisons=0,scenarios=0;
        {
            FileIndex files(toNarrow(WString(argv[2])),String("pack")); ResourceManager manager(files); resources=&manager;
            textureEnvironment().files=&files; textureEnvironment().load=[&](const void* bytes,const unsigned long& size,IDirect3DTexture8*& texture,TextureImageSize& dimensions) { getImageTexture(device,bytes,size,texture,dimensions); };
            fontFiles()=&files; Settings settings(files,[&](const String& name){return manager.loadSettingsXML(name);}); StringManager strings(files,settings,manager); localization=&strings;
            auto menu=manager.loadSettingsXML(String("UI/MainMenu.xml")); XmlNode* node=nullptr; require(menu->section(String("UserText"),node),"UserText missing");
            auto* font=manager.getFont(toNarrow(node->attribute(WString(L"Font"))));
            std::vector<FontNode> fontNodes(font->ranges().size()); unsigned index=0;
            for(const auto& pair:font->ranges()) { const auto& r=pair.second; auto& n=fontNodes[index++]; n.key=n.last=r.last; n.first=r.first; n.index=r.index; }
            FontNode head{}; head.nil=1; head.parent=tree(fontNodes,0,unsigned(fontNodes.size()),&head,&head); head.left=&fontNodes.front(); head.right=&fontNodes.back();
            std::array<unsigned char,68> fontObject{}; std::array<void*,6> fontTable{}; fontTable[5]=reinterpret_cast<void*>(&releaseFont);
            field<void**>(fontObject.data(),0)=fontTable.data(); field<unsigned>(fontObject.data(),4)=1; field<FontNode*>(fontObject.data(),0x28)=&head; field<const FontGlyph*>(fontObject.data(),0x34)=font->glyphs().data();
            std::mt19937 random(0x4c455454);
            for(trial=0;trial<300;++trial) {
                rtl=trial%2; settings.set(String("game"),String("RightToLeftEnable"),rtl); ViewRegistry views; ControlRegistry controls;
                TextLine line(nullptr,manager,settings,strings,views,controls); stage=1; std::array<unsigned char,0x2b8> object{};
                function<void*(__thiscall*)(void*,void*)>(0xdaca0)(object.data(),nullptr); line.loadXML(*node);
                auto* f=object.data(); call(0xd37d0,f); call(0xd3bb0,f);
                field<String>(f,0x258)=toNarrow(node->attribute(WString(L"Font"))); field<String>(f,0x274)=toNarrow(node->attribute(WString(L"Texture")));
                field<void*>(f,0x2ac)=fontObject.data(); ++field<unsigned>(fontObject.data(),4); field<cTexture*>(f,0x2b0)=manager.getTexture(field<String>(f,0x274));
                field<float>(f,0x5c)=line.geometry().x; field<float>(f,0x60)=line.geometry().y; field<float>(f,0x64)=line.geometry().width; field<float>(f,0x68)=line.geometry().height;
                field<float>(f,0x228)=line.deep(); field<unsigned>(f,0x6c)=line.renderFlags(); field<float>(f,0x290)=10; field<float>(f,0x298)=0.55f;
                WString initial=node->attribute(WString(L"String")); function<void(__thiscall*)(void*,const WString*)>(0xdd030)(f,&initial); call(0xdc800,f);
                for(action=0;action<30;++action) {
                    stage=2;
                    std::wstring value; for(unsigned i=0,n=random()%35;i<n;++i) { const wchar_t choices[]={L' ',wchar_t(0xa0),wchar_t(0xdead),wchar_t(0x17e),wchar_t(0x431),wchar_t(0x5d0),wchar_t(L'0'+random()%10),wchar_t(L'A'+random()%26),wchar_t(L'a'+random()%26)}; value.push_back(choices[random()%9]); }
                    WString text{std::wstring_view(value)};
                    if(action%4==0) { const auto actual=reverseTextDirection(text); const auto* expected=function<const WString*(__cdecl*)(const WString*)>(0xdc1c0)(&text); require(actual.view()==expected->view(),"Original RTL text differs"); ++directionComparisons; }
                    line.setString(text); function<void(__thiscall*)(void*,const WString*)>(0xdd030)(f,&text);
                    if(action%3==0) { const float alpha=float(random()%1001)/1000; line.setAlpha(alpha); function<void(__thiscall*)(void*,const float*)>(0xdbdc0)(f,&alpha); }
                    if(action%5==0) { const Point2 position{float(random()%800),float(random()%600)}; line.setPosition(position); function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(f,&position); }
                    line.enabledStep(); call(0xdae40,f); stage=3; comparisons+=compare(line,f); ++scenarios;
                }
                stage=4; call(0xda840,f);
            }
            font->releaseReference(); require(field<unsigned>(fontObject.data(),4)==1,"Original TextLine leaked font references");
            field<WString>(image,0x260270).~WString(); textureEnvironment()={}; fontFiles()=nullptr; resources=nullptr; localization=nullptr;
        }
        for(auto* p:allocations) std::free(p); allocations.clear(); device->Release(); d3d->Release(); DestroyWindow(window); FreeLibrary(runtime); FreeLibrary(reference);
        std::cout<<"{\"scenarios\":"<<scenarios<<",\"letter_comparisons\":"<<comparisons<<",\"direction_comparisons\":"<<directionComparisons
            <<",\"lines_destroyed\":300,\"device_flags\":"<<(preserve ? 0x22:0x20)<<",\"x87_control_before\":"<<controlBefore<<",\"x87_control_after\":"<<_control87(0,0)<<"}\n";
    } catch(const std::exception& e) { std::cerr<<e.what()<<" stage "<<stage<<" trial "<<trial<<" action "<<action<<'\n'; return 1; }
      catch(const char* e) { std::cerr<<e<<" stage "<<stage<<" trial "<<trial<<" action "<<action<<'\n'; return 1; }
}
