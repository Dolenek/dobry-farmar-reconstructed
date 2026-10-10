// Original owned TextButton/TextAnimationButton menu XML reference only. Implementation libraries
// never load the original PE; CRT/device/service fixtures are explicit adapters.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "monotonic_ui_test_heap.h"
#include "ButtonText.h"
#include "MenuView.h"
#include "MainMenuContent.h"
#include "EditBox.h"
#include "GameTimer.h"
#include "GameCursor.h"
#include "Profile.h"
#include "GameProfileDefaults.h"
#include "FileIndex.h"
#include "ResourceManager.h"
#include "Texture.h"
#include "ImageTexture.h"
#include "Settings.h"
#include "StringManager.h"
#include "StringConversion.h"
#include "XmlDocument.h"
#include "GraphicsRenderState.h"
#include "SpriteBatches.h"
#include "sprite_device_trace.h"
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
#include <map>
#include <memory>
#include <sstream>
#include <filesystem>
using namespace farmar::recovered;
namespace {
unsigned char* image=nullptr;
unsigned stage=0,trial=0,action=0;
float elapsed=0;
float currentElapsed() { return GameTimer::instance() ? GameTimer::instance()->elapsed():elapsed; }
float __fastcall timer(void*,void*) { return currentElapsed(); }
void __fastcall frameService(void*,void*) {}
DWORD menuMilliseconds=0;
DWORD WINAPI menuClock() { return menuMilliseconds; }
void WINAPI menuSleep(DWORD value) { menuMilliseconds+=value; }
MMRESULT WINAPI menuPeriod(UINT) { return 0; }
GameCursor* menuCursor=nullptr;void* nativeCursorFrame=nullptr;
POINT menuScreen{};Point2 menuPoint{};
BOOL WINAPI menuCursorPosition(LPPOINT output) { *output=menuScreen;return TRUE; }
std::vector<unsigned> menuInputEvents;std::array<int,3> menuMouseDelta{};
std::vector<std::pair<std::string,unsigned>> actualSounds,expectedSounds;
unsigned menuSoundRequests=0;
std::set<void*> allocations;
ResourceManager* resources=nullptr; StringManager* localization=nullptr; bool rtl=false;
template<class T> T& field(void* object,unsigned offset) { return *reinterpret_cast<T*>(static_cast<unsigned char*>(object)+offset); }
template<class T> T function(unsigned rva) { return reinterpret_cast<T>(image+rva); }
void call(unsigned rva,void* object) { function<void(__thiscall*)(void*)>(rva)(object); }
void require(bool result,const char* message) { if(!result) throw std::runtime_error(message); }
void* __cdecl allocate(unsigned bytes) { auto* p=::operator new(bytes); allocations.insert(p); return p; }
void __cdecl release(void* p) { allocations.erase(p); if(!ui_test_heap::owns(p)) std::free(p); }
LONG CALLBACK diagnose(EXCEPTION_POINTERS* e) {
    if(e->ExceptionRecord->ExceptionCode==EXCEPTION_ACCESS_VIOLATION) {
        std::cerr<<"Text control AV stage "<<stage<<" trial "<<trial<<" action "<<action<<" RVA 0x"<<std::hex
            <<(reinterpret_cast<unsigned>(e->ExceptionRecord->ExceptionAddress)-reinterpret_cast<unsigned>(image))<<" address 0x"<<e->ExceptionRecord->ExceptionInformation[1]<<'\n';
        auto* stack=reinterpret_cast<unsigned*>(e->ContextRecord->Esp);std::cerr<<"EIP "<<e->ContextRecord->Eip<<" ECX "<<e->ContextRecord->Ecx<<" EDX "<<e->ContextRecord->Edx<<" stack";
        for(unsigned i=0;i<20;++i) std::cerr<<' '<<stack[i];std::cerr<<'\n';
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
std::set<cTexture*> textureResources;
cTexture* __fastcall getTexture(void*,void*,const String* name) { auto* texture=resources->getTexture(*name);textureResources.insert(texture);return texture; }
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
    // Width/height accessors construct dirty letters. Apply the native
    // accessor's virtual makeLetters path before comparing its raw fields.
    if(field<unsigned char>(object,0x254)) call(0xdc800,object);
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
struct XmlAdapter { void** table; const XmlNode* node; };
std::array<void*,39> xmlTable{};
std::map<const XmlNode*,std::unique_ptr<XmlAdapter>> xmlNodes;
XmlAdapter* wrap(const XmlNode* node) {
    auto& adapter=xmlNodes[node]; if(!adapter) adapter=std::make_unique<XmlAdapter>(XmlAdapter{xmlTable.data(),node}); return adapter.get();
}
WString* __fastcall attribute(XmlAdapter* self,void*,WString* output,const wchar_t* key) {
    return new(output) WString(self->node->attribute(WString(key)));
}
bool __fastcall section(XmlAdapter* self,void*,const String* name,XmlAdapter** output) {
    XmlNode* child=nullptr; if(!self->node->section(*name,child)) return false; *output=wrap(child); return true;
}
bool __fastcall indexedSection(XmlAdapter* self,void*,unsigned index,XmlAdapter** output,String* name) {
    XmlNode* child=nullptr;if(!self->node->section(index,child,name)) return false;*output=wrap(child);return true;
}
unsigned __fastcall sectionCount(XmlAdapter* self,void*) { return unsigned(self->node->children().size()); }
template<class T> T* __cdecl typed(T* output,const WString* value) {
    if(!value->empty()) { std::wistringstream stream(value->c_str()); stream>>*output; } return output;
}
template<unsigned N> float* __cdecl vectorValue(float* output,const WString* value) {
    if(!value->empty()) { std::wistringstream stream(value->c_str()); for(unsigned i=0;i<N;++i) { if(i) stream.ignore(1); stream>>output[i]; } } return output;
}
Color* __cdecl colorValue(Color* output,const WString* value) {
    if(!value->empty()) { unsigned packed=0; std::wistringstream stream(value->c_str()); stream>>std::hex>>packed; *output=Color::fromARGB(packed); } return output;
}
String* __fastcall narrow(WString* self,void*,String* output) { return new(output) String(toNarrow(*self)); }
String* __fastcall assignString(String* self,void*,const String* source,unsigned start,unsigned count) { return &self->assign(*source,start,count); }
WString* __fastcall fromNarrow(WString* self,void*,const char* text) { return new(self) WString(toWide(String(text))); }
struct FontFixture {
    std::vector<FontNode> nodes; FontNode head{}; std::array<unsigned char,68> object{}; std::array<void*,6> table{};
    explicit FontFixture(cFont& font):nodes(font.ranges().size()) {
        unsigned i=0; for(const auto& pair:font.ranges()) { const auto& r=pair.second; auto& n=nodes[i++]; n.key=n.last=r.last;n.first=r.first;n.index=r.index; }
        head.nil=1;head.parent=tree(nodes,0,unsigned(nodes.size()),&head,&head);head.left=&nodes.front();head.right=&nodes.back();
        table[5]=reinterpret_cast<void*>(&releaseFont); field<void**>(object.data(),0)=table.data();field<unsigned>(object.data(),4)=1;
        field<FontNode*>(object.data(),0x28)=&head;field<const FontGlyph*>(object.data(),0x34)=font.glyphs().data();
    }
};
std::map<std::string,std::unique_ptr<FontFixture>> fontFixtures;
void* __fastcall getFont(void*,void*,const String* name) {
    auto& fixture=fontFixtures[std::string(name->view())]; if(!fixture) {
        auto* font=resources->getFont(*name); fixture=std::make_unique<FontFixture>(*font);font->releaseReference();
    } ++field<unsigned>(fixture->object.data(),4);return fixture->object.data();
}
class Sound final:public cResource {
public: explicit Sound(const String& name):cResource(name) {} unsigned resourceType() const override { return 4; }
};
std::vector<std::unique_ptr<Sound>> sounds;
cResource* loadSound(const String& name) { sounds.push_back(std::make_unique<Sound>(name));return sounds.back().get(); }
cResource* __fastcall getSound(void*,void*,const String* name) { return loadSound(*name); }
void __fastcall nativeHoverSound(void*,void*,cResource* sound,bool loop,unsigned count) {
    require(!loop && count==1,"Native menu hover sound arguments differ");expectedSounds.emplace_back(std::string(sound->resourceName().view()),0);
}
void __fastcall nativeClickSound(void*,void*,cResource* sound) { expectedSounds.emplace_back(std::string(sound->resourceName().view()),1); }
struct NativeMaskColumn { unsigned count,allocator;unsigned* words;unsigned size,capacity; };
void __fastcall getMask(void*,void*,const String* name,void* output) {
    const auto columns=loadClickMask(*name); auto* rows=static_cast<NativeMaskColumn*>(allocate(unsigned(columns.size())*20));
    for(unsigned i=0;i<columns.size();++i) { const auto& column=columns[i];auto* words=static_cast<unsigned*>(allocate(unsigned(column.words.size())*4));
        std::memcpy(words,column.words.data(),column.words.size()*4);rows[i]={column.bitCount,0,words,unsigned(column.words.size()),unsigned(column.words.size())}; }
    field<NativeMaskColumn*>(output,4)=rows;field<NativeMaskColumn*>(output,8)=field<NativeMaskColumn*>(output,12)=rows+columns.size();
}
Point2 cursorPosition{-1000,-1000};
SpriteBatches* nativeBatches=nullptr;
struct Submission { SpriteDefinition sprite;cTexture* texture;unsigned flags; };
std::vector<Submission> nativeSubmissions;
void __fastcall submitNative(void*,void*,const SpriteDefinition* sprite,cTexture* texture,unsigned flags) {
    require(nativeBatches!=nullptr,"Native menu submission outside frame");
    nativeSubmissions.push_back({*sprite,texture,flags});
    nativeBatches->submit(*sprite,texture,flags,false,DesktopImagePlacement{});
}
const Point2* __fastcall cursor(void* self,void*) { return nativeCursorFrame ? reinterpret_cast<Point2*>(static_cast<unsigned char*>(self)+0x10):&cursorPosition; }
struct InputAdapter { void** table;InputBindings* input; };
void __fastcall menuNativeInput(InputAdapter* self,void*) {
    self->input->dispatchControls(menuInputEvents);self->input->dispatchMouse(menuMouseDelta);
}
void __fastcall bind(InputAdapter* self,void*,unsigned c,InputBindings::Handler h,void* v) { self->input->bind(c,h,v); }
void __fastcall unbind(InputAdapter* self,void*,unsigned c,InputBindings::Handler h,void* v) { self->input->unbind(c,h,v); }
void __fastcall bindMouse(InputAdapter* self,void*,InputBindings::MouseHandler h,void* v) { self->input->bindMouse(h,v); }
void __fastcall unbindMouse(InputAdapter* self,void*,InputBindings::MouseHandler h,void* v) { self->input->unbindMouse(h,v); }
void __fastcall bindCharacters(InputAdapter* self,void*,InputBindings::CharHandler h,void* v) { self->input->bindKeyboard(h,v); }
void __fastcall unbindCharacters(InputAdapter* self,void*,InputBindings::CharHandler h,void* v) { self->input->unbindKeyboard(h,v); }
unsigned editLanguage=0x0405;
HKL WINAPI editKeyboardLayout(DWORD) { return reinterpret_cast<HKL>(editLanguage); }
std::vector<std::pair<std::wstring,bool>> actualEditEvents,nativeEditEvents;
bool editConfirm(wchar_t* value) { if(*value==L'~') *value=L'X';return *value!=L'!'; }
void actualEditChanged(EditBox* self) { actualEditEvents.emplace_back(std::wstring(self->string().view()),self->rightInput()); }
void nativeEditChanged(void* self) { nativeEditEvents.emplace_back(std::wstring(field<WString>(self,0x22c).view()),bool(field<unsigned char>(self,0x2b8))); }
WString* __fastcall substring(WString* self,void*,WString* output,unsigned first,unsigned count) { return new(output) WString(self->view().substr(first,count)); }
void compareControl(Control& control,void* object) {
    compareFloat(control.geometry().x,field<float>(object,0x5c),"control x");compareFloat(control.geometry().y,field<float>(object,0x60),"control y");
    compareFloat(control.geometry().width,field<float>(object,0x64),"control width");compareFloat(control.geometry().height,field<float>(object,0x68),"control height");
    compareFloat(control.deep(),field<float>(object,0x228),"control depth");compareFloat(control.alpha(),*field<float*>(object,0x58),"control alpha");
    compareFloat(control.animation().time,field<float>(object,0x1ac),"control time");
    require(control.texture()==field<cTexture*>(object,0x30),"Control texture differs");
    require(control.state().shown==bool(field<unsigned char>(object,0x24)) && control.state().parentShown==bool(field<unsigned char>(object,0x25)) &&
        control.state().enabled==bool(field<unsigned char>(object,0x26)) && control.state().parentEnabled==bool(field<unsigned char>(object,0x27)),"Control enable/show differs");
}
std::map<cTexture*,int> textureCounts() {
    std::map<cTexture*,int> result;for(auto* texture:textureResources) result[texture]=texture->referenceCount();return result;
}
void compareReferenceDeltas(const std::map<cTexture*,int>& before,const std::map<cTexture*,int>& middle) {
    for(auto* texture:textureResources) {
        if(middle.at(texture)-before.at(texture)!=texture->referenceCount()-middle.at(texture)) std::cerr<<"Reference delta "<<texture->resourceName().c_str()<<' '<<before.at(texture)<<' '<<middle.at(texture)<<' '<<texture->referenceCount()<<'\n';
        require(middle.at(texture)-before.at(texture)==texture->referenceCount()-middle.at(texture),"Owned texture reference delta differs");
    }
}
void collectControls(Node* node,std::vector<void*>& output) {
    if(node->nil) return;collectControls(node->left,output);output.push_back(node->key);collectControls(node->right,output);
}
unsigned compareDecorations(MenuView& view,void* original) {
    auto* head=field<Node*>(original,0x30);std::vector<void*> expected;collectControls(head->parent,expected);
    std::vector<Control*> actual;for(auto* item:view.decorations()) actual.push_back(static_cast<Control*>(item));
    require(actual.size()==expected.size(),"Menu decoration count differs");
    const auto actualLess=[](Control* a,Control* b) { if(a->deep()!=b->deep()) return a->deep()<b->deep();return a->geometry().y<b->geometry().y; };
    const auto nativeLess=[](void* a,void* b) { if(field<float>(a,0x228)!=field<float>(b,0x228)) return field<float>(a,0x228)<field<float>(b,0x228);return field<float>(a,0x60)<field<float>(b,0x60); };
    std::sort(actual.begin(),actual.end(),actualLess);std::sort(expected.begin(),expected.end(),nativeLess);
    for(unsigned i=0;i<actual.size();++i) { compareControl(*actual[i],expected[i]);actual[i]->updateTransform();call(0xf0b70,expected[i]);
        require(!std::memcmp(&actual[i]->sprite(),static_cast<unsigned char*>(expected[i])+0x70,76),"Menu decoration sprite differs"); }
    return unsigned(actual.size());
}

std::vector<unsigned char> readPixels(IDirect3DDevice8* device) {
    IDirect3DSurface8 *back=nullptr,*copy=nullptr;
    require(SUCCEEDED(device->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&back)),"Menu backbuffer unavailable");
    D3DSURFACE_DESC desc{};back->GetDesc(&desc);
    require(SUCCEEDED(device->CreateImageSurface(desc.Width,desc.Height,desc.Format,&copy)),"Menu image surface unavailable");
    const auto copied=device->CopyRects(back,nullptr,0,copy,nullptr);back->Release();require(SUCCEEDED(copied),"Menu readback failed");
    D3DLOCKED_RECT lock{};require(SUCCEEDED(copy->LockRect(&lock,nullptr,D3DLOCK_READONLY)),"Menu readback lock failed");
    require(desc.Format==D3DFMT_X8R8G8B8,"Menu readback format differs");
    std::vector<unsigned char> result(desc.Width*desc.Height*4);
    for(unsigned y=0;y<desc.Height;++y) std::memcpy(result.data()+y*desc.Width*4,static_cast<unsigned char*>(lock.pBits)+y*lock.Pitch,desc.Width*4);
    for(unsigned i=3;i<result.size();i+=4) result[i]=0;
    copy->UnlockRect();copy->Release();return result;
}
unsigned long long compareMenuFrame(IDirect3D8* d3d,IDirect3DDevice8* device,HWND window,ViewRegistry& views,ControlRegistry& controls,InputBindings& input) {
    require(GameTimer::instance()!=nullptr,"Menu frame has no reconstructed timer");
    PrimaryGraphicsSettings settings;GraphicsPresentation presentation(d3d,window);presentation.prepare(settings);
    WINDOWINFO bounds{};bounds.cbSize=sizeof(bounds);require(GetWindowInfo(window,&bounds)!=FALSE,"Menu input client bounds failed");
    menuScreen={static_cast<LONG>(menuPoint[0])+bounds.rcClient.left,static_cast<LONG>(menuPoint[1])+bounds.rcClient.top};
    require(menuCursor && nativeCursorFrame,"Menu cursor frame is not bound");menuCursor->update(false,DesktopImagePlacement{});
    actualSounds.clear();expectedSounds.clear();
    GameTimer::instance()->process();
    input.dispatchControls(menuInputEvents);input.dispatchMouse(menuMouseDelta);
    views.dispatchSteps();views.updateTransitions([] { return currentElapsed(); });
    GraphicsRenderState render(device);SpriteBuffers actualBuffers(device),expectedBuffers(device);actualBuffers.resize(3000);expectedBuffers.resize(3000);
    SpriteBatches actualBatches(actualBuffers),expectedBatches(expectedBuffers);
    std::vector<Submission> actualSubmissions;
    actualBatches.begin();controls.forEachVisible([&](Control& control) {
        actualSubmissions.push_back({control.sprite(),control.texture(),control.renderFlags()});
        if(control.animation().doubleRender) actualSubmissions.push_back({control.secondSprite(),control.texture(),control.renderFlags()});
        control.submitSprites(actualBatches,false,DesktopImagePlacement{});
    });actualBatches.finish();
    expectedBatches.begin();nativeBatches=&expectedBatches;nativeSubmissions.clear();
    call(0x131b20,nativeCursorFrame);nativeBatches=nullptr;expectedBatches.finish();
    require(!std::memcmp(menuCursor->position().data(),static_cast<unsigned char*>(nativeCursorFrame)+0x10,8),"Menu native/source cursor differs");
    require(actualSounds==expectedSounds,"Menu input sound request order differs");
    menuSoundRequests+=unsigned(actualSounds.size());
    require(actualSubmissions.size()==nativeSubmissions.size(),"Menu submission count differs");
    for(unsigned i=0;i<actualSubmissions.size();++i) {
        const auto& actual=actualSubmissions[i];const auto& expected=nativeSubmissions[i];
        if(actual.texture!=expected.texture || actual.flags!=expected.flags || std::memcmp(&actual.sprite,&expected.sprite,76)) {
            std::cerr<<"Submission "<<i<<" texture "<<(actual.texture ? actual.texture->resourceName().c_str():"null")<<'/'<<(expected.texture ? expected.texture->resourceName().c_str():"null")<<" flags "<<actual.flags<<'/'<<expected.flags<<'\n';
            for(unsigned word=0;word<19;++word) if(field<unsigned>(const_cast<SpriteDefinition*>(&actual.sprite),word*4)!=field<unsigned>(const_cast<SpriteDefinition*>(&expected.sprite),word*4))
                std::cerr<<"Sprite word "<<word<<" actual "<<std::hex<<field<unsigned>(const_cast<SpriteDefinition*>(&actual.sprite),word*4)<<" native "<<field<unsigned>(const_cast<SpriteDefinition*>(&expected.sprite),word*4)<<std::dec<<'\n';
            throw std::runtime_error("Menu ordered sprite submission differs");
        }
    }
    require(actualBatches.vertexCount()==expectedBatches.vertexCount() && actualBatches.indexCount()==expectedBatches.indexCount(),"Menu submitted geometry counts differ");
    const auto reset=[&] {
        render.setup(settings,presentation);device->SetVertexShader(0x142);device->SetRenderState(D3DRS_STENCILENABLE,0);
        device->SetTextureStageState(0,D3DTSS_MAGFILTER,1);device->SetTextureStageState(0,D3DTSS_MINFILTER,1);
        require(SUCCEEDED(device->Clear(0,nullptr,D3DCLEAR_TARGET,0xff17293b,0,0)),"Menu clear failed");
        require(SUCCEEDED(device->BeginScene()),"Menu BeginScene failed");
    };
    SpriteDeviceTrace actualTrace(device),expectedTrace(device);
    reset();const auto actualCount=drawSpriteBatches(actualTrace.device(),actualBuffers,actualBatches.batches(),render.transposedViewProjection(),false);
    require(SUCCEEDED(device->EndScene()),"Menu recovered EndScene failed");const auto actual=readPixels(device);
    struct BatchNode { BatchNode *next,*previous;DWORD words[9]; };
    std::vector<BatchNode> nodes(expectedBatches.batches().size()+1);auto* head=&nodes[0];head->next=nodes.size()>1 ? &nodes[1]:head;head->previous=&nodes.back();
    for(unsigned i=0;i<expectedBatches.batches().size();++i) {
        auto& node=nodes[i+1];node.next=i+2<nodes.size() ? &nodes[i+2]:head;node.previous=&nodes[i];
        std::memcpy(node.words,&expectedBatches.batches()[i],32);node.words[8]=0;
    }
    std::array<unsigned char,680> graphics{};field<IDirect3DDevice8*>(graphics.data(),0x10)=expectedTrace.device();
    field<IDirect3DIndexBuffer8*>(graphics.data(),0x15c)=expectedBuffers.indices();field<IDirect3DVertexBuffer8*>(graphics.data(),0x160)=expectedBuffers.vertices();
    field<void*>(graphics.data(),0x180)=head;field<Matrix4>(graphics.data(),0x98)=render.transposedViewProjection();
    reset();call(0x115830,graphics.data());require(SUCCEEDED(device->EndScene()),"Menu original EndScene failed");const auto expected=readPixels(device);
    require(actualCount==field<unsigned>(graphics.data(),0x1e8),"Menu triangle metric differs");
    // Buffer pointers differ because the two hierarchies populate independent
    // GPU buffers. Normalize only those two diagnostic resource identities.
    for(auto& entry:expectedTrace.calls) {
        if(entry[0]==85) entry[1]=reinterpret_cast<unsigned>(actualBuffers.indices());
        if(entry[0]==83) entry[2]=reinterpret_cast<unsigned>(actualBuffers.vertices());
    }
    if(actual!=expected) {
        for(unsigned i=0;i<actual.size();++i) if(actual[i]!=expected[i]) { std::cerr<<"Menu pixel "<<i/4<<" channel "<<i%4<<" actual "<<unsigned(actual[i])<<" native "<<unsigned(expected[i])<<'\n';break; }
        throw std::runtime_error("Whole menu RGB pixels differ");
    }
    if(actualTrace.calls!=expectedTrace.calls) {
        std::cerr<<"Menu draw calls "<<actualTrace.calls.size()<<'/'<<expectedTrace.calls.size()<<'\n';
        for(unsigned i=0;i<actualTrace.calls.size() && i<expectedTrace.calls.size();++i) if(actualTrace.calls[i]!=expectedTrace.calls[i]) {
            std::cerr<<"Draw call "<<i<<" actual";for(auto word:actualTrace.calls[i]) std::cerr<<' '<<word;
            std::cerr<<" native";for(auto word:expectedTrace.calls[i]) std::cerr<<' '<<word;std::cerr<<'\n';break;
        }
        throw std::runtime_error("Whole menu draw API calls differ");
    }
    unsigned nonbackground=0;for(unsigned i=0;i<actual.size();i+=4) if(actual[i]!=0x3b || actual[i+1]!=0x29 || actual[i+2]!=0x17) ++nonbackground;
    require(nonbackground>100000,"Whole menu has no substantial rendered content");return actual.size()/4;
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
#include "text_buttons_oracle_fixups.inc"
#include "sprite_draw_oracle_fixups.inc"
#include "ui_frame_oracle_fixups.inc"
#include "cursor_oracle_fixups.inc"
        };
        std::set<unsigned> applied;
        for(const auto& f:fixups) { if(!applied.insert(f.rva).second) continue; unsigned address; std::memcpy(&address,image+f.rva,4); require(address==f.address,"Text control oracle operand differs"); address=reinterpret_cast<unsigned>(image)+address-0x400000; write(f.rva,&address,4); }
        redirect(0x17edb1,reinterpret_cast<void*>(&allocate)); redirect(0x1803af,reinterpret_cast<void*>(&release));
        redirect(0x19c0,reinterpret_cast<void*>(&assignNarrow)); redirect(0x17c0,reinterpret_cast<void*>(&assignWide)); redirect(0x1ab0,reinterpret_cast<void*>(&assignUnits));
        redirect(0x16d0,reinterpret_cast<void*>(&assignString));redirect(0xda20,reinterpret_cast<void*>(&narrow));redirect(0xdb50,reinterpret_cast<void*>(&fromNarrow));
        redirect(0xd95c0,reinterpret_cast<void*>(&typed<float>));redirect(0xd9670,reinterpret_cast<void*>(&typed<bool>));
        redirect(0xd97d0,reinterpret_cast<void*>(&typed<unsigned>));redirect(0xd9720,reinterpret_cast<void*>(&typed<unsigned>));redirect(0xd94a0,reinterpret_cast<void*>(&typed<unsigned>));
        redirect(0xd9240,reinterpret_cast<void*>(&vectorValue<2>));redirect(0xd9180,reinterpret_cast<void*>(&vectorValue<3>));redirect(0xd93b0,reinterpret_cast<void*>(&colorValue));
        redirect(0xd9dc0,reinterpret_cast<void*>(&appendString)); redirect(0x1600,reinterpret_cast<void*>(&appendUnits)); redirect(0xda980,reinterpret_cast<void*>(&replaceUnits));
        redirect(0x1c10,reinterpret_cast<void*>(&substring));
        for(unsigned rva:{0x26025c,0x2604fc,0x2605fc,0x260508,0x260514,0x260520,0x26052c,0x260538,0x260544,0x260550}) initializeSet(rva);
        auto* discarded=static_cast<unsigned char*>(allocate(48)); std::memset(discarded,0,48); for(unsigned offset:{0U,4U,8U}) std::memcpy(discarded+offset,&discarded,4); discarded[44]=discarded[45]=1; putPointer(0x2604e8,discarded);
        std::array<void*,11> settingsTable{}; settingsTable[10]=reinterpret_cast<void*>(&getBool); auto* settingsPointer=settingsTable.data(); putPointer(0x25f728,&settingsPointer);
        std::array<void*,6> managerTable{}; managerTable[1]=reinterpret_cast<void*>(&getTexture);managerTable[2]=reinterpret_cast<void*>(&getFont);managerTable[3]=reinterpret_cast<void*>(&getSound); auto* managerPointer=managerTable.data(); putPointer(0x25f730,&managerPointer);
        std::array<void*,2> stringTable{}; stringTable[1]=reinterpret_cast<void*>(&getString); auto* stringPointer=stringTable.data(); putPointer(0x26026c,&stringPointer);
        // Bypass only static string initialization/atexit; original direction
        // algorithm and original reverse routine execute on ABI String objects.
        new(image+0x260270) WString(); *reinterpret_cast<unsigned*>(image+0x26028c)=1;
        std::array<void*,3> timerTable{};timerTable[1]=reinterpret_cast<void*>(&frameService);timerTable[2]=reinterpret_cast<void*>(&timer);auto* timerPointer=timerTable.data();putPointer(0x25ff50,&timerPointer);
        uiElapsedTime()=[] { return currentElapsed(); };
        xmlTable[4]=reinterpret_cast<void*>(&section);xmlTable[5]=reinterpret_cast<void*>(&indexedSection);xmlTable[31]=reinterpret_cast<void*>(&attribute);xmlTable[38]=reinterpret_cast<void*>(&sectionCount);
        std::array<void*,5> cursorTable{};cursorTable[4]=reinterpret_cast<void*>(&cursor);auto* cursorPointer=cursorTable.data();putPointer(0x25ff60,&cursorPointer);
        std::array<void*,20> graphicsTable{};graphicsTable[1]=graphicsTable[2]=reinterpret_cast<void*>(&frameService);graphicsTable[11]=reinterpret_cast<void*>(&submitNative);graphicsTable[18]=reinterpret_cast<void*>(&getMask);
        graphicsTable[14]=image+0x112330;graphicsTable[19]=image+0x10d260;
        std::array<unsigned char,680> graphicsObject{};field<void*>(graphicsObject.data(),0)=graphicsTable.data();putPointer(0x25ff8c,graphicsObject.data());
        void* windowInfoFunction=reinterpret_cast<void*>(&GetWindowInfo);write(0x1ee23c,&windowInfoFunction,4);
        void* cursorPositionFunction=reinterpret_cast<void*>(&menuCursorPosition);write(0x1ee258,&cursorPositionFunction,4);
        InputBindings input,nativeInput;std::array<void*,12> inputTable{};inputTable[1]=reinterpret_cast<void*>(&bind);inputTable[2]=reinterpret_cast<void*>(&unbind);inputTable[11]=reinterpret_cast<void*>(&frameService);
        inputTable[8]=reinterpret_cast<void*>(&bindMouse);inputTable[9]=reinterpret_cast<void*>(&unbindMouse);InputAdapter inputAdapter{inputTable.data(),&nativeInput};putPointer(0x25f6e0,&inputAdapter);
        inputTable[5]=reinterpret_cast<void*>(&bindCharacters);inputTable[7]=reinterpret_cast<void*>(&unbindCharacters);
        void* keyboardLayoutFunction=reinterpret_cast<void*>(&editKeyboardLayout);write(0x1ee254,&keyboardLayoutFunction,4);
        ButtonInteraction interaction;interaction.cursor=[] { return cursorPosition; };interaction.loadSound=&loadSound;
        interaction.cursor=[] { return menuCursor ? menuCursor->position():cursorPosition; };
        interaction.playHover=[](cResource* sound,bool loop,unsigned count) { require(!loop && count==1,"Menu source hover sound arguments differ");actualSounds.emplace_back(std::string(sound->resourceName().view()),0); };
        interaction.playClick=[](cResource* sound) { actualSounds.emplace_back(std::string(sound->resourceName().view()),1); };
        std::array<void*,3> soundTable{};soundTable[1]=reinterpret_cast<void*>(&nativeHoverSound);soundTable[2]=reinterpret_cast<void*>(&nativeClickSound);auto* soundPointer=soundTable.data();putPointer(0x25f734,&soundPointer);
        FlushInstructionCache(GetCurrentProcess(),image,0x200000);
        WNDCLASSW cls{}; cls.lpfnWndProc=DefWindowProcW; cls.hInstance=GetModuleHandleW(nullptr); cls.lpszClassName=L"Text control oracle"; RegisterClassW(&cls);
        window=CreateWindowW(cls.lpszClassName,cls.lpszClassName,WS_OVERLAPPEDWINDOW,0,0,800,600,nullptr,nullptr,cls.hInstance,nullptr); require(window!=nullptr,"Diagnostic window failed");
        field<HWND>(graphicsObject.data(),8)=window;
        wchar_t system[MAX_PATH]; GetSystemDirectoryW(system,MAX_PATH); runtime=LoadLibraryW((std::wstring(system)+L"\\d3d8.dll").c_str()); require(runtime!=nullptr,"D3D8 unavailable");
        auto factory=reinterpret_cast<IDirect3D8*(WINAPI*)(UINT)>(GetProcAddress(runtime,"Direct3DCreate8")); d3d=factory(120); require(d3d!=nullptr,"D3D8 SDK120 failed");
        D3DPRESENT_PARAMETERS parameters{}; parameters.BackBufferWidth=800; parameters.BackBufferHeight=600; parameters.BackBufferFormat=D3DFMT_X8R8G8B8; parameters.BackBufferCount=1;
        parameters.SwapEffect=D3DSWAPEFFECT_DISCARD; parameters.Windowed=TRUE; parameters.hDeviceWindow=window;
        const auto controlBefore=_control87(0,0);
        const bool preserve=argc==3;
        require(SUCCEEDED(d3d->CreateDevice(0,D3DDEVTYPE_HAL,window,D3DCREATE_SOFTWARE_VERTEXPROCESSING|(preserve ? D3DCREATE_FPU_PRESERVE:0),&parameters,&device)),"D3D8 device failed");
        if(preserve) require(_control87(0,0)==controlBefore,"Diagnostic FPU precision changed");
        unsigned comparisons=0,directionComparisons=0,scenarios=0,decorationComparisons=0,menuContentComparisons=0,menuFrames=0,editFrames=0,editLetters=0;unsigned long long menuPixels=0;
        {
            FileIndex files(toNarrow(WString(argv[2])),String("pack")); ResourceManager manager(files); resources=&manager;
            textureEnvironment().files=&files; textureEnvironment().load=[&](const void* bytes,const unsigned long& size,IDirect3DTexture8*& texture,TextureImageSize& dimensions) { getImageTexture(device,bytes,size,texture,dimensions); };
            fontFiles()=&files; Settings settings(files,[&](const String& name){return manager.loadSettingsXML(name);}); StringManager strings(files,settings,manager); localization=&strings;

            auto menu=manager.loadSettingsXML(String("UI/MainMenu.xml"));
            auto options=manager.loadSettingsXML(String("UI/Options.xml"));
            for(const char* name:{"UI/MainMenu.xml","UI/FixedBack.xml","UI/Options.xml"}) {
                stage=16;
                // loadSettingsXML deliberately owns the deleting iXML view.
                // Share an existing owner's control block for repeated cached
                // documents rather than creating a second deleting owner.
                auto document=std::strcmp(name,"UI/MainMenu.xml")==0 ? menu:std::strcmp(name,"UI/Options.xml")==0 ? options:manager.loadSettingsXML(String(name));
                ViewRegistry views;ControlRegistry controls;
                auto view=std::make_unique<MenuView>(nullptr,manager,settings,strings,views,controls);
                std::array<unsigned char,56> original{};stage=10;function<void*(__thiscall*)(void*,void*)>(0xfb040)(original.data(),nullptr);
                for(unsigned repeat=0;repeat<2;++repeat) { stage=11;view->loadXML(*document);function<void(__thiscall*)(void*,void*)>(0xfa8c0)(original.data(),wrap(document.get()));
                    stage=12;decorationComparisons+=compareDecorations(*view,original.data()); }
                stage=13;const auto before=textureCounts();view.reset();const auto middle=textureCounts();stage=14;call(0xfafc0,original.data());stage=15;compareReferenceDeltas(before,middle);
            }
            {
                stage=20;menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});
                GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
                std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);
                inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
                ViewRegistry views;ControlRegistry controls;auto owner=std::make_unique<MenuView>(nullptr,manager,settings,strings,views,controls);
                MainMenuContent content(*owner,manager,settings,strings,input,interaction,views,controls);
                std::array<unsigned char,96> nativeMenu{};auto* base=nativeMenu.data()+4;function<void*(__thiscall*)(void*,void*)>(0xfb040)(base,nullptr);
                for(unsigned offset:{0x3cU,0x40U,0x44U,0x4cU,0x58U,0x5cU,0x54U,0x50U,0x48U}) {
                    const bool text=offset==0x58 || offset==0x5c;auto* control=allocate(text ? 696:640);
                    function<void*(__thiscall*)(void*,void*)>(text ? 0xdaca0:0x1e8d0)(control,base);field<void*>(nativeMenu.data(),offset)=control;
                }
                // An existing fixed background exercises 00481990's singleton
                // guard. Its construction/loading is compared independently.
                auto fixedDocument=manager.loadSettingsXML(String("UI/FixedBack.xml"));
                auto fixed=std::make_unique<MenuView>(nullptr,manager,settings,strings,views,controls);fixed->loadXML(*fixedDocument);fixed->show();
                std::array<unsigned char,56> nativeFixed{};function<void*(__thiscall*)(void*,void*)>(0xfb040)(nativeFixed.data(),nullptr);
                function<void(__thiscall*)(void*,void*)>(0xfa8c0)(nativeFixed.data(),wrap(fixedDocument.get()));call(0xd37d0,nativeFixed.data());putPointer(0x25ffec,nativeFixed.data());
                std::array<unsigned char,80> profile{};new(profile.data()+0x24) WString(L"Reference Farmer");putPointer(0x25f738,profile.data());
                // Source name now comes from persisted profiles with the actual
                // game defaults, read through the resource/XML pipeline. Native
                // menu still uses its original-layout name service fixture; full
                // cProfile lifecycle is independently compared by its own oracle.
                wchar_t moduleName[32768]{};require(GetModuleFileNameW(nullptr,moduleName,32768)!=0,"Menu profile diagnostic path missing");
                auto diagnosticPath=(std::filesystem::path(moduleName).parent_path()/L"../../.."/L".local/menu-profile-validation-work").lexically_normal();
                const String diagnosticRoot=toNarrow(WString(diagnosticPath.native()));
                FileIndex::createDirectories(String(std::string(diagnosticRoot.view())+"/"));files.mountCommon(diagnosticRoot);
                String previousProfiles;settings.get(String("game"),String("profiles"),previousProfiles);settings.set(String("game"),String("profiles"),String("MenuProfiles"));
                const auto openProfile=[&](const WString& selected) {
                    return std::make_unique<Profile>(files,settings,selected,[] { initializeGameProfile(profileProperties()); },
                        [&](const String& filename) { return manager.loadSettingsXML(filename); },
                        [&](const String& filename) { manager.releaseResource(filename); });
                };
                auto persistedProfile=openProfile(field<WString>(profile.data(),0x24));unsigned ensureCalls=0;
                const auto* name=&field<WString>(profile.data(),0x24);
                stage=21;content.loadXML(*menu,[&] { ++ensureCalls; },&persistedProfile->headers().currentName());function<void(__thiscall*)(void*,void*)>(0x81e10)(base,wrap(menu.get()));
                require(ensureCalls==1 && owner->children().size()==12,"Main menu content construction differs");
                const std::pair<TextAnimationButton*,unsigned> buttons[]={{content.start,0x3c},{content.exit,0x40},{content.changeUser,0x44},{content.options,0x4c},
                    {content.awards,0x54},{content.credits,0x50},{content.unused,0x48}};
                for(const auto& pair:buttons) { auto* original=field<void*>(nativeMenu.data(),pair.second);compareControl(*pair.first,original);
                    require(unsigned(pair.first->buttonState())==field<unsigned>(original,0x238),"Main menu button state differs");
                    comparisons+=compare(pair.first->text(),field<void*>(original,0x27c));++menuContentComparisons; }
                comparisons+=compare(*content.userName,field<void*>(nativeMenu.data(),0x58));comparisons+=compare(*content.userText,field<void*>(nativeMenu.data(),0x5c));
                for(unsigned repeat=0;repeat<20;++repeat) {
                    const WString value(repeat%2 ? L"Changed Farmer":L"New Profile");field<WString>(profile.data(),0x24)=value;
                    persistedProfile.reset();persistedProfile=openProfile(value);
                    require(persistedProfile->headers().currentName().view()==name->view(),"Persisted menu profile selection differs");
                    if(repeat%3==0) { content.userName->setString(WString());const WString empty;function<void(__thiscall*)(void*,const WString*)>(0xdd030)(field<void*>(nativeMenu.data(),0x58),&empty); }
                    stage=22;content.updateProfileName(&persistedProfile->headers().currentName());call(0x81770,base);content.userName->enabledStep();call(0xdae40,field<void*>(nativeMenu.data(),0x58));
                    comparisons+=compare(*content.userName,field<void*>(nativeMenu.data(),0x58));++menuContentComparisons;
                }
                for(unsigned frame=0;frame<48;++frame) {
                    stage=24;action=frame;menuMilliseconds+=frame<24 ? 17*(1+frame%3):50;
                    menuInputEvents.clear();menuMouseDelta={};menuPoint={0,0};
                    if(frame<24) {
                        const auto& geometry=buttons[frame/4].first->geometry();
                        if(frame%4!=3) menuPoint={geometry.x,geometry.y};
                        if(frame%4==0 || frame%4==3) menuMouseDelta={1,0,0};
                        if(frame%4==1) menuInputEvents={0x208};
                        if(frame%4==2) menuInputEvents={0x200};
                    }
                    if(frame==24) { owner->hideTransparently();call(0xd4980,base); }
                    if(frame==36) { owner->showTransparently();call(0xd4590,base); }
                    try { menuPixels+=compareMenuFrame(d3d,device,window,views,controls,input);++menuFrames; }
                    catch(...) {
                        std::cerr<<"Native transition count "<<field<unsigned>(image,0x260528)<<" dispatch "<<unsigned(image[0x2604f8])<<" normal count "<<field<unsigned>(image,0x260510)<<" source "<<views.enabled().size()<<'\n';
                        for(unsigned i=0;i<6;++i) { auto& text=buttons[i].first->text();auto* native=field<void*>(field<void*>(nativeMenu.data(),buttons[i].second),0x27c);
                            std::cerr<<"Button "<<i<<" text y "<<text.geometry().y<<'/'<<field<float>(native,0x60)<<" hover "<<text.hovered()<<'/'<<unsigned(field<unsigned char>(native,0x2b8))<<" moving "<<text.moving()<<'/'<<unsigned(field<unsigned char>(native,0x2c4))<<" dirty "<<text.textDirty()<<'/'<<unsigned(field<unsigned char>(native,0x254))<<" enable "<<text.state().enabled<<text.state().parentEnabled<<'/'<<unsigned(field<unsigned char>(native,0x26))<<unsigned(field<unsigned char>(native,0x27))<<'\n'; }
                        throw;
                    }
                    for(unsigned i=0;i<6;++i) require(unsigned(buttons[i].first->buttonState())==field<unsigned>(field<void*>(nativeMenu.data(),buttons[i].second),0x238),"Menu input button state differs");
                    if(frame<24) require(buttons[frame/4].first->buttonState()==(frame%4==1 ? ButtonState::Pressed:frame%4==3 ? ButtonState::Normal:ButtonState::Hovered),"Menu input did not produce hover/press/release/leave state");
                    if(frame==35) require(!owner->state().shown && owner->state().transition==0 && field<unsigned>(base,0x10)==0,"Menu hide transition did not finish");
                    if(frame==47) require(owner->state().shown && owner->state().transition==0 && field<unsigned>(base,0x10)==0,"Menu show transition did not finish");
                }
                stage=23;const auto before=textureCounts();fixed.reset();owner.reset();const auto middle=textureCounts();
                call(0xfafc0,nativeFixed.data());call(0xfafc0,base);compareReferenceDeltas(before,middle);
                putPointer(0x25ffec,nullptr);putPointer(0x25f738,nullptr);field<WString>(profile.data(),0x24).~WString();
                persistedProfile.reset();settings.set(String("game"),String("profiles"),previousProfiles);
                menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
            }
            const char* names[]={"StartButton","CreditsButton","Options","ExitButton","AwardsButton","ChangeUserButton","ContinueButton","SubmenuStart","SubmenuCancel"};
            std::mt19937 random(0x584d4c42);
            {
                auto select=manager.loadSettingsXML(String("UI/SelectUser.xml"));XmlNode* node=nullptr;require(select->section(String("NameEdit"),node),"Shipped profile NameEdit missing");
                bool oldRtl=false;settings.get(String("game"),String("RightToLeftEnable"),oldRtl);
                for(trial=0;trial<12;++trial) {
                    stage=25;rtl=trial>=6;editLanguage=trial%2 ? 0x040d:0x0405;settings.set(String("game"),String("RightToLeftEnable"),rtl);
                    ViewRegistry views;ControlRegistry controls;auto box=std::make_unique<EditBox>(nullptr,manager,settings,strings,input,views,controls);
                    box->keyboardLanguage=[] { return editLanguage; };auto* nativeBox=allocate(732);function<void*(__thiscall*)(void*,void*)>(0x1213c0)(nativeBox,nullptr);
                    const auto before=textureCounts();box->loadXML(*node);const auto middle=textureCounts();function<void(__thiscall*)(void*,void*)>(0x121860)(nativeBox,wrap(node));compareReferenceDeltas(before,middle);
                    box->confirm=trial%3 ? &editConfirm:nullptr;box->changed=&actualEditChanged;
                    field<void*>(nativeBox,0x2bc)=trial%3 ? reinterpret_cast<void*>(&editConfirm):nullptr;field<void*>(nativeBox,0x2c0)=reinterpret_cast<void*>(&nativeEditChanged);
                    actualEditEvents.clear();nativeEditEvents.clear();
                    const unsigned alignment=(trial/2)%3;box->setAlignment(alignment);function<void(__thiscall*)(void*,const unsigned*)>(0x15cf0)(nativeBox,&alignment);
                    const wchar_t values[]={L'A',L'B',L' ',L'~',L'!',8,13,10,L'\u017e',L'\u05d0',L'\u05d1',L'\u00a0',L'\u65e5',0,8,8};
                    for(action=0;action<200;++action) {
                        stage=26;
                        if(action%17==0) { box->disable();call(0xd3750,nativeBox);require(input.keyboard().empty() && nativeInput.keyboard().empty(),"Disabled EditBox retained character binding");box->enable();call(0xd3bb0,nativeBox); }
                        const wchar_t value=values[action%std::size(values)];input.dispatchCharacter(value);nativeInput.dispatchCharacter(value);
                        require(actualEditEvents==nativeEditEvents,"EditBox accepted character callback sequence differs");
                        elapsed=action%4==0 ? 0.3f:action%4==1 ? 0.000001f:action%4==2 ? 0.8f:0.f;
                        box->enabledStep();call(0x121f00,nativeBox);box->makeLetters();call(0x122320,nativeBox);box->process();call(0x122860,nativeBox);
                        editLetters+=compare(*box,nativeBox);auto* caret=field<void*>(nativeBox,0x2d4);require(caret && box->caret(),"EditBox owned caret missing");
                        compareControl(*box,nativeBox);compareControl(*const_cast<Letter*>(box->caret()),caret);
                        require(box->rightInput()==bool(field<unsigned char>(nativeBox,0x2b8)) && box->caretShown()==bool(field<unsigned char>(nativeBox,0x2cc)),"EditBox direction/blink state differs");
                        compareFloat(box->maximumWidth(),field<float>(nativeBox,0x2c4),"EditBox maximum width");compareFloat(box->blinkingPeriod(),field<float>(nativeBox,0x2c8),"EditBox period");
                        compareFloat(box->blinkTime(),field<float>(nativeBox,0x2d0),"EditBox elapsed");compareFloat(box->caretAdvance(),field<float>(nativeBox,0x2d8),"EditBox underscore advance");
                        require(!std::memcmp(box->caret()->uv().data(),static_cast<unsigned char*>(caret)+0x238,16),"EditBox caret UV differs");
                        const auto& sprite=const_cast<Letter*>(box->caret())->sprite();call(0x10c500,caret);require(!std::memcmp(&sprite,static_cast<unsigned char*>(caret)+0x70,76),"EditBox caret sprite differs");++editFrames;
                    }
                    stage=27;const auto beforeDelete=textureCounts();box.reset();const auto middleDelete=textureCounts();call(0x122c40,nativeBox);compareReferenceDeltas(beforeDelete,middleDelete);
                    require(input.keyboard().empty() && nativeInput.keyboard().empty(),"EditBox destructor retained keyboard handler");
                }
                rtl=oldRtl;settings.set(String("game"),String("RightToLeftEnable"),oldRtl);
            }
            for(trial=0;trial<9;++trial) {
                rtl=bool(trial%2);settings.set(String("game"),String("RightToLeftEnable"),rtl);ViewRegistry views;ControlRegistry controls;
                XmlNode* node=nullptr;require(menu->section(String(names[trial]),node),"Menu button XML missing");
                std::array<unsigned char,0x280> object{};auto* f=object.data();stage=1;
                std::unique_ptr<ButtonPrototype> parent;
                if(trial<6) parent=std::make_unique<TextAnimationButton>(nullptr,manager,settings,strings,input,interaction,views,controls);
                else parent=std::make_unique<TextButton>(nullptr,manager,settings,strings,input,interaction,views,controls);
                function<void*(__thiscall*)(void*,void*)>(trial<6 ? 0x1e8d0:0x11fe90)(f,nullptr);
                auto& line=trial<6 ? static_cast<TextAnimationButton&>(*parent).text():static_cast<TextButton&>(*parent).text();
                auto* nativeLine=field<void*>(f,0x27c);require(parent->children().size()==1,"Text button did not own text child");
                stage=2;parent->loadXML(*node);function<void(__thiscall*)(void*,void*)>(trial<6 ? 0x1f120:0x11f9b0)(f,wrap(node));
                stage=3;compareControl(*parent,f);compareControl(line,nativeLine);comparisons+=compare(line,nativeLine);
                // Exercise nonnull texture switches using a shipped Slider
                // definition as an explicit separate base Button XML pass.
                // MainMenu loader 00481e10 never reads these legacy submenu
                // sections. This exercises the separate TextButton class.
                if(trial>=6) {
                    XmlNode* volume=nullptr;XmlNode* picture=nullptr;
                    require(options->section(String("SoundVolume"),volume) && volume->section(String("Slider"),picture),"Slider picture missing");
                    static_cast<Button&>(*parent).Button::loadXML(*picture);function<void(__thiscall*)(void*,void*)>(0x108560)(f,wrap(picture));
                    parent->show();parent->enable();call(0xd37d0,f);call(0xd3bb0,f);
                }
                for(action=0;action<120;++action) {
                    stage=4;const auto state=ButtonState(random()%6);const auto referencesBefore=textureCounts();parent->setState(state);const auto referencesMiddle=textureCounts();
                    function<void(__thiscall*)(void*,unsigned)>(trial<6 ? 0x1ed10:0x11f5a0)(f,unsigned(state));
                    compareReferenceDeltas(referencesBefore,referencesMiddle);
                    const float alpha=action%3 ? float(random()%1001)/1000:parent->alpha();
                    parent->setAlpha(alpha);function<void(__thiscall*)(void*,const float*)>(0x11f570)(f,&alpha);
                    if(action%7==0) {
                        const wchar_t* values[]={L"",L"A 123 \u017e",L"\u0418\u0413\u0420\u0410",L"AB \u00a0 \u05d0",L"Repeated text"};const WString value(values[(action/7)%5]);
                        if(trial<6) static_cast<TextAnimationButton&>(*parent).setText(value);else static_cast<TextButton&>(*parent).setText(value);
                        function<void(__thiscall*)(void*,const WString*)>(trial<6 ? 0xfe470:0x1202d0)(f,&value);
                    }
                    if(action%5==0) { const Point2 position{float(random()%800),float(random()%600)};parent->setPosition(position);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(f,&position); }
                    elapsed=float(random()%600)/1000;line.enabledStep();call(trial<6 ? 0x1fa40:0x11e040,nativeLine);
                    require(unsigned(parent->buttonState())==field<unsigned>(f,0x238),"Text button state differs");
                    require(line.hovered()==bool(field<unsigned char>(nativeLine,0x2b8)) && line.moving()==bool(field<unsigned char>(nativeLine,0x2c4)),"Owned text motion flags differ");
                    compareControl(*parent,f);compareControl(line,nativeLine);comparisons+=compare(line,nativeLine);
                    parent->updateTransform();call(0xf0b70,f);require(!std::memcmp(&parent->sprite(),f+0x70,76),"Text button background sprite differs");++scenarios;
                }
                stage=5;const auto referencesBefore=textureCounts();parent.reset();const auto referencesMiddle=textureCounts();call(trial<6 ? 0x6bea0:0x11f540,f);
                compareReferenceDeltas(referencesBefore,referencesMiddle);
                require(input.mouse().empty() && nativeInput.mouse().empty(),"Owned button left input bindings");
            }
            for(const auto& pair:fontFixtures) require(field<unsigned>(pair.second->object.data(),4)==1,"Owned text font lifetime differs");
            for(const auto& sound:sounds) require(sound->referenceCount()==0,"Button XML sound lifetime differs");sounds.clear();fontFixtures.clear();xmlNodes.clear();
            field<WString>(image,0x260270).~WString(); textureEnvironment()={}; fontFiles()=nullptr; resources=nullptr; localization=nullptr;
        }
        for(auto* p:allocations) if(!ui_test_heap::owns(p)) std::free(p); allocations.clear(); uiElapsedTime()={};device->Release(); d3d->Release(); DestroyWindow(window); FreeLibrary(runtime); FreeLibrary(reference);
        std::cout<<"{\"scenarios\":"<<scenarios<<",\"letter_comparisons\":"<<comparisons<<",\"direction_comparisons\":"<<directionComparisons
            <<",\"edit_frames\":"<<editFrames<<",\"edit_letters\":"<<editLetters<<",\"lines_destroyed\":9,\"decoration_comparisons\":"<<decorationComparisons<<",\"menu_views_destroyed\":3,\"menu_content_comparisons\":"<<menuContentComparisons<<",\"menu_frames\":"<<menuFrames<<",\"menu_pixels\":"<<menuPixels<<",\"menu_input_frames\":24,\"menu_sound_requests\":"<<menuSoundRequests<<",\"device_flags\":"<<(preserve ? 0x22:0x20)<<",\"x87_control_before\":"<<controlBefore<<",\"x87_control_after\":"<<_control87(0,0)<<"}\n";
    } catch(const std::exception& e) { std::cerr<<e.what()<<" stage "<<stage<<" trial "<<trial<<" action "<<action<<'\n'; return 1; }
      catch(const char* e) { std::cerr<<e<<" stage "<<stage<<" trial "<<trial<<" action "<<action<<'\n'; return 1; }
}
