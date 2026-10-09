// Test-only makeLetters oracle. Fixture virtual methods capture geometry. String
// copying/freeing and control activation calls are isolated adapter boundaries;
// this test does not claim to verify localization, ownership, or activation.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "TextLineLayout.h"
#include "LetterSprite.h"
#include "FileIndex.h"
#include "GameWString.h"
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <random>
#include <set>
#include <stdexcept>
using namespace farmar::recovered;
namespace {
LONG CALLBACK diagnosticException(EXCEPTION_POINTERS* info) {
    if(info->ExceptionRecord->ExceptionCode==EXCEPTION_ACCESS_VIOLATION) {
        const auto& c=*info->ContextRecord;
        std::cerr << "Oracle access violation EIP=" << std::hex << c.Eip << " ECX=" << c.Ecx << " EAX=" << c.Eax << " EDX=" << c.Edx
            << " address=" << info->ExceptionRecord->ExceptionInformation[1] << '\n';
        TerminateProcess(GetCurrentProcess(),1);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
struct Fixup { std::uint32_t rva,address; };
const Fixup fixups[]={
#include "text_layout_oracle_fixups.inc"
#include "letter_oracle_fixups.inc"
};
struct Node { Node *left,*parent,*right; std::uint16_t key,first,last,index; unsigned char color,nil,pad[2]; };
struct Link { Link *next,*previous; void* control; };
struct LetterFixture {
    std::array<unsigned char,0x24c> object{};
    float uv[4]{},size[2]{},depth=0;
    unsigned activations=0,enablements=0,positions=0;
};
static_assert(sizeof(Node)==24,"Original font node");
template<class T,std::size_t N> void put(std::array<unsigned char,N>& object,unsigned offset,const T& value) {
    std::memcpy(object.data()+offset,&value,sizeof(value));
}
String path(const wchar_t* input) {
    int count=WideCharToMultiByte(CP_ACP,0,input,-1,nullptr,0,nullptr,nullptr); std::string output(count,'\0');
    WideCharToMultiByte(CP_ACP,0,input,-1,output.data(),count,nullptr,nullptr); return String(output.c_str());
}
Node* tree(std::vector<Node>& nodes,std::size_t a,std::size_t b,Node* parent,Node* head) {
    if(a==b) return head; auto* n=&nodes[(a+b)/2]; n->parent=parent;
    n->left=tree(nodes,a,(a+b)/2,n,head); n->right=tree(nodes,(a+b)/2+1,b,n,head); return n;
}
void jump(unsigned char* target,const void* function) {
    DWORD old; if(!VirtualProtect(target,5,PAGE_EXECUTE_READWRITE,&old)) throw std::runtime_error("Oracle adapter protection failed");
    target[0]=0xe9; const auto relative=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(function)-reinterpret_cast<std::uintptr_t>(target)-5);
    std::memcpy(target+1,&relative,4); DWORD restored; VirtualProtect(target,5,old,&restored);
}
WString* __fastcall copy(WString* self,void*,const WString* text,unsigned start,unsigned count) { self->assign(*text,start,count); return self; }
void __fastcall uv(LetterFixture* self,void*,const float* value) { std::memcpy(self->uv,value,16); }
void __fastcall size(LetterFixture* self,void*,const float* value) { std::memcpy(self->size,value,8); }
const float* __fastcall getSize(LetterFixture* self,void*) { return self->size; }
void __fastcall position(LetterFixture* self,void*,const float* value) { put(self->object,0x5c,value[0]); put(self->object,0x60,value[1]); ++self->positions; }
void __fastcall depth(LetterFixture* self,void*,const float* value) { self->depth=*value; }
unsigned __fastcall parentValue(void*,void*) { return 1; }
void __fastcall inherited(void*,void*,unsigned) {}
void __fastcall assignTexture(void* field,void*,const String* name) {
    // This oracle isolates the resource lookup boundary; record the supplied
    // name length in fixture storage, without an original singleton or GPU.
    const unsigned length=name->size(); std::memcpy(static_cast<unsigned char*>(field)+4,&length,4);
}
void __fastcall activate(LetterFixture* self,void*) { ++self->activations; }
void __fastcall enable(LetterFixture* self,void*) { ++self->enablements; }
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=3) return 2;
    auto module=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES); if(!module) return 2;
    AddVectoredExceptionHandler(1,diagnosticException);
    try {
        auto* image=reinterpret_cast<unsigned char*>(module);
        std::set<unsigned> applied;
        for(const auto& fixup:fixups) {
            if(!applied.insert(fixup.rva).second) continue;
            std::uint32_t address; std::memcpy(&address,image+fixup.rva,4); if(address!=fixup.address) throw std::runtime_error("Text layout oracle operand differs");
            DWORD old; VirtualProtect(image+fixup.rva,4,PAGE_EXECUTE_READWRITE,&old);
            address=reinterpret_cast<std::uintptr_t>(image)+(address-0x400000U); std::memcpy(image+fixup.rva,&address,4);
            DWORD restored; VirtualProtect(image+fixup.rva,4,old,&restored);
        }
        jump(image+0x17c0,reinterpret_cast<const void*>(&copy)); jump(image+0x1803af,reinterpret_cast<const void*>(&std::free));
        jump(image+0xd37d0,reinterpret_cast<const void*>(&activate)); jump(image+0xd3bb0,reinterpret_cast<const void*>(&enable));
        jump(image+0xd2260,reinterpret_cast<const void*>(&assignTexture));
        FlushInstructionCache(GetCurrentProcess(),image,0x200000);
        using MakeLetters=void(__thiscall*)(void*); const auto makeLetters=reinterpret_cast<MakeLetters>(image+0xdc800);
        const auto makeLetterSprite=reinterpret_cast<MakeLetters>(image+0x10c500);
        std::array<void*,72> parentTable{},letterTable{};
        for(auto slot:{0x40,0xf4,0xc4}) parentTable[slot/4]=reinterpret_cast<void*>(&parentValue);
        for(auto slot:{0x44,0x108,0xc8}) letterTable[slot/4]=reinterpret_cast<void*>(&inherited);
        letterTable[0x110/4]=reinterpret_cast<void*>(&uv); letterTable[0x4c/4]=reinterpret_cast<void*>(&size);
        letterTable[0x48/4]=reinterpret_cast<void*>(&getSize); letterTable[0xc0/4]=reinterpret_cast<void*>(&position); letterTable[0xd4/4]=reinterpret_cast<void*>(&depth);
        FileIndex files(path(argv[2]),String("pack")); fontFiles()=&files;
        std::mt19937 random(0x54455854); unsigned trials=0,letterComparisons=0,spriteComparisons=0;
        auto number=[&] { return static_cast<float>(static_cast<int>(random()%200001)-100000)/193.f; };
        for(const char* name:{"UI/Font/font_tips.fnt","UI/honey_font.fnt","UI/menu_font.fnt"}) {
            cFont font{String(name)}; std::vector<Node> nodes(font.ranges().size()); unsigned at=0;
            for(const auto& pair:font.ranges()) { const auto& r=pair.second; auto& n=nodes[at++]; n.key=n.last=r.last; n.first=r.first; n.index=r.index; }
            Node head{}; head.nil=1; head.parent=tree(nodes,0,nodes.size(),&head,&head); head.left=&nodes.front(); head.right=&nodes.back();
            std::array<unsigned char,68> fontObject{}; Node* headPointer=&head; const auto* glyphs=font.glyphs().data();
            put(fontObject,0x28,headPointer); put(fontObject,0x34,glyphs);
            for(unsigned trial=0;trial<4000;++trial) {
                std::wstring text; const auto count=random()%41;
                for(unsigned i=0;i<count;++i) {
                    const unsigned choice=random()%5;
                    text.push_back(choice==0 ? L' ' : choice==1 ? wchar_t(0xa0) : choice==2 ? wchar_t(0xdead) : wchar_t(33+random()%531));
                }
                const unsigned width=128U<<(random()%5),height=128U<<(random()%4);
                const float scaleU=trial%2 ? 0.875f:1,scaleV=trial%3 ? 0.75f:1;
                TextLinePlacement p{number(),number(),number(),number()/30,number()/100,random()%4};
                const float oldHeight=trial%3 ? 0:50;
                const auto actual=layoutTextLine(font,text,width,height,scaleU,scaleV,p,oldHeight);
                std::vector<LetterFixture> letters(actual.letters.size()); std::vector<Link> links(letters.size()); Link listHead{};
                listHead.next=links.empty() ? &listHead:links.data(); listHead.previous=links.empty() ? &listHead:&links.back();
                for(unsigned i=0;i<letters.size();++i) {
                    auto& link=links[i]; link.control=&letters[i]; link.next=i+1<links.size() ? &links[i+1]:&listHead; link.previous=i ? &links[i-1]:&listHead;
                    auto* table=letterTable.data(); put(letters[i].object,0,table);
                }
                std::array<unsigned char,0x2b8> object{}; auto* table=parentTable.data(); put(object,0,table);
                WString displayed{std::wstring_view(text)}; std::memcpy(object.data()+0x22c,&displayed,28);
                put(object,0x1b0,static_cast<unsigned char>(1)); put(object,0x288,1U); put(object,0x26c,1U);
                auto* fontPointer=fontObject.data(); put(object,0x2ac,fontPointer); auto* linkPointer=&listHead; put(object,0x24c,linkPointer);
                std::array<unsigned char,72> texture{}; put(texture,0x38,width); put(texture,0x3c,height); put(texture,0x28,scaleU); put(texture,0x2c,scaleV);
                auto* texturePointer=texture.data(); put(object,0x2b0,texturePointer);
                put(object,0x5c,p.x); put(object,0x60,p.y); put(object,0x228,p.depth); put(object,0x290,p.spaceWidth); put(object,0x298,p.multiplier);
                put(object,0x2a4,p.alignment); put(object,0x2a0,oldHeight); put(object,0x254,static_cast<unsigned char>(1));
                makeLetters(object.data());
                if(std::memcmp(object.data()+0x29c,&actual.width,4) || std::memcmp(object.data()+0x2a0,&actual.height,4) || object[0x254])
                    throw std::runtime_error("Original TextLine dimensions differ");
                for(unsigned i=0;i<letters.size();++i) {
                    const auto& a=actual.letters[i]; auto& e=letters[i];
                    const float expectedUV[]={a.glyph.u0,a.glyph.v0,a.glyph.u1,a.glyph.v1},expectedSize[]={a.width,a.height};
                    if(std::memcmp(expectedUV,e.uv,16) || std::memcmp(expectedSize,e.size,8) || std::memcmp(&a.x,e.object.data()+0x5c,4)
                       || std::memcmp(&a.y,e.object.data()+0x60,4) || std::memcmp(&a.depth,&e.depth,4) || e.activations!=1 || e.enablements!=1 || e.positions!=2) {
                        std::cerr << "font " << name << " trial " << trial << " letter " << i << " size " << std::hexfloat << a.width << ',' << a.height
                            << " expected " << e.size[0] << ',' << e.size[1] << " x " << a.x << " expected "; float x; std::memcpy(&x,e.object.data()+0x5c,4); std::cerr << x << '\n';
                        std::cerr << " depth " << a.depth << " expected " << e.depth << " callbacks " << e.activations << ',' << e.enablements << ',' << e.positions << '\n';
                        for(unsigned j=0;j<4;++j) std::cerr << " UV " << j << " actual " << expectedUV[j] << " expected " << e.uv[j] << '\n';
                        throw std::runtime_error("Original TextLine letter geometry differs");
                    }
                    ++letterComparisons;
                    const DWORD color=random(); const float rootAlpha=number()/500,alpha=number()/500;
                    std::array<unsigned char,0x24c> root{}; put(root,0x8,rootAlpha); auto* rootPointer=root.data();
                    put(e.object,0x20,rootPointer); const auto* alphaPointer=&alpha; put(e.object,0x58,alphaPointer);
                    put(e.object,0x64,e.size[0]); put(e.object,0x68,e.size[1]); put(e.object,0x248,color);
                    std::memcpy(e.object.data()+0x238,e.uv,16); put(e.object,0x1b0,static_cast<unsigned char>(1));
                    SpriteDefinition sprite{}; sprite.subdivisions=1; std::memcpy(e.object.data()+0x70,&sprite,76);
                    updateLetterSprite(sprite,a,color,rootAlpha,alpha); makeLetterSprite(e.object.data());
                    if(std::memcmp(e.object.data()+0x70,&sprite,76) || e.object[0x1b0]) throw std::runtime_error("Original Letter sprite differs");
                    ++spriteComparisons;
                }
                ++trials;
            }
        }
        fontFiles()=nullptr; FreeLibrary(module);
        std::cout << "{\"layout_scenarios\":" << trials << ",\"letter_comparisons\":" << letterComparisons << ",\"letter_sprite_comparisons\":" << spriteComparisons << "}\n";
    } catch(const std::exception& error) { std::cerr << error.what() << '\n'; FreeLibrary(module); return 1; }
}
