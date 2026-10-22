// Original owned TextButton/TextAnimationButton menu XML reference only. Implementation libraries
// never load the original PE; CRT/device/service fixtures are explicit adapters.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "monotonic_ui_test_heap.h"
#include "ButtonText.h"
#include "ExtendedTextButton.h"
#include "PriceText.h"
#include "ContractDialogs.h"
#include "ContractProvide.h"
#include "GamePauseMenu.h"
#include "StateView.h"
#include "MovableUnion.h"
#include "BuySeeds.h"
#include "BuyBuilding.h"
#include "ProductionBuilding.h"
#include "Manufacture.h"
#include "ActionArea.h"
#include "Warehouse.h"
#include "WarehouseTip.h"
#include "MenuView.h"
#include "MainMenuContent.h"
#include "GameConfiguration.h"
#include "GameplayEvents.h"
#include <cmath>
#include "EditBox.h"
#include "ListBox.h"
#include "MultilineText.h"
#include "YesNoDialog.h"
#include "SelectProfile.h"
#include "Credits.h"
#include "ExitDialog.h"
#include "CheckBox.h"
#include "Slider.h"
#include "Options.h"
#include "ProgressBar.h"
#include "Histogram.h"
#include "Market.h"
#include "SalesHistory.h"
#include "FlyingText.h"
#include "GameEffects.h"
#include "DigBuilding.h"
#include "WaterSource.h"
#include "MowBuilding.h"
#include "FarmAreas.h"
#include "FarmField.h"
#include "Sprinkle.h"
#include "WaterParticles.h"
#include "WaterEmitters.h"
#include "TooltipFrame.h"
#include "AwardEffect.h"
#include "Awards.h"
#include "AwardPanel.h"
#include "ProfileProperties.h"
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
#include <type_traits>
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
#include <fstream>
using namespace farmar::recovered;
namespace {
unsigned char* image=nullptr;
unsigned stage=0,trial=0,action=0;
bool expectedListFault=false;
float elapsed=0;
bool explicitTipElapsed=false;
bool menuStencil=false;
float currentElapsed() { return !explicitTipElapsed && GameTimer::instance() ? GameTimer::instance()->elapsed():elapsed; }
float __fastcall timer(void*,void*) { return currentElapsed(); }
void __fastcall frameService(void*,void*) {}
DWORD menuMilliseconds=0;
DWORD WINAPI menuClock() { return menuMilliseconds; }
void WINAPI menuSleep(DWORD value) { menuMilliseconds+=value; }
MMRESULT WINAPI menuPeriod(UINT) { return 0; }
GameCursor* menuCursor=nullptr;void* nativeCursorFrame=nullptr;
POINT menuScreen{};Point2 menuPoint{};
BOOL WINAPI menuCursorPosition(LPPOINT output) { *output=menuScreen;return TRUE; }
std::array<unsigned,2> tooltipDimensions{800,600};
unsigned __fastcall tooltipWidth(void*,void*) { return tooltipDimensions[0]; }
unsigned __fastcall tooltipHeight(void*,void*) { return tooltipDimensions[1]; }
float __fastcall tooltipUnscaledTimer(void*,void*) { return GameTimer::instance()->unscaledElapsed(); }
std::vector<unsigned> menuInputEvents;std::array<int,3> menuMouseDelta{};
std::vector<std::pair<std::string,unsigned>> actualSounds,expectedSounds;
unsigned menuSoundRequests=0;
std::set<void*> allocations;
void* pauseLastNativeDialog=nullptr;void* lastNativeAwardPanel=nullptr;void* mainMenuLastNativeExit=nullptr;void* mainMenuLastNativeLoading=nullptr;
void* lastNativeFlyingText=nullptr;std::function<void(void*)> nativeFlyingAllocation;
ResourceManager* resources=nullptr; StringManager* localization=nullptr; bool rtl=false;
template<class T> T& field(void* object,unsigned offset) { return *reinterpret_cast<T*>(static_cast<unsigned char*>(object)+offset); }
template<class T> T function(unsigned rva) { return reinterpret_cast<T>(image+rva); }
void call(unsigned rva,void* object) { function<void(__thiscall*)(void*)>(rva)(object); }
void require(bool result,const char* message) { if(!result) { std::cerr<<message<<'\n';throw std::runtime_error(message); } }
void* __cdecl allocate(unsigned bytes) { auto* p=::operator new(bytes); allocations.insert(p);if(bytes==760) {lastNativeFlyingText=p;if(nativeFlyingAllocation) nativeFlyingAllocation(p);}if(bytes==592) pauseLastNativeDialog=p;if(bytes==596) lastNativeAwardPanel=p;if(bytes==100) mainMenuLastNativeExit=p;if(bytes==196) mainMenuLastNativeLoading=p; return p; }
void __cdecl release(void* p) { allocations.erase(p); if(!ui_test_heap::owns(p)) std::free(p); }
LONG CALLBACK diagnose(EXCEPTION_POINTERS* e) {
    if(e->ExceptionRecord->ExceptionCode==EXCEPTION_ACCESS_VIOLATION) {
        if(expectedListFault) {
            std::cout<<"{\"native_clear_fault\":true,\"source_clear_exception\":true,\"exception_code\":"<<e->ExceptionRecord->ExceptionCode
                <<",\"fault_rva\":"<<(reinterpret_cast<unsigned>(e->ExceptionRecord->ExceptionAddress)-reinterpret_cast<unsigned>(image))<<"}\n"<<std::flush;
            TerminateProcess(GetCurrentProcess(),77);
        }
        std::cerr<<"Text control AV stage "<<stage<<" trial "<<trial<<" action "<<action<<" RVA 0x"<<std::hex
            <<(reinterpret_cast<unsigned>(e->ExceptionRecord->ExceptionAddress)-reinterpret_cast<unsigned>(image))<<" address 0x"<<e->ExceptionRecord->ExceptionInformation[1]<<'\n';
        auto* stack=reinterpret_cast<unsigned*>(e->ContextRecord->Esp);std::cerr<<"EIP "<<e->ContextRecord->Eip<<" ECX "<<e->ContextRecord->Ecx<<" EDX "<<e->ContextRecord->Edx<<" ESI "<<e->ContextRecord->Esi<<" EDI "<<e->ContextRecord->Edi<<" EBX "<<e->ContextRecord->Ebx<<" stack";
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
bool nativeOptionsFull=false;
bool __fastcall getBool(void*,void*,const String* section,const String* key,bool* output) { *output=section->view()=="graphics" && key->view()=="fullscreen" ? nativeOptionsFull:rtl; return true; }
void __fastcall setBool(void*,void*,const String* section,const String* key,const bool* value) { require(section->view()=="graphics" && key->view()=="fullscreen","Options settings write differs");nativeOptionsFull=*value; }
const WString* __fastcall getString(void*,void*,const WString* source) { return &localization->getString(*source); }
std::set<cTexture*> textureResources;
cTexture* __fastcall getTexture(void*,void*,const String* name) { try { auto* texture=resources->getTexture(*name);textureResources.insert(texture);return texture; } catch(const std::exception& e) { std::cerr<<"Texture adapter "<<name->c_str()<<": "<<e.what()<<'\n';throw; } }
bool __fastcall releaseFont(void* font,void*) { return --field<unsigned>(font,4)==0; }
void* __fastcall addFont(void* font,void*) { ++field<unsigned>(font,4);return font; }
String* __fastcall assignNarrow(String* self,void*,const char* source,unsigned count) { return &self->assign(source,count); }
WString* __fastcall assignWide(WString* self,void*,const WString* source,unsigned start,unsigned count) { try { return &self->assign(*source,start,count); } catch(const std::exception& e) { std::cerr<<"assignWide "<<source->size()<<' '<<start<<' '<<count<<' '<<e.what()<<'\n';throw; } }
WString* __fastcall assignUnits(WString* self,void*,const wchar_t* source,unsigned count) { return &self->assign(source,count); }
WString* __fastcall appendString(WString* self,void*,const WString* source,unsigned start,unsigned count) { try { return &self->append(*source,start,count); } catch(const std::exception& e) { std::cerr<<"appendWide "<<source->size()<<' '<<start<<' '<<count<<' '<<e.what()<<'\n';throw; } }
WString* __fastcall appendUnits(WString* self,void*,unsigned count,wchar_t unit) { for(unsigned i=0;i<count;++i) self->append(&unit,1); return self; }
WString* __fastcall replaceUnits(WString* self,void*,unsigned position,unsigned count,const wchar_t* source,unsigned length) {
    try { std::wstring text(self->view()); text.replace(position,count,source,length); return &self->assign(text.data(),unsigned(text.size())); } catch(const std::exception& e) { std::cerr<<"replaceWide "<<self->size()<<' '<<position<<' '<<count<<' '<<length<<' '<<e.what()<<'\n';throw; }
}
struct Link { Link *next,*previous; void* control; };
std::vector<void*> originalLetters(void* object) { auto* head=field<Link*>(object,0x24c); std::vector<void*> result; for(auto* node=head->next;node!=head;node=node->next) result.push_back(node->control); return result; }
void compareFloat(float a,float e,const char* message) { if(std::memcmp(&a,&e,4)) { std::cerr<<message<<" actual "<<std::hexfloat<<a<<" expected "<<e<<'\n'; throw std::runtime_error("Original TextLine float differs"); } }
unsigned compare(TextLine& line,void* object) {
    // Width/height accessors construct dirty letters. Apply the native
    // accessor's virtual makeLetters path before comparing its raw fields.
    const unsigned callerStage=stage;if(callerStage==47) stage=48;
    if(field<unsigned char>(object,0x254)) reinterpret_cast<void(__thiscall*)(void*)>(field<void**>(object,0)[0x11c/4])(object);
    if(callerStage==47) stage=49;
    auto expected=originalLetters(object); if(expected.size()!=line.letters().size()) {std::cerr<<"Letter pool source/native "<<line.letters().size()<<'/'<<expected.size()<<" strings";for(auto c:line.string().view()) std::cerr<<' '<<std::hex<<unsigned(c);std::cerr<<" /";for(auto c:field<WString>(object,0x22c).view()) std::cerr<<' '<<std::hex<<unsigned(c);std::cerr<<std::dec<<'\n';}require(expected.size()==line.letters().size(),"Original TextLine letter pool differs");
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
    stage=callerStage;return at;
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
bool __fastcall sectionText(XmlAdapter* self,void*,const String* name,WString* output) {
    XmlNode* child=nullptr;if(!self->node->section(*name,child)) return false;*output=child->text();return true;
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
        table[4]=reinterpret_cast<void*>(&addFont);table[5]=reinterpret_cast<void*>(&releaseFont); field<void**>(object.data(),0)=table.data();field<unsigned>(object.data(),4)=1;
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
unsigned __cdecl wideLength(const wchar_t* text) { return unsigned(std::wcslen(text)); }
unsigned __fastcall findWide(WString* self,void*,const wchar_t* text,unsigned first,unsigned count) { const auto found=self->view().find(std::wstring_view(text,count),first);return found==std::wstring_view::npos ? WString::npos:unsigned(found); }
int __fastcall compareWide(WString* self,void*,unsigned first,unsigned count,const wchar_t* text,unsigned size) { return self->compare(first,count,text,size); }
std::wostringstream listKey;
void __fastcall listStreamConstruct(void*,void*,void*,int) { listKey.str(L"");listKey.clear(); }
void __fastcall listStreamNoop(void*,void*) {}
void* __cdecl listWriteLiteral(void* self,const wchar_t* text) { listKey<<text;return self; }
void* __fastcall listWriteUnsigned(void* self,void*,unsigned value) { listKey<<value;return self; }
WString* __fastcall listStreamContents(void*,void*,WString* output) { return new(output) WString(listKey.str()); }
WString* __fastcall wideAttribute(XmlAdapter* self,void*,WString* output,const WString* key) { return new(output) WString(self->node->attribute(*key)); }
std::vector<unsigned> actualSelections,nativeSelections;
void actualSelect(unsigned index,ListBox*) { actualSelections.push_back(index); }
void nativeSelect(unsigned index,void*) { nativeSelections.push_back(index); }
const XmlNode* dialogDefinition=nullptr;
const XmlNode* selectorDefinition=nullptr;
const XmlNode* creditsDefinition=nullptr;
const XmlNode* exitDefinition=nullptr;
const XmlNode* optionsDefinition=nullptr;
XmlAdapter* __fastcall dialogXML(void*,void*,const String* name) {
    if(name->view()=="UI/Options.xml" && optionsDefinition) return wrap(optionsDefinition);
    if(name->view()=="UI/YesNoExit.xml" && exitDefinition) return wrap(exitDefinition);
    if(name->view()=="UI/Credits.xml" && creditsDefinition) return wrap(creditsDefinition);
    if(name->view()=="UI/SelectUser.xml" && selectorDefinition) return wrap(selectorDefinition);
    require(name->view()=="UI/YesNo.xml" && dialogDefinition,"Dialog settings XML boundary differs");return wrap(dialogDefinition);
}
void __fastcall releaseDialogXML(void*,void*) {}
std::vector<unsigned> actualDialogEvents,nativeDialogEvents;
YesNoDialog* actualDialog=nullptr;void* nativeDialog=nullptr;
void yesDialog(YesNoDialog* self) { require(self==actualDialog,"Source yes callback object differs");actualDialogEvents.push_back(1); }
void noDialog(YesNoDialog* self) { require(self==actualDialog,"Source no callback object differs");actualDialogEvents.push_back(2); }
void yesNativeDialog(void* self) { require(self==nativeDialog,"Native yes callback object differs");nativeDialogEvents.push_back(1); }
void noNativeDialog(void* self) { require(self==nativeDialog,"Native no callback object differs");nativeDialogEvents.push_back(2); }
std::map<void*,std::string> nativeDiscards;
void __cdecl nativeDiscard(void* view,const String* origin) { nativeDiscards[view]=std::string(origin->view()); }
std::map<void*,String> nativeDeferredStrings;
String* __fastcall deferredString(void* self,void*,void** key) {require(self==image+0x2604e4,"Native deferred registry differs");return &nativeDeferredStrings[*key];}
std::vector<void*> nativeExitRegistrations;
int __cdecl registerOracleExit(void* callback) {nativeExitRegistrations.push_back(callback);return 0;}
String* __fastcall decimalString(String* output,void*,unsigned value) { return new(output) String(std::to_string(value)); }
String* __cdecl concatenateString(String* output,const String* a,const String* b) { return new(output) String(std::string(a->view())+std::string(b->view())); }
#include "select_profile_oracle_services.h"
std::vector<unsigned> actualExitEvents,nativeExitEvents;
ExitDialog* actualExitDialog=nullptr;void* nativeExitDialog=nullptr;
void exitYes(ExitDialog* self) { require(self==actualExitDialog,"Exit yes callback object differs");actualExitEvents.push_back(1); }
void exitNo(ExitDialog* self) { require(self==actualExitDialog,"Exit no callback object differs");actualExitEvents.push_back(2); }
void nativeExitYes(void* self) { require(self==nativeExitDialog,"Native exit yes callback object differs");nativeExitEvents.push_back(1); }
void nativeExitNo(void* self) { require(self==nativeExitDialog,"Native exit no callback object differs");nativeExitEvents.push_back(2); }
CheckBox* actualCheckBox=nullptr;void* nativeCheckBox=nullptr;
std::vector<unsigned> actualCheckEvents,nativeCheckEvents;
void checkClicked(ButtonPrototype* self) { require(self==actualCheckBox,"Checkbox click object differs");actualCheckEvents.push_back(10+actualCheckBox->checked()); }
void nativeCheckClicked(void* self) { require(self==nativeCheckBox,"Native checkbox click object differs");nativeCheckEvents.push_back(10+field<unsigned char>(self,0x284)); }
void checkChanged(CheckBox* self,bool checked) { require(self==actualCheckBox && checked==self->checked(),"Checkbox changed object/value differs");actualCheckEvents.push_back(20+checked); }
void nativeCheckChanged(void* self,bool checked) { require(self==nativeCheckBox && checked==bool(field<unsigned char>(self,0x284)),"Native checkbox changed object/value differs");nativeCheckEvents.push_back(20+checked); }
Slider* actualSlider=nullptr;void* nativeSlider=nullptr;bool mutateSlider=false;
std::vector<std::pair<unsigned,unsigned>> actualSliderEvents,nativeSliderEvents;
unsigned floatBits(float value) { unsigned bits;std::memcpy(&bits,&value,4);return bits; }
void sliderValue(Slider* self,float value) { require(self==actualSlider && floatBits(value)==floatBits(self->value()),"Slider value callback differs");actualSliderEvents.emplace_back(1,floatBits(value));if(mutateSlider) self->setValue(value+7.25f); }
void nativeSliderValue(void* self,float value) { require(self==nativeSlider && floatBits(value)==floatBits(field<float>(self,0x290)),"Native slider value callback differs");nativeSliderEvents.emplace_back(1,floatBits(value));if(mutateSlider) field<float>(self,0x290)=value+7.25f; }
void sliderDrag(Slider* self,bool drag) { require(self==actualSlider && drag==self->dragging(),"Slider drag callback differs");actualSliderEvents.emplace_back(2,drag); }
void nativeSliderDrag(void* self,bool drag) { require(self==nativeSlider && drag==bool(field<unsigned char>(self,0x284)),"Native slider drag callback differs");nativeSliderEvents.emplace_back(2,drag); }
struct OptionsAudioFixture { bool disabled=false;unsigned sound=100,music=100;std::vector<std::pair<unsigned,unsigned>> events; };
OptionsAudioFixture actualOptionsAudio,nativeOptionsAudio;
bool __fastcall optionsDisabled(void*,void*) { nativeOptionsAudio.events.emplace_back(0,nativeOptionsAudio.disabled);return nativeOptionsAudio.disabled; }
unsigned __fastcall optionsSoundVolume(void*,void*) { nativeOptionsAudio.events.emplace_back(1,nativeOptionsAudio.sound);return nativeOptionsAudio.sound; }
unsigned __fastcall optionsMusicVolume(void*,void*) { nativeOptionsAudio.events.emplace_back(2,nativeOptionsAudio.music);return nativeOptionsAudio.music; }
void __fastcall optionsSetSound(void*,void*,unsigned value) { nativeOptionsAudio.events.emplace_back(3,value);nativeOptionsAudio.sound=value; }
void __fastcall optionsSetMusic(void*,void*,unsigned value) { nativeOptionsAudio.events.emplace_back(4,value);nativeOptionsAudio.music=value; }
std::vector<std::pair<bool,bool>> actualOptionsResets,nativeOptionsResets;
void __fastcall optionsReset(void*,void*,bool full) { nativeOptionsResets.emplace_back(full,nativeOptionsFull); }
void compareControl(Control& control,void* object) {
    compareFloat(control.geometry().x,field<float>(object,0x5c),"control x");compareFloat(control.geometry().y,field<float>(object,0x60),"control y");
    compareFloat(control.geometry().width,field<float>(object,0x64),"control width");compareFloat(control.geometry().height,field<float>(object,0x68),"control height");
    compareFloat(control.deep(),field<float>(object,0x228),"control depth");compareFloat(control.alpha(),*field<float*>(object,0x58),"control alpha");
    compareFloat(control.animation().time,field<float>(object,0x1ac),"control time");
    require(control.texture()==field<cTexture*>(object,0x30),"Control texture differs");
    if(control.state().shown!=bool(field<unsigned char>(object,0x24)) || control.state().parentShown!=bool(field<unsigned char>(object,0x25)) || control.state().enabled!=bool(field<unsigned char>(object,0x26)) || control.state().parentEnabled!=bool(field<unsigned char>(object,0x27))) {
        std::cerr<<"Control activation "<<control.geometry().x<<','<<control.geometry().y<<" source "<<control.state().shown<<control.state().parentShown<<control.state().enabled<<control.state().parentEnabled<<" native "<<unsigned(field<unsigned char>(object,0x24))<<unsigned(field<unsigned char>(object,0x25))<<unsigned(field<unsigned char>(object,0x26))<<unsigned(field<unsigned char>(object,0x27))<<'\n';
    }
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
        require(!std::memcmp(&actual[i]->sprite(),static_cast<unsigned char*>(expected[i])+0x70,76),"Menu decoration sprite differs");
        if(auto* line=dynamic_cast<TextLine*>(actual[i])) compare(*line,expected[i]);
        if(auto* text=dynamic_cast<MultilineText*>(actual[i])) {
            require(text->string().view()==field<WString>(expected[i],0x23c).view(),"Menu multiline string differs");
            auto* head=field<Link*>(expected[i],0x230);auto* node=head->next;
            require(text->lines().size()==field<unsigned>(expected[i],0x234),"Menu multiline row count differs");
            for(auto* line:text->lines()) { require(node!=head,"Menu multiline list ended early");compareControl(*line,node->control);compare(*line,node->control);node=node->next; }
            require(node==head,"Menu multiline list ended late");
        }
    }
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
unsigned long long compareMenuFrame(IDirect3D8* d3d,IDirect3DDevice8* device,HWND window,ViewRegistry& views,ControlRegistry& controls,InputBindings& input,unsigned minimumContent=100000) {
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
    if(actualSounds!=expectedSounds) {std::cerr<<"Actual menu sounds";for(const auto& item:actualSounds) std::cerr<<char(32)<<item.first<<char(58)<<item.second;std::cerr<<"\nNative menu sounds";for(const auto& item:expectedSounds) std::cerr<<char(32)<<item.first<<char(58)<<item.second;std::cerr<<"\n";}
    require(actualSounds==expectedSounds,"Menu input sound request order differs");
    menuSoundRequests+=unsigned(actualSounds.size());
    if(actualSubmissions.size()!=nativeSubmissions.size()) {std::cerr<<"submissions source/native "<<actualSubmissions.size()<<'/'<<nativeSubmissions.size()<<'\n';for(const auto& s:actualSubmissions) std::cerr<<"source "<<(s.texture ? s.texture->resourceName().c_str():"null")<<'\n';for(const auto& s:nativeSubmissions) std::cerr<<"native "<<(s.texture ? s.texture->resourceName().c_str():"null")<<'\n';}
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
        // PictureButton exercises stencil writes and tests. Both renderings
        // must start with the same device state, including states the ordinary
        // no-stencil menus leave untouched. These writes are outside tracing.
        device->SetRenderState(D3DRS_STENCILFUNC,8);device->SetRenderState(D3DRS_STENCILREF,0);
        device->SetRenderState(D3DRS_STENCILMASK,0xffffffffU);device->SetRenderState(D3DRS_STENCILWRITEMASK,0xffffffffU);
        device->SetRenderState(D3DRS_STENCILFAIL,1);device->SetRenderState(D3DRS_STENCILZFAIL,1);device->SetRenderState(D3DRS_STENCILPASS,1);
        device->SetRenderState(D3DRS_ALPHAREF,0);
        device->SetTextureStageState(0,D3DTSS_MAGFILTER,1);device->SetTextureStageState(0,D3DTSS_MINFILTER,1);
        require(SUCCEEDED(device->Clear(0,nullptr,D3DCLEAR_TARGET|(menuStencil ? D3DCLEAR_STENCIL:0),0xff17293b,0,0)),"Menu clear failed");
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
    if(minimumContent) require(nonbackground>minimumContent,"Whole menu has no substantial rendered content");return actual.size()/4;
}

}
#include "loading_oracle_services.h"
#include "awards_oracle_services.h"
#include "resource_log_oracle_services.h"
#include "main_menu_oracle_services.h"
#include "game_configuration_oracle_services.h"
#include "gameplay_oracle_services.h"
#include "gameplay_runtime_oracle_services.h"
#include "upgrade_oracle_services.h"
#include "contract_status_oracle_services.h"
#include "contract_provide_oracle_services.h"
#include "game_pause_oracle_services.h"
#include "game_menu_oracle_services.h"
#include "tutorial_tips_oracle_services.h"
#include "tutorial_manager_oracle_services.h"
#include "buy_seeds_oracle_services.h"
#include "buy_building_oracle_services.h"
#include "production_house_oracle_services.h"
#include "action_area_oracle_services.h"
#include "warehouse_oracle_services.h"
#include "market_oracle_services.h"
#include "dig_building_oracle_services.h"
#include "water_source_oracle_services.h"
#include "mow_building_oracle_services.h"
#include "sprinkle_oracle_services.h"
#include "farm_field_oracle_services.h"
int __cdecl manufactureMoney() {return contractNativeProfile->ints["Money"];}
int wmain(int argc,wchar_t** argv) {
    if(argc!=3 && argc!=4) {std::cerr<<"Reference diagnostic argument count "<<argc<<'\n';return 2;}
    HMODULE reference=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES),runtime=nullptr; HWND window=nullptr;
    IDirect3D8* d3d=nullptr; IDirect3DDevice8* device=nullptr;
    if(!reference) {std::cerr<<"Reference image mapping failed, Win32 error "<<GetLastError()<<" diagnostic base "<<GetModuleHandleW(nullptr)<<'\n';return 2;}
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
        redirect(0x17ffc0,reinterpret_cast<void*>(&std::memmove)); // diagnostic CRT boundary for vector growth
        redirect(0x19c0,reinterpret_cast<void*>(&assignNarrow)); redirect(0x17c0,reinterpret_cast<void*>(&assignWide)); redirect(0x1ab0,reinterpret_cast<void*>(&assignUnits));
        redirect(0x16d0,reinterpret_cast<void*>(&assignString));redirect(0xda20,reinterpret_cast<void*>(&narrow));redirect(0xdb50,reinterpret_cast<void*>(&fromNarrow));
        redirect(0xd95c0,reinterpret_cast<void*>(&typed<float>));redirect(0xd9670,reinterpret_cast<void*>(&typed<bool>));
        redirect(0xd97d0,reinterpret_cast<void*>(&typed<unsigned>));redirect(0xd9720,reinterpret_cast<void*>(&typed<unsigned>));redirect(0xd94a0,reinterpret_cast<void*>(&typed<unsigned>));
        redirect(0xd9240,reinterpret_cast<void*>(&vectorValue<2>));redirect(0xd9180,reinterpret_cast<void*>(&vectorValue<3>));redirect(0xd92e0,reinterpret_cast<void*>(&vectorValue<4>));redirect(0xd93b0,reinterpret_cast<void*>(&colorValue));
        redirect(0xd9dc0,reinterpret_cast<void*>(&appendString)); redirect(0x1600,reinterpret_cast<void*>(&appendUnits)); redirect(0xda980,reinterpret_cast<void*>(&replaceUnits));
        redirect(0x1c10,reinterpret_cast<void*>(&substring));
        redirect(0x1802fd,reinterpret_cast<void*>(&wideLength));redirect(0xda320,reinterpret_cast<void*>(&findWide));
        redirect(0x1050,reinterpret_cast<void*>(&compareWide));
        redirect(0x6a20,reinterpret_cast<void*>(&listStreamConstruct));redirect(0x4ff0,reinterpret_cast<void*>(&listStreamNoop));
        redirect(0xae240,reinterpret_cast<void*>(&listWriteLiteral));redirect(0x6400,reinterpret_cast<void*>(&listWriteUnsigned));
        redirect(0xa430,reinterpret_cast<void*>(&listStreamContents));redirect(0x5710,reinterpret_cast<void*>(&listStreamNoop));redirect(0x17f6f0,reinterpret_cast<void*>(&listStreamNoop));
        for(unsigned rva:{0x26025c,0x2604fc,0x2605fc,0x260508,0x260514,0x260520,0x26052c,0x260538,0x260544,0x260550}) initializeSet(rva);
        auto* discarded=static_cast<unsigned char*>(allocate(48)); std::memset(discarded,0,48); for(unsigned offset:{0U,4U,8U}) std::memcpy(discarded+offset,&discarded,4); discarded[44]=discarded[45]=1; putPointer(0x2604e8,discarded);
        std::array<void*,21> settingsTable{}; settingsTable[10]=reinterpret_cast<void*>(&getBool);settingsTable[20]=reinterpret_cast<void*>(&setBool); auto* settingsPointer=settingsTable.data(); putPointer(0x25f728,&settingsPointer);
        std::array<void*,19> managerTable{}; managerTable[1]=reinterpret_cast<void*>(&getTexture);managerTable[2]=reinterpret_cast<void*>(&getFont);managerTable[3]=reinterpret_cast<void*>(&getSound);managerTable[6]=reinterpret_cast<void*>(&dialogXML); auto* managerPointer=managerTable.data(); putPointer(0x25f730,&managerPointer);
        std::array<void*,2> stringTable{}; stringTable[1]=reinterpret_cast<void*>(&getString); auto* stringPointer=stringTable.data(); putPointer(0x26026c,&stringPointer);
        // Bypass only static string initialization/atexit; original direction
        // algorithm and original reverse routine execute on ABI String objects.
        new(image+0x260270) WString(); *reinterpret_cast<unsigned*>(image+0x26028c)=1;
        std::array<void*,4> timerTable{};timerTable[1]=reinterpret_cast<void*>(&frameService);timerTable[2]=reinterpret_cast<void*>(&timer);auto* timerPointer=timerTable.data();putPointer(0x25ff50,&timerPointer);
        uiElapsedTime()=[] { return currentElapsed(); };
        xmlTable[4]=reinterpret_cast<void*>(&section);xmlTable[5]=reinterpret_cast<void*>(&indexedSection);xmlTable[31]=reinterpret_cast<void*>(&attribute);xmlTable[38]=reinterpret_cast<void*>(&sectionCount);
        xmlTable[32]=reinterpret_cast<void*>(&wideAttribute);
        xmlTable[15]=reinterpret_cast<void*>(&sectionText);
        xmlTable[0]=reinterpret_cast<void*>(&releaseDialogXML);
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
        std::array<void*,9> soundTable{};soundTable[1]=reinterpret_cast<void*>(&nativeHoverSound);soundTable[2]=reinterpret_cast<void*>(&nativeClickSound);soundTable[4]=reinterpret_cast<void*>(&optionsDisabled);soundTable[5]=reinterpret_cast<void*>(&optionsSoundVolume);soundTable[6]=reinterpret_cast<void*>(&optionsSetSound);soundTable[7]=reinterpret_cast<void*>(&optionsMusicVolume);soundTable[8]=reinterpret_cast<void*>(&optionsSetMusic);auto* soundPointer=soundTable.data();putPointer(0x25f734,&soundPointer);
        FlushInstructionCache(GetCurrentProcess(),image,0x200000);
        WNDCLASSW cls{}; cls.lpfnWndProc=DefWindowProcW; cls.hInstance=GetModuleHandleW(nullptr); cls.lpszClassName=L"Text control oracle"; RegisterClassW(&cls);
        window=CreateWindowW(cls.lpszClassName,cls.lpszClassName,WS_OVERLAPPEDWINDOW,0,0,800,600,nullptr,nullptr,cls.hInstance,nullptr); require(window!=nullptr,"Diagnostic window failed");
        field<HWND>(graphicsObject.data(),8)=window;
        wchar_t system[MAX_PATH]; GetSystemDirectoryW(system,MAX_PATH); runtime=LoadLibraryW((std::wstring(system)+L"\\d3d8.dll").c_str()); require(runtime!=nullptr,"D3D8 unavailable");
        auto factory=reinterpret_cast<IDirect3D8*(WINAPI*)(UINT)>(GetProcAddress(runtime,"Direct3DCreate8")); d3d=factory(120); require(d3d!=nullptr,"D3D8 SDK120 failed");
        D3DPRESENT_PARAMETERS parameters{}; parameters.BackBufferWidth=800; parameters.BackBufferHeight=600; parameters.BackBufferFormat=D3DFMT_X8R8G8B8; parameters.BackBufferCount=1;
        parameters.SwapEffect=D3DSWAPEFFECT_DISCARD; parameters.Windowed=TRUE; parameters.hDeviceWindow=window;
        const auto controlBefore=_control87(0,0);
        const bool nativeBug=argc==4 && std::wcscmp(argv[3],L"clear-fault")==0;
        const bool controllers=argc==4 && (std::wcscmp(argv[3],L"controllers")==0 || std::wcscmp(argv[3],L"controllers-device")==0);
        const bool profileControllers=argc==4 && (std::wcscmp(argv[3],L"profiles")==0 || std::wcscmp(argv[3],L"profiles-device")==0);
        const bool creditsControllers=argc==4 && (std::wcscmp(argv[3],L"credits")==0 || std::wcscmp(argv[3],L"credits-device")==0);
        const bool exitControllers=argc==4 && (std::wcscmp(argv[3],L"exit")==0 || std::wcscmp(argv[3],L"exit-device")==0);
        const bool widgets=argc==4 && (std::wcscmp(argv[3],L"widgets")==0 || std::wcscmp(argv[3],L"widgets-device")==0);
        const bool optionsControllers=argc==4 && (std::wcscmp(argv[3],L"options")==0 || std::wcscmp(argv[3],L"options-device")==0);
        const bool flyingMarketMode=argc==4 && (std::wcscmp(argv[3],L"flying-market")==0 || std::wcscmp(argv[3],L"flying-market-device")==0);
        const bool salesHistoryMode=flyingMarketMode || argc==4 && (std::wcscmp(argv[3],L"sales-history")==0 || std::wcscmp(argv[3],L"sales-history-device")==0);
        const bool marketMode=salesHistoryMode || argc==4 && (std::wcscmp(argv[3],L"market")==0 || std::wcscmp(argv[3],L"market-device")==0);
        const bool warehouseTipMode=marketMode || argc==4 && (std::wcscmp(argv[3],L"warehouse-tip")==0 || std::wcscmp(argv[3],L"warehouse-tip-device")==0);
        const bool mowBuildingMode=argc==4 && (std::wcscmp(argv[3],L"mow-building")==0 || std::wcscmp(argv[3],L"mow-building-device")==0);
        const bool farmFieldMode=argc==4 && (std::wcscmp(argv[3],L"farm-field")==0 || std::wcscmp(argv[3],L"farm-field-device")==0);
        const bool sprinkleMode=farmFieldMode || argc==4 && (std::wcscmp(argv[3],L"sprinkle")==0 || std::wcscmp(argv[3],L"sprinkle-device")==0);
        const bool waterSourceMode=sprinkleMode || argc==4 && (std::wcscmp(argv[3],L"water-source")==0 || std::wcscmp(argv[3],L"water-source-device")==0);
        const bool digBuildingMode=argc==4 && (std::wcscmp(argv[3],L"dig-building")==0 || std::wcscmp(argv[3],L"dig-building-device")==0);
        const bool warehouseMode=mowBuildingMode || waterSourceMode || digBuildingMode || warehouseTipMode || argc==4 && (std::wcscmp(argv[3],L"warehouse")==0 || std::wcscmp(argv[3],L"warehouse-device")==0);
        const bool actionAreaMode=argc==4 && (std::wcscmp(argv[3],L"action-area")==0 || std::wcscmp(argv[3],L"action-area-device")==0);
        const bool manufactureMode=argc==4 && (std::wcscmp(argv[3],L"manufacture")==0 || std::wcscmp(argv[3],L"manufacture-device")==0);
        const bool productionHouses=warehouseMode || actionAreaMode || manufactureMode || argc==4 && (std::wcscmp(argv[3],L"production-houses")==0 || std::wcscmp(argv[3],L"production-houses-device")==0);
        if(productionHouses) {menuStencil=true;parameters.EnableAutoDepthStencil=TRUE;parameters.AutoDepthStencilFormat=D3DFMT_D24S8;}
        const bool buyBuildingMode=argc==4 && (std::wcscmp(argv[3],L"buy-building")==0 || std::wcscmp(argv[3],L"buy-building-device")==0);
        if(buyBuildingMode) {menuStencil=true;parameters.EnableAutoDepthStencil=TRUE;parameters.AutoDepthStencilFormat=D3DFMT_D24S8;}
        const bool buySeedsMode=argc==4 && (std::wcscmp(argv[3],L"buy-seeds")==0 || std::wcscmp(argv[3],L"buy-seeds-device")==0);
        const bool movableUnions=argc==4 && (std::wcscmp(argv[3],L"movable-union")==0 || std::wcscmp(argv[3],L"movable-union-device")==0);
        const bool tutorialManagerMode=argc==4 && (std::wcscmp(argv[3],L"tutorial-manager")==0 || std::wcscmp(argv[3],L"tutorial-manager-device")==0);
        const bool tutorialTips=buyBuildingMode || buySeedsMode || movableUnions || tutorialManagerMode || argc==4 && (std::wcscmp(argv[3],L"tutorial-tips")==0 || std::wcscmp(argv[3],L"tutorial-tips-device")==0);
        const bool gameMenus=productionHouses || tutorialTips || argc==4 && (std::wcscmp(argv[3],L"game-menu")==0 || std::wcscmp(argv[3],L"game-menu-device")==0);
        const bool stateViews=argc==4 && (std::wcscmp(argv[3],L"state-view")==0 || std::wcscmp(argv[3],L"state-view-device")==0);
        const bool gamePauses=argc==4 && (std::wcscmp(argv[3],L"game-pause")==0 || std::wcscmp(argv[3],L"game-pause-device")==0);
        const bool contractProvides=argc==4 && (std::wcscmp(argv[3],L"contract-provide")==0 || std::wcscmp(argv[3],L"contract-provide-device")==0);
        const bool contractDialogs=argc==4 && (std::wcscmp(argv[3],L"contract-dialogs")==0 || std::wcscmp(argv[3],L"contract-dialogs-device")==0);
        const bool moneyTexts=argc==4 && (std::wcscmp(argv[3],L"text-money")==0 || std::wcscmp(argv[3],L"text-money-device")==0);
        const bool priceTexts=moneyTexts || argc==4 && (std::wcscmp(argv[3],L"price-text")==0 || std::wcscmp(argv[3],L"price-text-device")==0);
        const bool contractStatuses=argc==4 && (std::wcscmp(argv[3],L"contract-status")==0 || std::wcscmp(argv[3],L"contract-status-device")==0);
        const bool extendedFieldButtons=argc==4 && (std::wcscmp(argv[3],L"extended-field-buttons")==0 || std::wcscmp(argv[3],L"extended-field-buttons-device")==0);
        const bool extendedPictureButtons=argc==4 && (std::wcscmp(argv[3],L"extended-picture-buttons")==0 || std::wcscmp(argv[3],L"extended-picture-buttons-device")==0);
        if(extendedPictureButtons) {menuStencil=true;parameters.EnableAutoDepthStencil=TRUE;parameters.AutoDepthStencilFormat=D3DFMT_D24S8;}
        const bool extendedAnimationButtons=argc==4 && (std::wcscmp(argv[3],L"extended-animation-buttons")==0 || std::wcscmp(argv[3],L"extended-animation-buttons-device")==0);
        const bool extendedButtons=extendedFieldButtons || extendedPictureButtons || extendedAnimationButtons || argc==4 && (std::wcscmp(argv[3],L"extended-buttons")==0 || std::wcscmp(argv[3],L"extended-buttons-device")==0);
        const bool upgrades=argc==4 && (std::wcscmp(argv[3],L"upgrades")==0 || std::wcscmp(argv[3],L"upgrades-device")==0);
        const bool gameplayRuntime=argc==4 && (std::wcscmp(argv[3],L"gameplay-runtime")==0 || std::wcscmp(argv[3],L"gameplay-runtime-device")==0);
        const bool gameplayDefinitions=argc==4 && (std::wcscmp(argv[3],L"gameplay")==0 || std::wcscmp(argv[3],L"gameplay-device")==0);
        const bool gameConfigurations=argc==4 && (std::wcscmp(argv[3],L"game-configuration")==0 || std::wcscmp(argv[3],L"game-configuration-device")==0);
        const bool mainMenus=argc==4 && (std::wcscmp(argv[3],L"main-menu")==0 || std::wcscmp(argv[3],L"main-menu-device")==0);
        const bool resourceLogs=argc==4 && (std::wcscmp(argv[3],L"resource-log")==0 || std::wcscmp(argv[3],L"resource-log-device")==0);
        const bool awardPanels=argc==4 && (std::wcscmp(argv[3],L"panel")==0 || std::wcscmp(argv[3],L"panel-device")==0);
        const bool awardsControllers=argc==4 && (std::wcscmp(argv[3],L"awards")==0 || std::wcscmp(argv[3],L"awards-device")==0);
        const bool effectControllers=argc==4 && (std::wcscmp(argv[3],L"effect")==0 || std::wcscmp(argv[3],L"effect-device")==0);
        const bool tooltipControllers=argc==4 && (std::wcscmp(argv[3],L"tooltip")==0 || std::wcscmp(argv[3],L"tooltip-device")==0);
        const bool waterEmittersMode=argc==4 && (std::wcscmp(argv[3],L"water-emitters")==0 || std::wcscmp(argv[3],L"water-emitters-device")==0);
        const bool waterParticlesMode=waterEmittersMode || argc==4 && (std::wcscmp(argv[3],L"water-particles")==0 || std::wcscmp(argv[3],L"water-particles-device")==0);
        const bool flyingTextMode=argc==4 && (std::wcscmp(argv[3],L"flying-text")==0 || std::wcscmp(argv[3],L"flying-text-device")==0);
        const bool histogramMode=argc==4 && (std::wcscmp(argv[3],L"histogram")==0 || std::wcscmp(argv[3],L"histogram-device")==0);
        const bool progressControllers=argc==4 && (std::wcscmp(argv[3],L"progress")==0 || std::wcscmp(argv[3],L"progress-device")==0);
        const bool loadingControllers=argc==4 && (std::wcscmp(argv[3],L"loading")==0 || std::wcscmp(argv[3],L"loading-device")==0);
        const bool preserve=(farmFieldMode && std::wcscmp(argv[3],L"farm-field")==0) || (sprinkleMode && std::wcscmp(argv[3],L"sprinkle")==0) || (waterEmittersMode && std::wcscmp(argv[3],L"water-emitters")==0) || (waterParticlesMode && std::wcscmp(argv[3],L"water-particles")==0) || (mowBuildingMode && std::wcscmp(argv[3],L"mow-building")==0) || (waterSourceMode && std::wcscmp(argv[3],L"water-source")==0) || (digBuildingMode && std::wcscmp(argv[3],L"dig-building")==0) || (flyingMarketMode && std::wcscmp(argv[3],L"flying-market")==0) || (flyingTextMode && std::wcscmp(argv[3],L"flying-text")==0) || (salesHistoryMode && std::wcscmp(argv[3],L"sales-history")==0) || (marketMode && std::wcscmp(argv[3],L"market")==0) || (histogramMode && std::wcscmp(argv[3],L"histogram")==0) || (warehouseTipMode && std::wcscmp(argv[3],L"warehouse-tip")==0) || (warehouseMode && std::wcscmp(argv[3],L"warehouse")==0) || (actionAreaMode && std::wcscmp(argv[3],L"action-area")==0) || (extendedFieldButtons && std::wcscmp(argv[3],L"extended-field-buttons")==0) || (manufactureMode && std::wcscmp(argv[3],L"manufacture")==0) || (productionHouses && std::wcscmp(argv[3],L"production-houses")==0) || (extendedPictureButtons && std::wcscmp(argv[3],L"extended-picture-buttons")==0) || (buyBuildingMode && std::wcscmp(argv[3],L"buy-building")==0) || (buySeedsMode && std::wcscmp(argv[3],L"buy-seeds")==0) || (movableUnions && std::wcscmp(argv[3],L"movable-union")==0) || (tutorialManagerMode && std::wcscmp(argv[3],L"tutorial-manager")==0) || (tutorialTips && std::wcscmp(argv[3],L"tutorial-tips")==0) || (gameMenus && std::wcscmp(argv[3],L"game-menu")==0) || (stateViews && std::wcscmp(argv[3],L"state-view")==0) || (gamePauses && std::wcscmp(argv[3],L"game-pause")==0) || (contractProvides && std::wcscmp(argv[3],L"contract-provide")==0) || (contractDialogs && std::wcscmp(argv[3],L"contract-dialogs")==0) || (moneyTexts && std::wcscmp(argv[3],L"text-money")==0) || (priceTexts && std::wcscmp(argv[3],L"price-text")==0) || (contractStatuses && std::wcscmp(argv[3],L"contract-status")==0) || (extendedAnimationButtons && std::wcscmp(argv[3],L"extended-animation-buttons")==0) || (extendedButtons && std::wcscmp(argv[3],L"extended-buttons")==0) || (upgrades && std::wcscmp(argv[3],L"upgrades")==0) || (gameplayRuntime && std::wcscmp(argv[3],L"gameplay-runtime")==0) || (gameplayDefinitions && std::wcscmp(argv[3],L"gameplay")==0) || (gameConfigurations && std::wcscmp(argv[3],L"game-configuration")==0) || (mainMenus && std::wcscmp(argv[3],L"main-menu")==0) || (resourceLogs && std::wcscmp(argv[3],L"resource-log")==0) || (awardPanels && std::wcscmp(argv[3],L"panel")==0) || (awardsControllers && std::wcscmp(argv[3],L"awards")==0) || (effectControllers && std::wcscmp(argv[3],L"effect")==0) || (tooltipControllers && std::wcscmp(argv[3],L"tooltip")==0) || argc==3 || nativeBug || (loadingControllers && std::wcscmp(argv[3],L"loading")==0) || (progressControllers && std::wcscmp(argv[3],L"progress")==0) || (optionsControllers && std::wcscmp(argv[3],L"options")==0) || (widgets && std::wcscmp(argv[3],L"widgets")==0) || (controllers && std::wcscmp(argv[3],L"controllers")==0) || (profileControllers && std::wcscmp(argv[3],L"profiles")==0) || (creditsControllers && std::wcscmp(argv[3],L"credits")==0) || (exitControllers && std::wcscmp(argv[3],L"exit")==0);
        require(SUCCEEDED(d3d->CreateDevice(0,D3DDEVTYPE_HAL,window,D3DCREATE_SOFTWARE_VERTEXPROCESSING|(preserve ? D3DCREATE_FPU_PRESERVE:0),&parameters,&device)),"D3D8 device failed");
        if(preserve) require(_control87(0,0)==controlBefore,"Diagnostic FPU precision changed");
        unsigned comparisons=0,directionComparisons=0,scenarios=0,decorationComparisons=0,menuContentComparisons=0,menuFrames=0,editFrames=0,editLetters=0,copies=0,copyLetters=0,listStates=0,listLetters=0,listBoxes=0,listSelections=0,listXml=0,listFaults=0,multiStates=0,multiLetters=0,multiObjects=0,menuTextBranches=0,dialogStates=0,dialogLetters=0,dialogObjects=0,dialogCallbacks=0,dialogDiscardChecks=0,dialogFrames=0;unsigned long long menuPixels=0,dialogPixels=0;
        unsigned creditsStates=0,creditsDecorations=0,creditsObjects=0,creditsFrames=0,creditsSoundResidues=0;unsigned long long creditsPixels=0;
        unsigned exitStates=0,exitLetters=0,exitObjects=0,exitCallbacks=0,exitFrames=0;unsigned long long exitPixels=0;
        unsigned checkStates=0,checkObjects=0,checkCallbacks=0,checkFrames=0;unsigned long long checkPixels=0;
        unsigned sliderStates=0,sliderObjects=0,sliderCallbacks=0,sliderFrames=0;unsigned long long sliderPixels=0;
        unsigned loadingStates=0,loadingObjects=0,loadingFrames=0,loadingDecorations=0,loadingCompleted=0,loadingMissing=0,loadingRanges=0;unsigned long long loadingPixels=0;
        unsigned mainMenuStates=0,mainMenuLetters=0,mainMenuActions=0,mainMenuFrames=0,mainMenuQuitChecks=0,mainMenuLoadingSteps=0;unsigned long long mainMenuPixels=0;
        unsigned manufactureStates=0,manufactureObjects=0,manufactureFactories=0,manufacturePlacements=0,manufactureRemovals=0,manufacturePurchases=0,manufactureBuilds=0;
        unsigned manufactureFrames=0,manufactureEffects=0,manufactureInvalidSlots=0,manufactureReloads=0;unsigned long long manufacturePixels=0;
        unsigned warehouseTipStates=0,warehouseTipFrames=0,warehouseTipReloads=0;unsigned long long warehouseTipPixels=0;
        unsigned warehouseStates=0,warehouseObjects=0,warehouseTransfers=0,warehouseFrames=0;unsigned long long warehousePixels=0;
        unsigned productionPlacementStates=0;
        unsigned fieldObjectStates=0;
        unsigned actionAreaStates=0,actionAreaObjects=0,actionAreaGoals=0,actionAreaClicks=0;
        unsigned actionAreaFrames=0,actionAreaReloads=0;unsigned long long actionAreaPixels=0;
        unsigned long long productionHousePixels=0;
        unsigned productionHouseStates=0,productionHouseObjects=0,productionHouseActions=0,productionHouseFrames=0;
        unsigned buyBuildingStates=0,buyBuildingStarts=0,buyBuildingFrames=0,buyBuildingChoices=0,buyBuildingLetters=0;
        unsigned buySeedsStates=0,buySeedsStarts=0,buySeedsFrames=0,buySeedsChoices=0,buySeedsLetters=0;
        unsigned movableUnionStates=0,movableUnionMoves=0,movableUnionSteps=0,movableUnionFrames=0,movableUnionReloads=0;
        unsigned tutorialManagerStates=0,tutorialManagerStarts=0,tutorialManagerSteps=0,tutorialManagerPredicates=0,tutorialManagerFrames=0;
        unsigned tutorialTipObjects=0,tutorialTipStates=0,tutorialTipLetters=0,tutorialTipStarts=0,tutorialTipSteps=0,tutorialTipFrames=0;
        unsigned hudStates=0,hudLetters=0,hudFrames=0,hudSteps=0,hudActions=0;unsigned long long hudPixels=0;
        unsigned stateViewStates=0,stateViewMoves=0,stateViewSteps=0,stateViewFrames=0,stateViewReloads=0;unsigned long long stateViewPixels=0;
        unsigned pauseStates=0,pauseLetters=0,pauseFrames=0,pauseActions=0,pauseReloads=0,pauseLoadingSteps=0,pauseSoundResidues=0;unsigned long long pausePixels=0;
        unsigned contractOfferStates=0,contractOfferLetters=0,contractOfferObjects=0,contractOfferStarts=0,contractOfferClicks=0,contractOfferReloads=0,contractOfferFrames=0,contractOfferInputClicks=0;unsigned long long contractOfferPixels=0;
        unsigned contractDialogStates=0,contractDialogLetters=0,contractDialogObjects=0,contractDialogStarts=0,contractDialogClicks=0,contractDialogReloads=0,contractDialogFrames=0,contractDialogInputClicks=0;unsigned long long contractDialogPixels=0;
        unsigned priceStates=0,priceLetters=0,priceObjects=0,priceReloads=0,priceFrames=0;unsigned long long pricePixels=0;
        unsigned contractStates=0,contractObjects=0,contractAssignments=0,contractSteps=0,contractClicks=0,contractReloads=0,contractLetters=0,contractFormats=0,contractFrames=0,contractInputClicks=0;unsigned long long contractPixels=0;
        unsigned extendedStates=0,extendedObjects=0,extendedReloads=0,extendedLetters=0,extendedFrames=0,extendedSteps=0,extendedSoundResidues=0;unsigned long long extendedPixels=0;
        unsigned upgradeStates=0,upgradeQueries=0,upgradeChanges=0,upgradeCosts=0,upgradeObjects=0,upgradeReloads=0,upgradeLevelLoads=0,upgradeSoundCalls=0;
        unsigned marketForecasts=0,marketPriceStates=0;
        unsigned gameplayRuntimeStates=0,gameplayRuntimeSteps=0,gameplayRuntimeObjects=0,gameplayRuntimeEvents=0,gameplayRuntimeRestores=0,gameplayRandomStates=0;
        unsigned gameplayTimeStates=0,gameplayTimelineStates=0,gameplayEventStates=0,gameplayContractStates=0,gameplayPopularityStates=0,gameplayObjects=0,gameplayReloads=0;
        unsigned gameConfigurationStates=0,gameConfigurationObjects=0,gameConfigurationReloads=0,gameConfigurationAppearances=0,gameConfigurationSoundResidues=0;
        unsigned resourceLogStates=0,resourceLogShaders=0;unsigned long long resourceLogUnits=0,resourceLogEntries=0;
        unsigned panelStates=0,panelObjects=0,panelFrames=0,panelLetters=0,panelReloads=0,panelSoundResidues=0,panelSourceCallbacks=0,panelNativeCallbacks=0;unsigned long long panelPixels=0;
        unsigned awardsStates=0,awardsObjects=0,awardsFrames=0,awardsReloads=0,awardsDecorations=0,awardsLetters=0,awardsSoundResidues=0,awardsPanelCallbacks=0;unsigned long long awardsPixels=0;
        unsigned effectStates=0,effectObjects=0,effectFrames=0,effectSteps=0,effectReloads=0,effectSounds=0,effectSoundReloads=0;unsigned long long effectPixels=0;
        unsigned tooltipStates=0,tooltipObjects=0,tooltipFrames=0,tooltipPlacements=0,tooltipReloads=0,tooltipLetters=0;unsigned long long tooltipPixels=0;
        unsigned flyingMarketEffects=0,flyingMarketStates=0;
        unsigned salesHistoryStates=0,salesHistoryRecords=0,salesHistoryPrunes=0;
        unsigned waterEmitterStates=0,waterEmitterObjects=0,waterEmitterCreations=0,waterEmitterFrames=0;unsigned long long waterEmitterPixels=0;
        unsigned waterParticleStates=0,waterParticleObjects=0,waterParticleCopies=0,waterParticleFrames=0;unsigned long long waterParticlePixels=0;
        unsigned mowBuildingStates=0,mowBuildingActions=0,mowBuildingFrames=0;unsigned long long mowBuildingPixels=0;
        unsigned farmFieldStates=0,farmFieldObjects=0,farmFieldActions=0,farmFieldFrames=0;unsigned long long farmFieldPixels=0;
        unsigned sprinkleStates=0,sprinkleObjects=0,sprinkleActions=0,sprinkleFrames=0;unsigned long long sprinklePixels=0;
        unsigned waterSourceStates=0,waterSourceActions=0,waterSourceFrames=0;unsigned long long waterSourcePixels=0;
        unsigned digBuildingStates=0,digBuildingActions=0,digBuildingFrames=0;unsigned long long digBuildingPixels=0;
        unsigned marketStates=0,marketActions=0,marketFrames=0;unsigned long long marketPixels=0;
        unsigned flyingTextStates=0,flyingTextObjects=0,flyingTextFrames=0,flyingTextReloads=0,flyingTextCallbacks=0;unsigned long long flyingTextPixels=0;
        unsigned histogramStates=0,histogramObjects=0,histogramReloads=0,histogramFrames=0;unsigned long long histogramPixels=0;
        unsigned progressStates=0,progressObjects=0,progressFrames=0,progressReloads=0;unsigned long long progressPixels=0;
        unsigned optionsStates=0,optionsObjects=0,optionsLetters=0,optionsEvents=0,optionsFrames=0,optionsResets=0;unsigned long long optionsPixels=0;
        {
            FileIndex files(toNarrow(WString(argv[2])),String("pack")); ResourceManager manager(files); resources=&manager;
            textureEnvironment().files=&files; textureEnvironment().load=[&](const void* bytes,const unsigned long& size,IDirect3DTexture8*& texture,TextureImageSize& dimensions) { getImageTexture(device,bytes,size,texture,dimensions); };
            fontFiles()=&files; Settings settings(files,[&](const String& name){return manager.loadSettingsXML(name);}); StringManager strings(files,settings,manager); localization=&strings;

            if(!waterParticlesMode && !flyingTextMode && !histogramMode && !controllers && !profileControllers && !creditsControllers && !exitControllers && !widgets && !optionsControllers && !progressControllers && !loadingControllers && !tooltipControllers && !effectControllers && !awardsControllers && !awardPanels && !resourceLogs && !mainMenus && !gameConfigurations && !gameplayDefinitions && !gameplayRuntime && !upgrades && !extendedButtons && !contractStatuses && !priceTexts && !contractDialogs && !contractProvides && !gamePauses && !stateViews && !gameMenus) {
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
                if(trial>=6) for(action=0;action<20;++action) {
                    stage=28;const auto beforeCopy=textureCounts();auto clone=std::make_unique<TextButton>(static_cast<TextButton&>(*parent));const auto middleCopy=textureCounts();auto* nativeClone=allocate(640);
                    function<void*(__thiscall*)(void*,void*)>(0x11f100)(nativeClone,f);compareReferenceDeltas(beforeCopy,middleCopy);
                    auto* nativeText=field<void*>(nativeClone,0x27c);compareControl(*clone,nativeClone);compareControl(clone->text(),nativeText);copyLetters+=compare(clone->text(),nativeText);
                    require(clone->parent()==parent->parent() && clone->text().parent()==clone.get(),"Copied text button parent/ownership differs");
                    require(!clone->text().hovered() && !clone->text().moving(),"Copied button text retained source motion flags");
                    const Point2 position{float(10+action*17),float(20+action*9)};clone->setPosition(position);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(nativeClone,&position);
                    clone->show();clone->enable();call(0xd37d0,nativeClone);call(0xd3bb0,nativeClone);const WString text(action%2 ? L"Profile clone":L"AB 123");clone->setText(text);function<void(__thiscall*)(void*,const WString*)>(0x1202d0)(nativeClone,&text);
                    clone->text().enabledStep();call(0x11e040,nativeText);copyLetters+=compare(clone->text(),nativeText);compareControl(*clone,nativeClone);
                    const auto beforeDelete=textureCounts();clone.reset();const auto middleDelete=textureCounts();call(0x11f540,nativeClone);compareReferenceDeltas(beforeDelete,middleDelete);++copies;
                }
                stage=5;const auto referencesBefore=textureCounts();parent.reset();const auto referencesMiddle=textureCounts();call(trial<6 ? 0x6bea0:0x11f540,f);
                compareReferenceDeltas(referencesBefore,referencesMiddle);
                require(input.mouse().empty() && nativeInput.mouse().empty(),"Owned button left input bindings");
            }
            {
                auto document=manager.loadSettingsXML(String("UI/SelectUser.xml"));XmlNode* definition=nullptr;require(document->section(String("UserList"),definition),"UserList missing");
                for(trial=0;trial<12;++trial) {
                    stage=30;ViewRegistry views;ControlRegistry controls;auto box=std::make_unique<ListBox>(nullptr,manager,settings,strings,input,interaction,views,controls);
                    auto* original=allocate(628);function<void*(__thiscall*)(void*,void*)>(0x121100)(original,nullptr);
                    box->selected=&actualSelect;field<void*>(original,0x270)=reinterpret_cast<void*>(&nativeSelect);actualSelections.clear();nativeSelections.clear();
                    stage=31;box->loadXML(*definition);function<void(__thiscall*)(void*,void*)>(0x1206e0)(original,wrap(definition));
                    const auto check=[&] {
                        compareControl(*box,original);compareControl(const_cast<Button&>(box->up()),field<void*>(original,0x248));compareControl(const_cast<Button&>(box->down()),field<void*>(original,0x24c));
                        auto** begin=field<void**>(original,0x23c);auto** end=field<void**>(original,0x240);const unsigned count=begin ? unsigned(end-begin):0;
                        require(box->rows().size()==count && box->firstRow()==field<unsigned>(original,0x268) && box->maxRows()==field<unsigned>(original,0x250),"ListBox rows/page differs");
                        compareFloat(box->interlinear(),field<float>(original,0x264),"ListBox interlinear");require(!std::memcmp(box->topLeft().data(),static_cast<unsigned char*>(original)+0x254,8) && !std::memcmp(box->bottomRight().data(),static_cast<unsigned char*>(original)+0x25c,8),"ListBox bounds differ");
                        auto* prototype=field<void*>(original,0x26c);compareControl(const_cast<TextButton&>(box->prototype()),prototype);listLetters+=compare(const_cast<ButtonText&>(box->prototype().text()),field<void*>(prototype,0x27c));
                        for(unsigned i=0;i<count;++i) { auto* row=box->rows()[i];compareControl(*row,begin[i]);require(row->userData==field<unsigned>(begin[i],0x1b4),"ListBox row index differs");listLetters+=compare(row->text(),field<void*>(begin[i],0x27c)); }
                        require(actualSelections==nativeSelections,"ListBox selection callbacks differ");++listStates;
                    };check();
                    for(action=0;action<25+trial;++action) {
                        stage=32;const WString name(std::wstring(L"Profile ")+std::to_wstring(action));const auto before=textureCounts();box->append(name);const auto middle=textureCounts();function<void(__thiscall*)(void*,const WString*)>(0x1212a0)(original,&name);compareReferenceDeltas(before,middle);check();
                    }
                    for(action=0;action<80;++action) {
                        stage=33;if(action%7<4) { box->scrollDown();function<void(__cdecl*)(void*)>(0x121020)(field<void*>(original,0x24c)); } else { box->scrollUp();function<void(__cdecl*)(void*)>(0x121060)(field<void*>(original,0x248)); }
                        const unsigned index=(action*13)%unsigned(box->rows().size());auto* row=box->rows()[index];row->clickEvent(row);function<void(__cdecl*)(void*)>(0x11ec50)(field<void**>(original,0x23c)[index]);++listSelections;
                        if(action%11==0) { const Point2 position{397.f+action*.75f,295.f-action*.125f};box->setPosition(position);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(original,&position);box->updateList();call(0x11e6d0,original); }
                        check();
                    }
                    if(trial==11 && nativeBug) {
                        stage=35;require(box->firstRow()>0,"ListBox clear fault fixture did not scroll");bool fault=false;const auto before=textureCounts();
                        try { box->clear(); } catch(const std::out_of_range&) { fault=true; }const auto middle=textureCounts();
                        require(fault,"Source scrolled ListBox did not detect out-of-range row");expectedListFault=true;call(0x121080,original);
                        throw std::runtime_error("Original scrolled ListBox clear did not fault");
                    } else {
                    stage=34;while(box->firstRow()) { box->scrollUp();function<void(__cdecl*)(void*)>(0x121060)(field<void*>(original,0x248)); }check();
                    const auto beforeClear=textureCounts();box->clear();const auto middleClear=textureCounts();call(0x121080,original);compareReferenceDeltas(beforeClear,middleClear);check();
                    for(unsigned i=0;i<3;++i) { const WString value(L"After clear");box->append(value);function<void(__thiscall*)(void*,const WString*)>(0x1212a0)(original,&value);check(); }
                    }
                    const auto beforeDelete=textureCounts();box.reset();const auto middleDelete=textureCounts();call(0x11d920,original);compareReferenceDeltas(beforeDelete,middleDelete);++listBoxes;
                    require(input.mouse().empty() && nativeInput.mouse().empty(),"ListBox left input bindings");
                }
                const auto directory=std::filesystem::current_path()/L".local/listbox-xml-fixtures";std::filesystem::create_directories(directory);
                const std::string text="<ListBox Interlinear=\"25\" TopLeft=\"0 0\" BottomRight=\"0 205\"><Strings Count=\"3\" str0=\"Alpha\" str1=\"Beta\" str2=\"Gamma\"/></ListBox>";
                { std::ofstream file(directory/L"indexed.xml",std::ios::binary);file<<text; }
                FileIndex fixtureFiles(toNarrow(WString(directory.wstring())),String("pack"));XmlDocument xml(fixtureFiles,String("indexed.xml"));
                for(unsigned populated=0;populated<2;++populated) {
                    stage=36;ViewRegistry views;ControlRegistry controls;auto box=std::make_unique<ListBox>(nullptr,manager,settings,strings,input,interaction,views,controls);
                    auto* original=allocate(628);function<void*(__thiscall*)(void*,void*)>(0x121100)(original,nullptr);box->loadXML(*definition);function<void(__thiscall*)(void*,void*)>(0x1206e0)(original,wrap(definition));
                    if(populated) for(unsigned i=0;i<5;++i) { const WString value(L"Old row");box->append(value);function<void(__thiscall*)(void*,const WString*)>(0x1212a0)(original,&value); }
                    const auto before=textureCounts();box->loadXML(xml);const auto middle=textureCounts();function<void(__thiscall*)(void*,void*)>(0x1206e0)(original,wrap(&xml));compareReferenceDeltas(before,middle);
                    auto** begin=field<void**>(original,0x23c);auto** end=field<void**>(original,0x240);require(box->rows().size()==unsigned(end-begin) && box->rows().size()==(populated ? 5:0),"ListBox Strings changed logical row count");
                    std::vector<void*> nativeChildren;auto* head=field<Node*>(original,0x18);collectControls(head->parent,nativeChildren);require(box->children().size()==nativeChildren.size(),"ListBox Strings hierarchy count differs");
                    unsigned i=0;for(auto* child:box->children()) { auto* control=static_cast<Control*>(child);auto* native=nativeChildren[i++];compareControl(*control,native);if(auto* textButton=dynamic_cast<TextButton*>(control)) listLetters+=compare(textButton->text(),field<void*>(native,0x27c)); }
                    for(unsigned row=0;row<box->rows().size();++row) { compareControl(*box->rows()[row],begin[row]);listLetters+=compare(box->rows()[row]->text(),field<void*>(begin[row],0x27c)); }
                    const auto beforeDelete=textureCounts();box.reset();const auto middleDelete=textureCounts();call(0x11d920,original);compareReferenceDeltas(beforeDelete,middleDelete);++listXml;
                }
            }
            {
                auto document=manager.loadSettingsXML(String("UI/YesNo.xml"));XmlNode* definition=nullptr;require(document->section(String("Text"),definition),"YesNo Text missing");
                for(trial=0;trial<16;++trial) {
                    rtl=bool(trial&2);settings.set(String("game"),String("RightToLeftEnable"),rtl);
                    stage=40;ViewRegistry views;ControlRegistry controls;auto text=std::make_unique<MultilineText>(nullptr,manager,settings,strings,views,controls);auto* original=allocate(636);
                    function<void*(__thiscall*)(void*,void*)>(0xf5e70)(original,nullptr);text->loadXML(*definition);function<void(__thiscall*)(void*,void*)>(0xf8580)(original,wrap(definition));
                    const bool wrapText=bool(trial&4),localized=bool(trial&8);text->setWrap(wrapText);field<unsigned char>(original,0x278)=wrapText;text->setLocalized(localized);field<unsigned char>(original,0x260)=localized;
                    text->setVerticalAlignment(trial%4);field<unsigned>(original,0x264)=trial%4;
                    const auto check=[&] {
                        stage=45;text->enabledStep();stage=46;call(0xf8e10,original);stage=47;
                        require(text->string().view()==field<WString>(original,0x23c).view() && text->textDirty()==bool(field<unsigned char>(original,0x258)),"Multiline text/dirty differs");
                        require(text->localized()==bool(field<unsigned char>(original,0x260)) && text->wraps()==bool(field<unsigned char>(original,0x278)) && text->verticalAlignment()==field<unsigned>(original,0x264),"Multiline options differ");
                        compareFloat(text->interlinear(),field<float>(original,0x25c),"multiline interlinear");require(!std::memcmp(&text->textColor(),static_cast<unsigned char*>(original)+0x268,16),"Multiline color differs");
                        compareControl(*text,original);compareControl(const_cast<TextLine&>(text->prototype()),field<void*>(original,0x238));
                        auto* head=field<Link*>(original,0x230);auto* node=head->next;require(text->lines().size()==field<unsigned>(original,0x234),"Multiline row count differs");
                        for(auto* line:text->lines()) { require(node!=head,"Multiline row list ended early");compareControl(*line,node->control);multiLetters+=compare(*line,node->control);node=node->next; }require(node==head,"Multiline row list ended late");++multiStates;
                    };check();
                    const wchar_t* values[]={L"",L"One line",L"First/nSecond",L"First\nSecond\n",L"\n\nFinal",L"Alpha Beta Gamma Delta Epsilon Zeta Eta Theta Iota Kappa",L" ALongWordWithoutSpacesButOverWidth",L"ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ",L"a  b    c d",L"Delete profile?",L"Привет/n\x5d0\x5d1\x5d2 ABC 123"};
                    for(action=0;action<110;++action) {
                        stage=41;const WString value(values[action%11]);const bool immediate=action%3==0;
                        text->setString(value,immediate);stage=43;function<void(__thiscall*)(void*,const WString*,bool)>(0xf8ce0)(original,&value,immediate);stage=44;check();
                        if(action%5==0) { const float alpha=float(action%7)*.125f;text->setAlpha(alpha);function<void(__thiscall*)(void*,const float*)>(0xf6a80)(original,&alpha);check(); }
                        if(action%7==0) { const Point2 position{400.f+action*.125f,268.f-action*.375f};text->setPosition(position);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(original,&position);check(); }
                    }
                    stage=42;const auto before=textureCounts();text.reset();const auto middle=textureCounts();call(0xf6130,original);compareReferenceDeltas(before,middle);++multiObjects;
                }
            }
            {
                const auto directory=std::filesystem::current_path()/".local"/"menu-text-fixtures";std::filesystem::create_directories(directory);
                std::ofstream(directory/"decorations.xml")<<R"(<root>
<BackText><Line position="130 110" size="240 40" Font="UI/menu_font.fnt" Texture="UI/menu_font.dds" Multiplier="0.75" String="One line" Localized="0" deep="7"/></BackText>
<BackMultitext><Text position="330 210" size="273 84" Font="UI/menu_font.fnt" Texture="UI/menu_font.dds" Multiplier="0.75" String="First/nSecond/nThird" Localized="0" Interlinear="25" WrapText="1" VerticalAlign="2" deep="11" Color="B37F25E1"/>
<Text position="330 370" size="273 84" Font="UI/menu_font.fnt" Texture="UI/menu_font.dds" Multiplier="0.75" String="Overridden" Localized="0" Interlinear="25" VerticalAlign="1"><Text>Section/ncontent
Third</Text></Text></BackMultitext>
</root>)";
                FileIndex fixtures(toNarrow(WString(directory.wstring())),String("pack"));XmlDocument definition(fixtures,String("decorations.xml"));
                for(trial=0;trial<2;++trial) {
                    rtl=bool(trial);settings.set(String("game"),String("RightToLeftEnable"),rtl);
                    ViewRegistry views;ControlRegistry controls;auto view=std::make_unique<MenuView>(nullptr,manager,settings,strings,views,controls);
                    std::array<unsigned char,56> original{};stage=50;function<void*(__thiscall*)(void*,void*)>(0xfb040)(original.data(),nullptr);
                    for(action=0;action<2;++action) {
                        view->loadXML(definition);function<void(__thiscall*)(void*,void*)>(0xfa8c0)(original.data(),wrap(&definition));menuTextBranches+=compareDecorations(*view,original.data());
                    }
                    const auto before=textureCounts();view.reset();const auto middle=textureCounts();call(0xfafc0,original.data());compareReferenceDeltas(before,middle);
                }
            }
            }
            if(controllers) {
                // The source loads its own resource-backed XML. Native virtual
                // load/release uses an independent parsed shipped tree fixture.
                XmlDocument definition(files,String("UI/YesNo.xml"));dialogDefinition=&definition;
                redirect(0x2fe80,reinterpret_cast<void*>(&decimalString));redirect(0x1cc0,reinterpret_cast<void*>(&concatenateString));redirect(0x7d0b0,reinterpret_cast<void*>(&nativeDiscard));
                for(trial=0;trial<16;++trial) {
                    rtl=bool(trial&2);settings.set(String("game"),String("RightToLeftEnable"),rtl);
                    ViewRegistry views;ControlRegistry controls;auto subject=std::make_unique<View>(nullptr,views);std::array<unsigned char,44> nativeSubject{};
                    function<void*(__thiscall*)(void*,void*)>(0xd55b0)(nativeSubject.data(),nullptr);subject->show();subject->enable();call(0xd37d0,nativeSubject.data());call(0xd3bb0,nativeSubject.data());
                    if(trial&4) { subject->disable();call(0xd3750,nativeSubject.data()); }
                    stage=60;auto dialog=std::make_unique<YesNoDialog>(trial&1 ? subject.get():nullptr,manager,settings,strings,input,interaction,views,controls);
                    auto* original=allocate(592);function<void*(__thiscall*)(void*,void*)>(0x101b60)(original,trial&1 ? nativeSubject.data():nullptr);actualDialog=dialog.get();nativeDialog=original;
                    dialog->setYesEvent(&yesDialog);dialog->setNoEvent(&noDialog);function<void(__thiscall*)(void*,void*)>(0x101430)(original,reinterpret_cast<void*>(&yesNativeDialog));function<void(__thiscall*)(void*,void*)>(0x101480)(original,reinterpret_cast<void*>(&noNativeDialog));
                    const auto check=[&] {
                        stage=61;compareControl(*dialog,original);dialog->updateTransform();call(0xf0b70,original);require(!std::memcmp(&dialog->sprite(),static_cast<unsigned char*>(original)+0x70,76),"Dialog background sprite differs");
                        for(const auto& pair:std::initializer_list<std::pair<TextAnimationButton*,unsigned>>{{&dialog->yesButton(),0x238},{&dialog->noButton(),0x23c}}) {
                            auto* button=field<void*>(original,pair.second);compareControl(*pair.first,button);require(unsigned(pair.first->buttonState())==field<unsigned>(button,0x238),"Dialog button state differs");dialogLetters+=compare(pair.first->text(),field<void*>(button,0x27c));
                        }
                        auto& text=dialog->text();auto* nativeText=field<void*>(original,0x240);text.enabledStep();call(0xf8e10,nativeText);compareControl(text,nativeText);
                        auto* head=field<Link*>(nativeText,0x230);auto* node=head->next;require(text.lines().size()==field<unsigned>(nativeText,0x234),"Dialog text rows differ");
                        for(auto* line:text.lines()) { require(node!=head,"Dialog text list ended early");compareControl(*line,node->control);dialogLetters+=compare(*line,node->control);node=node->next; }require(node==head,"Dialog text list ended late");
                        for(unsigned control:{1U,0x1cU,0x39U}) require(input.controls(control).size()==nativeInput.controls(control).size(),"Dialog key bindings differ");
                        require(subject->state().enabled==bool(field<unsigned char>(nativeSubject.data(),0x26)),"Dialog subject enable differs");require(actualDialogEvents==nativeDialogEvents,"Dialog callback order differs");++dialogStates;
                    };check();
                    for(action=0;action<72;++action) {
                        stage=62;const WString text(action%2 ? L"First/nSecond":L"Delete profile?");dialog->text().setString(text);function<void(__thiscall*)(void*,const WString*,bool)>(0xf8ce0)(field<void*>(original,0x240),&text,false);
                        switch(action%6) {
                        case 0:dialog->disable();call(0xd3750,original);break;
                        case 1:dialog->enable();call(0xd3bb0,original);break;
                        case 2:subject->enable();call(0xd3bb0,nativeSubject.data());dialog->enabledStep();call(0x101590,original);break;
                        case 3:input.dispatchControls({0x1c,0x39,1});nativeInput.dispatchControls({0x1c,0x39,1});break;
                        case 4:cursorPosition={dialog->yesButton().geometry().x,dialog->yesButton().geometry().y};dialog->yesButton().unclicked();call(0xf4620,field<void*>(original,0x238));break;
                        case 5:cursorPosition={dialog->noButton().geometry().x,dialog->noButton().geometry().y};dialog->noButton().unclicked();call(0xf4620,field<void*>(original,0x23c));break;
                        }check();
                    }
                    if(trial==0 || trial==2) {
                        menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
                        std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
                        for(unsigned frame=0;frame<16;++frame) {
                            stage=65;action=frame;menuMilliseconds+=17*(1+frame%3);menuInputEvents.clear();menuMouseDelta={};menuPoint={0,0};
                            if(frame<8) {
                                const auto& button=frame<4 ? dialog->yesButton():dialog->noButton();if(frame%4!=3) menuPoint={button.geometry().x,button.geometry().y};
                                if(frame%4==0 || frame%4==3) menuMouseDelta={1,0,0};if(frame%4==1) menuInputEvents={0x208};if(frame%4==2) menuInputEvents={0x200};
                            }
                            if(frame==8) menuInputEvents={0x1c};if(frame==9) menuInputEvents={0x39};if(frame==10) menuInputEvents={1};
                            if(frame==12) { const WString text(L"First/nSecond/nThird");dialog->text().setString(text);function<void(__thiscall*)(void*,const WString*,bool)>(0xf8ce0)(field<void*>(original,0x240),&text,false); }
                            if(frame==13) { const float alpha=.625f;dialog->setAlpha(alpha);function<void(__thiscall*)(void*,const float*)>(0x13b50)(original,&alpha); }
                            if(frame==14) { const Point2 position{418,310};dialog->setPosition(position);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(original,&position); }
                            dialogPixels+=compareMenuFrame(d3d,device,window,views,controls,input,60000);++dialogFrames;check();
                        }
                        menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
                    }
                    stage=63;require(actualSounds==expectedSounds,"Dialog sound request order differs");actualSounds.clear();expectedSounds.clear();dialogCallbacks+=unsigned(actualDialogEvents.size());actualDialogEvents.clear();nativeDialogEvents.clear();const auto before=textureCounts();dialog.reset();const auto middle=textureCounts();call(0x101970,original);compareReferenceDeltas(before,middle);++dialogObjects;
                    require(subject->state().enabled==bool(field<unsigned char>(nativeSubject.data(),0x26)),"Dialog destruction subject enable differs");subject.reset();call(0xd58a0,nativeSubject.data());
                    for(unsigned control:{1U,0x1cU,0x39U}) require(input.controls(control).empty() && nativeInput.controls(control).empty(),"Dialog destruction retained keyboard bindings");
                }
                // Default callback goes through native String/discard service
                // boundaries; source queue retains the dialog until frame deletion.
                ViewRegistry views;ControlRegistry controls;auto* dialog=new YesNoDialog(nullptr,manager,settings,strings,input,interaction,views,controls);
                auto* original=allocate(592);function<void*(__thiscall*)(void*,void*)>(0x101b60)(original,nullptr);stage=64;
                input.dispatchControls({0x1c,1});nativeInput.dispatchControls({0x1c,1});
                require(views.discarded().size()==1 && nativeDiscards.size()==1 && views.discarded().at(dialog).view()==nativeDiscards.at(original),"Dialog default discard request differs");++dialogDiscardChecks;
                const auto before=textureCounts();views.deleteDiscarded();const auto middle=textureCounts();call(0x101970,original);compareReferenceDeltas(before,middle);++dialogObjects;nativeDiscards.clear();dialogDefinition=nullptr;
                require(views.discarded().empty(),"Dialog source frame deletion retained a discard entry");
                for(unsigned control:{1U,0x1cU,0x39U}) require(input.controls(control).empty() && nativeInput.controls(control).empty(),"Default dialog destruction retained bindings");
            }
            if(profileControllers) {
#include "select_profile_oracle_cases.inc"
            }
            if(creditsControllers) {
#include "credits_oracle_cases.inc"
            }
            if(exitControllers) {
#include "exit_oracle_cases.inc"
            }
            if(widgets) {
#include "widgets_oracle_cases.inc"
#include "slider_oracle_cases.inc"
            }
            if(extendedButtons) {
#include "extended_button_oracle_run.inc"
            }
            if(priceTexts) {
#include "price_text_oracle_cases.inc"
            }
            if(gameMenus) {
#include "game_menu_oracle_cases.inc"
            }
            if(stateViews) {
#include "state_view_oracle_cases.inc"
            }
            if(gamePauses) {
#include "game_pause_oracle_cases.inc"
            }
            if(contractProvides) {
#include "contract_provide_oracle_cases.inc"
            }
            if(contractDialogs) {
#include "contract_dialog_oracle_cases.inc"
            }
            if(contractStatuses) {
#include "contract_status_oracle_cases.inc"
            }
            if(upgrades) {
#include "upgrade_oracle_cases.inc"
            }
            if(gameplayRuntime) {
#include "gameplay_runtime_oracle_cases.inc"
            }
            if(gameplayDefinitions) {
#include "gameplay_oracle_cases.inc"
            }
            if(gameConfigurations) {
#include "game_configuration_oracle_cases.inc"
            }
            if(mainMenus) {
#include "main_menu_oracle_cases.inc"
            }
            if(resourceLogs) {
#include "resource_log_oracle_cases.inc"
            }
            if(loadingControllers) {
#include "loading_oracle_cases.inc"
            }
            if(awardPanels) {
#include "award_panel_oracle_cases.inc"
            }
            if(awardsControllers) {
#include "awards_oracle_cases.inc"
            }
            if(effectControllers) {
#include "award_effect_oracle_cases.inc"
            }
            if(tooltipControllers) {
#include "tooltip_oracle_cases.inc"
            }
            if(waterParticlesMode) {
#include "water_particles_oracle_cases.inc"
}
            if(flyingTextMode) {
#include "flying_text_oracle_cases.inc"
            }
            if(histogramMode) {
#include "histogram_oracle_cases.inc"
            }
            if(progressControllers) {
#include "progress_oracle_cases.inc"
            }
            if(optionsControllers) {
#include "options_oracle_cases.inc"
            }
            for(const auto& pair:fontFixtures) require(field<unsigned>(pair.second->object.data(),4)==1,"Owned text font lifetime differs");
            for(const auto& sound:sounds) require(sound->referenceCount()==0,"Button XML sound lifetime differs");sounds.clear();fontFixtures.clear();xmlNodes.clear();
            field<WString>(image,0x260270).~WString(); textureEnvironment()={}; fontFiles()=nullptr; resources=nullptr; localization=nullptr;
        }
        for(auto* p:allocations) if(!ui_test_heap::owns(p)) std::free(p); allocations.clear(); uiElapsedTime()={};device->Release(); d3d->Release(); DestroyWindow(window); FreeLibrary(runtime); FreeLibrary(reference);
        std::cout<<"{\"scenarios\":"<<scenarios<<",\"letter_comparisons\":"<<comparisons<<",\"direction_comparisons\":"<<directionComparisons
            <<",\"list_states\":"<<listStates<<",\"list_letters\":"<<listLetters<<",\"list_boxes\":"<<listBoxes<<",\"list_selections\":"<<listSelections
            <<",\"list_xml\":"<<listXml<<",\"list_clear_faults\":"<<listFaults
            <<",\"multi_states\":"<<multiStates<<",\"multi_letters\":"<<multiLetters<<",\"multi_objects\":"<<multiObjects<<",\"menu_text_branches\":"<<menuTextBranches
            <<",\"dialog_states\":"<<dialogStates<<",\"dialog_letters\":"<<dialogLetters<<",\"dialog_objects\":"<<dialogObjects<<",\"dialog_callbacks\":"<<dialogCallbacks<<",\"dialog_discard_checks\":"<<dialogDiscardChecks
            <<",\"dialog_frames\":"<<dialogFrames<<",\"dialog_pixels\":"<<dialogPixels
            <<",\"profile_states\":"<<selectorStates<<",\"profile_letters\":"<<selectorLetters<<",\"profile_objects\":"<<selectorObjects<<",\"profile_callbacks\":"<<selectorCallbacks<<",\"profile_frames\":"<<selectorFrames<<",\"profile_pixels\":"<<selectorPixels<<",\"profile_deletions\":"<<selectorDeletions<<",\"profile_service_events\":"<<selectorServiceEvents
            <<",\"credits_states\":"<<creditsStates<<",\"credits_decorations\":"<<creditsDecorations<<",\"credits_objects\":"<<creditsObjects<<",\"credits_frames\":"<<creditsFrames<<",\"credits_pixels\":"<<creditsPixels
            <<",\"credits_sound_residues\":"<<creditsSoundResidues
            <<",\"exit_states\":"<<exitStates<<",\"exit_letters\":"<<exitLetters<<",\"exit_objects\":"<<exitObjects<<",\"exit_callbacks\":"<<exitCallbacks<<",\"exit_frames\":"<<exitFrames<<",\"exit_pixels\":"<<exitPixels
            <<",\"checkbox_states\":"<<checkStates<<",\"checkbox_objects\":"<<checkObjects<<",\"checkbox_callbacks\":"<<checkCallbacks<<",\"checkbox_frames\":"<<checkFrames<<",\"checkbox_pixels\":"<<checkPixels
            <<",\"slider_states\":"<<sliderStates<<",\"slider_objects\":"<<sliderObjects<<",\"slider_callbacks\":"<<sliderCallbacks<<",\"slider_frames\":"<<sliderFrames<<",\"slider_pixels\":"<<sliderPixels
            <<",\"loading_states\":"<<loadingStates<<",\"loading_objects\":"<<loadingObjects<<",\"loading_frames\":"<<loadingFrames<<",\"loading_pixels\":"<<loadingPixels<<",\"loading_decorations\":"<<loadingDecorations<<",\"loading_completed\":"<<loadingCompleted<<",\"loading_missing\":"<<loadingMissing<<",\"loading_ranges\":"<<loadingRanges
            <<",\"main_menu_states\":"<<mainMenuStates<<",\"main_menu_letters\":"<<mainMenuLetters<<",\"main_menu_actions\":"<<mainMenuActions<<",\"main_menu_frames\":"<<mainMenuFrames<<",\"main_menu_pixels\":"<<mainMenuPixels<<",\"main_menu_quit_checks\":"<<mainMenuQuitChecks<<",\"main_menu_loading_steps\":"<<mainMenuLoadingSteps
            <<",\"extended_states\":"<<extendedStates<<",\"extended_objects\":"<<extendedObjects<<",\"extended_reloads\":"<<extendedReloads<<",\"extended_letters\":"<<extendedLetters<<",\"extended_frames\":"<<extendedFrames<<",\"extended_pixels\":"<<extendedPixels<<",\"extended_steps\":"<<extendedSteps<<",\"extended_sound_residues\":"<<extendedSoundResidues
            <<",\"production_house_states\":"<<productionHouseStates<<",\"production_house_objects\":"<<productionHouseObjects<<",\"production_house_actions\":"<<productionHouseActions<<",\"production_house_frames\":"<<productionHouseFrames<<",\"production_house_pixels\":"<<productionHousePixels<<",\"production_placement_states\":"<<productionPlacementStates<<",\"field_object_states\":"<<fieldObjectStates<<",\"warehouse_tip_states\":"<<warehouseTipStates<<",\"warehouse_tip_frames\":"<<warehouseTipFrames<<",\"warehouse_tip_pixels\":"<<warehouseTipPixels<<",\"warehouse_tip_reloads\":"<<warehouseTipReloads<<",\"warehouse_states\":"<<warehouseStates<<",\"warehouse_objects\":"<<warehouseObjects<<",\"warehouse_transfers\":"<<warehouseTransfers<<",\"warehouse_frames\":"<<warehouseFrames<<",\"warehouse_pixels\":"<<warehousePixels<<",\"action_area_states\":"<<actionAreaStates<<",\"action_area_objects\":"<<actionAreaObjects<<",\"action_area_goals\":"<<actionAreaGoals<<",\"action_area_clicks\":"<<actionAreaClicks<<",\"action_area_frames\":"<<actionAreaFrames<<",\"action_area_pixels\":"<<actionAreaPixels<<",\"action_area_partial_reloads\":"<<actionAreaReloads<<",\"manufacture_states\":"<<manufactureStates<<",\"manufacture_objects\":"<<manufactureObjects<<",\"manufacture_factories\":"<<manufactureFactories<<",\"manufacture_placements\":"<<manufacturePlacements<<",\"manufacture_removals\":"<<manufactureRemovals<<",\"manufacture_purchases\":"<<manufacturePurchases<<",\"manufacture_builds\":"<<manufactureBuilds<<",\"manufacture_frames\":"<<manufactureFrames<<",\"manufacture_pixels\":"<<manufacturePixels<<",\"manufacture_effects\":"<<manufactureEffects<<",\"manufacture_invalid_slots\":"<<manufactureInvalidSlots<<",\"manufacture_reloads\":"<<manufactureReloads
            <<",\"buy_building_states\":"<<buyBuildingStates<<",\"buy_building_starts\":"<<buyBuildingStarts<<",\"buy_building_frames\":"<<buyBuildingFrames<<",\"buy_building_choices\":"<<buyBuildingChoices<<",\"buy_building_letters\":"<<buyBuildingLetters
            <<",\"buy_seeds_states\":"<<buySeedsStates<<",\"buy_seeds_starts\":"<<buySeedsStarts<<",\"buy_seeds_frames\":"<<buySeedsFrames<<",\"buy_seeds_choices\":"<<buySeedsChoices<<",\"buy_seeds_letters\":"<<buySeedsLetters
            <<",\"movable_union_states\":"<<movableUnionStates<<",\"movable_union_moves\":"<<movableUnionMoves<<",\"movable_union_steps\":"<<movableUnionSteps<<",\"movable_union_frames\":"<<movableUnionFrames<<",\"movable_union_reloads\":"<<movableUnionReloads
            <<",\"tutorial_manager_states\":"<<tutorialManagerStates<<",\"tutorial_manager_starts\":"<<tutorialManagerStarts<<",\"tutorial_manager_steps\":"<<tutorialManagerSteps<<",\"tutorial_manager_predicates\":"<<tutorialManagerPredicates<<",\"tutorial_manager_frames\":"<<tutorialManagerFrames
            <<",\"tutorial_tip_objects\":"<<tutorialTipObjects<<",\"tutorial_tip_states\":"<<tutorialTipStates<<",\"tutorial_tip_letters\":"<<tutorialTipLetters<<",\"tutorial_tip_starts\":"<<tutorialTipStarts<<",\"tutorial_tip_steps\":"<<tutorialTipSteps<<",\"tutorial_tip_frames\":"<<tutorialTipFrames
            <<",\"hud_states\":"<<hudStates<<",\"hud_letters\":"<<hudLetters<<",\"hud_frames\":"<<hudFrames<<",\"hud_pixels\":"<<hudPixels<<",\"hud_steps\":"<<hudSteps<<",\"hud_actions\":"<<hudActions
            <<",\"state_view_states\":"<<stateViewStates<<",\"state_view_moves\":"<<stateViewMoves<<",\"state_view_steps\":"<<stateViewSteps<<",\"state_view_frames\":"<<stateViewFrames<<",\"state_view_pixels\":"<<stateViewPixels<<",\"state_view_reloads\":"<<stateViewReloads
            <<",\"pause_states\":"<<pauseStates<<",\"pause_letters\":"<<pauseLetters<<",\"pause_frames\":"<<pauseFrames<<",\"pause_pixels\":"<<pausePixels<<",\"pause_actions\":"<<pauseActions<<",\"pause_reloads\":"<<pauseReloads<<",\"pause_loading_steps\":"<<pauseLoadingSteps<<",\"pause_sound_residues\":"<<pauseSoundResidues
            <<",\"contract_offer_states\":"<<contractOfferStates<<",\"contract_offer_letters\":"<<contractOfferLetters<<",\"contract_offer_objects\":"<<contractOfferObjects<<",\"contract_offer_starts\":"<<contractOfferStarts<<",\"contract_offer_clicks\":"<<contractOfferClicks<<",\"contract_offer_reloads\":"<<contractOfferReloads<<",\"contract_offer_frames\":"<<contractOfferFrames<<",\"contract_offer_pixels\":"<<contractOfferPixels<<",\"contract_offer_input_clicks\":"<<contractOfferInputClicks
            <<",\"contract_dialog_states\":"<<contractDialogStates<<",\"contract_dialog_letters\":"<<contractDialogLetters<<",\"contract_dialog_objects\":"<<contractDialogObjects<<",\"contract_dialog_starts\":"<<contractDialogStarts<<",\"contract_dialog_clicks\":"<<contractDialogClicks<<",\"contract_dialog_reloads\":"<<contractDialogReloads<<",\"contract_dialog_frames\":"<<contractDialogFrames<<",\"contract_dialog_pixels\":"<<contractDialogPixels<<",\"contract_dialog_input_clicks\":"<<contractDialogInputClicks
            <<",\"price_states\":"<<priceStates<<",\"price_letters\":"<<priceLetters<<",\"price_objects\":"<<priceObjects<<",\"price_reloads\":"<<priceReloads<<",\"price_frames\":"<<priceFrames<<",\"price_pixels\":"<<pricePixels
            <<",\"contract_states\":"<<contractStates<<",\"contract_objects\":"<<contractObjects<<",\"contract_assignments\":"<<contractAssignments<<",\"contract_steps\":"<<contractSteps<<",\"contract_clicks\":"<<contractClicks<<",\"contract_reloads\":"<<contractReloads<<",\"contract_letters\":"<<contractLetters<<",\"contract_formats\":"<<contractFormats<<",\"contract_frames\":"<<contractFrames<<",\"contract_pixels\":"<<contractPixels<<",\"contract_input_clicks\":"<<contractInputClicks
            <<",\"upgrade_states\":"<<upgradeStates<<",\"upgrade_queries\":"<<upgradeQueries<<",\"upgrade_changes\":"<<upgradeChanges<<",\"upgrade_costs\":"<<upgradeCosts<<",\"upgrade_objects\":"<<upgradeObjects<<",\"upgrade_reloads\":"<<upgradeReloads<<",\"upgrade_level_loads\":"<<upgradeLevelLoads<<",\"upgrade_sound_calls\":"<<upgradeSoundCalls
            <<",\"market_forecasts\":"<<marketForecasts<<",\"market_price_states\":"<<marketPriceStates
            <<",\"gameplay_runtime_states\":"<<gameplayRuntimeStates<<",\"gameplay_runtime_steps\":"<<gameplayRuntimeSteps<<",\"gameplay_runtime_objects\":"<<gameplayRuntimeObjects<<",\"gameplay_runtime_events\":"<<gameplayRuntimeEvents<<",\"gameplay_runtime_restores\":"<<gameplayRuntimeRestores<<",\"gameplay_random_states\":"<<gameplayRandomStates
            <<",\"gameplay_time_states\":"<<gameplayTimeStates<<",\"gameplay_timeline_states\":"<<gameplayTimelineStates<<",\"gameplay_event_states\":"<<gameplayEventStates<<",\"gameplay_contract_states\":"<<gameplayContractStates<<",\"gameplay_popularity_states\":"<<gameplayPopularityStates<<",\"gameplay_objects\":"<<gameplayObjects<<",\"gameplay_reloads\":"<<gameplayReloads
            <<",\"game_configuration_states\":"<<gameConfigurationStates<<",\"game_configuration_objects\":"<<gameConfigurationObjects<<",\"game_configuration_reloads\":"<<gameConfigurationReloads<<",\"game_configuration_appearances\":"<<gameConfigurationAppearances<<",\"game_configuration_sound_residues\":"<<gameConfigurationSoundResidues
            <<",\"resource_log_states\":"<<resourceLogStates<<",\"resource_log_units\":"<<resourceLogUnits<<",\"resource_log_entries\":"<<resourceLogEntries<<",\"resource_log_shaders\":"<<resourceLogShaders
            <<",\"panel_states\":"<<panelStates<<",\"panel_objects\":"<<panelObjects<<",\"panel_frames\":"<<panelFrames<<",\"panel_pixels\":"<<panelPixels<<",\"panel_letters\":"<<panelLetters<<",\"panel_reloads\":"<<panelReloads<<",\"panel_sound_residues\":"<<panelSoundResidues<<",\"panel_callbacks\":"<<panelSourceCallbacks
            <<",\"awards_states\":"<<awardsStates<<",\"awards_objects\":"<<awardsObjects<<",\"awards_frames\":"<<awardsFrames<<",\"awards_pixels\":"<<awardsPixels<<",\"awards_reloads\":"<<awardsReloads<<",\"awards_decorations\":"<<awardsDecorations<<",\"awards_letters\":"<<awardsLetters<<",\"awards_returns\":"<<awardsSourceReturns<<",\"awards_sound_residues\":"<<awardsSoundResidues<<",\"awards_panel_callbacks\":"<<awardsPanelCallbacks
            <<",\"effect_states\":"<<effectStates<<",\"effect_objects\":"<<effectObjects<<",\"effect_frames\":"<<effectFrames<<",\"effect_pixels\":"<<effectPixels<<",\"effect_steps\":"<<effectSteps<<",\"effect_reloads\":"<<effectReloads<<",\"effect_sounds\":"<<effectSounds<<",\"effect_sound_reloads\":"<<effectSoundReloads
            <<",\"tooltip_states\":"<<tooltipStates<<",\"tooltip_objects\":"<<tooltipObjects<<",\"tooltip_frames\":"<<tooltipFrames<<",\"tooltip_pixels\":"<<tooltipPixels<<",\"tooltip_placements\":"<<tooltipPlacements<<",\"tooltip_reloads\":"<<tooltipReloads<<",\"tooltip_letters\":"<<tooltipLetters
            <<",\"flying_market_effects\":"<<flyingMarketEffects<<",\"flying_market_states\":"<<flyingMarketStates
            <<",\"sales_history_states\":"<<salesHistoryStates<<",\"sales_history_records\":"<<salesHistoryRecords<<",\"sales_history_prunes\":"<<salesHistoryPrunes
            <<",\"water_emitter_states\":"<<waterEmitterStates<<",\"water_emitter_objects\":"<<waterEmitterObjects<<",\"water_emitter_creations\":"<<waterEmitterCreations<<",\"water_emitter_frames\":"<<waterEmitterFrames<<",\"water_emitter_pixels\":"<<waterEmitterPixels
            <<",\"water_particle_states\":"<<waterParticleStates<<",\"water_particle_objects\":"<<waterParticleObjects<<",\"water_particle_copies\":"<<waterParticleCopies<<",\"water_particle_frames\":"<<waterParticleFrames<<",\"water_particle_pixels\":"<<waterParticlePixels
            <<",\"mow_building_states\":"<<mowBuildingStates<<",\"mow_building_actions\":"<<mowBuildingActions<<",\"mow_building_frames\":"<<mowBuildingFrames<<",\"mow_building_pixels\":"<<mowBuildingPixels
            <<",\"farm_field_states\":"<<farmFieldStates<<",\"farm_field_objects\":"<<farmFieldObjects<<",\"farm_field_actions\":"<<farmFieldActions<<",\"farm_field_frames\":"<<farmFieldFrames<<",\"farm_field_pixels\":"<<farmFieldPixels
            <<",\"sprinkle_states\":"<<sprinkleStates<<",\"sprinkle_objects\":"<<sprinkleObjects<<",\"sprinkle_actions\":"<<sprinkleActions<<",\"sprinkle_frames\":"<<sprinkleFrames<<",\"sprinkle_pixels\":"<<sprinklePixels
            <<",\"water_source_states\":"<<waterSourceStates<<",\"water_source_actions\":"<<waterSourceActions<<",\"water_source_frames\":"<<waterSourceFrames<<",\"water_source_pixels\":"<<waterSourcePixels
            <<",\"dig_building_states\":"<<digBuildingStates<<",\"dig_building_actions\":"<<digBuildingActions<<",\"dig_building_frames\":"<<digBuildingFrames<<",\"dig_building_pixels\":"<<digBuildingPixels
            <<",\"market_states\":"<<marketStates<<",\"market_actions\":"<<marketActions<<",\"market_frames\":"<<marketFrames<<",\"market_pixels\":"<<marketPixels
            <<",\"flying_text_states\":"<<flyingTextStates<<",\"flying_text_objects\":"<<flyingTextObjects<<",\"flying_text_frames\":"<<flyingTextFrames<<",\"flying_text_pixels\":"<<flyingTextPixels<<",\"flying_text_reloads\":"<<flyingTextReloads<<",\"flying_text_callbacks\":"<<flyingTextCallbacks
            <<",\"histogram_states\":"<<histogramStates<<",\"histogram_objects\":"<<histogramObjects<<",\"histogram_reloads\":"<<histogramReloads<<",\"histogram_frames\":"<<histogramFrames<<",\"histogram_pixels\":"<<histogramPixels
            <<",\"progress_states\":"<<progressStates<<",\"progress_objects\":"<<progressObjects<<",\"progress_frames\":"<<progressFrames<<",\"progress_pixels\":"<<progressPixels<<",\"progress_reloads\":"<<progressReloads
            <<",\"options_states\":"<<optionsStates<<",\"options_objects\":"<<optionsObjects<<",\"options_letters\":"<<optionsLetters<<",\"options_events\":"<<optionsEvents<<",\"options_frames\":"<<optionsFrames<<",\"options_pixels\":"<<optionsPixels<<",\"options_resets\":"<<optionsResets
            <<",\"edit_frames\":"<<editFrames<<",\"edit_letters\":"<<editLetters<<",\"copies\":"<<copies<<",\"copy_letters\":"<<copyLetters<<",\"lines_destroyed\":9,\"decoration_comparisons\":"<<decorationComparisons<<",\"menu_views_destroyed\":3,\"menu_content_comparisons\":"<<menuContentComparisons<<",\"menu_frames\":"<<menuFrames<<",\"menu_pixels\":"<<menuPixels<<",\"menu_input_frames\":24,\"menu_sound_requests\":"<<menuSoundRequests<<",\"device_flags\":"<<(preserve ? 0x22:0x20)<<",\"x87_control_before\":"<<controlBefore<<",\"x87_control_after\":"<<_control87(0,0)<<"}\n";
    } catch(const std::exception& e) { std::cerr<<e.what()<<" heap "<<ui_test_heap::used<<"/"<<ui_test_heap::capacity<<" stage "<<stage<<" trial "<<trial<<" action "<<action<<'\n'; return 1; }
      catch(const char* e) { std::cerr<<e<<" stage "<<stage<<" trial "<<trial<<" action "<<action<<'\n'; return 1; }
}
