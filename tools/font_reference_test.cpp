// Test-only original font lookup oracle. Constructed tree/vector fixtures are
// supplied to two read-only methods; no original constructor or entry point runs.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Font.h"
#include "FileIndex.h"
#include "ResourceManager.h"
#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace farmar::recovered;
namespace {
struct Fixup { std::uint32_t rva,address; };
const Fixup fixups[]={
#include "font_oracle_fixups.inc"
};
struct Node { Node *left,*parent,*right; std::uint16_t key,first,last,index; unsigned char color,nil,pad[2]; };
static_assert(sizeof(Node)==24,"Original font map node");
String path(const wchar_t* input) {
    int count=WideCharToMultiByte(CP_ACP,0,input,-1,nullptr,0,nullptr,nullptr);
    std::string output(count,'\0'); WideCharToMultiByte(CP_ACP,0,input,-1,output.data(),count,nullptr,nullptr);
    return String(output.c_str());
}
Node* tree(std::vector<Node>& nodes,std::size_t begin,std::size_t end,Node* parent,Node* head) {
    if(begin==end) return head;
    const auto mid=(begin+end)/2; auto* node=&nodes[mid]; node->parent=parent;
    node->left=tree(nodes,begin,mid,node,head); node->right=tree(nodes,mid+1,end,node,head); return node;
}
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=3) return 2;
    auto module=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES); if(!module) return 2;
    try {
        auto* image=reinterpret_cast<unsigned char*>(module);
        for(const auto& fixup:fixups) {
            std::uint32_t address; std::memcpy(&address,image+fixup.rva,4);
            if(address!=fixup.address) throw std::runtime_error("Font oracle operand differs");
            DWORD old; if(!VirtualProtect(image+fixup.rva,4,PAGE_EXECUTE_READWRITE,&old)) throw std::runtime_error("Font oracle protection failed");
            address=reinterpret_cast<std::uintptr_t>(image)+(address-0x400000U); std::memcpy(image+fixup.rva,&address,4);
            DWORD restored; VirtualProtect(image+fixup.rva,4,old,&restored);
        }
        FlushInstructionCache(GetCurrentProcess(),image,0x200000);
        using Available=bool(__thiscall*)(const void*,std::uint16_t);
        using Glyph=bool(__thiscall*)(const void*,std::uint16_t,FontGlyph*);
        const auto available=reinterpret_cast<Available>(image+0x10b3d0);
        const auto glyph=reinterpret_cast<Glyph>(image+0x10b450);
        FileIndex files(path(argv[2]),String("pack")); fontFiles()=&files;
        std::uint32_t comparisons=0,valid=0,cacheChecks=0;
        {
            ResourceManager manager(files);
            for(const char* name:{"UI/Font/font_tips.fnt","UI/honey_font.fnt","UI/menu_font.fnt"}) {
                auto* font=manager.getFont(String(name));
                // Parse the binary independently for the fixture and check all
                // decoded fields before constructing the original map layout.
                auto file=files.open(String(name)); std::vector<unsigned char> bytes(file->size()); file->read(bytes.data(),bytes.size());
                std::uint32_t count; std::memcpy(&count,bytes.data()+4,4);
                if(count!=font->ranges().size()) throw std::runtime_error("Font range count differs");
                std::vector<Node> nodes(count); Node head{}; head.nil=1;
                for(unsigned i=0;i<count;++i) {
                    FontRange range; std::memcpy(&range,bytes.data()+8+i*6,6);
                    const auto& decoded=font->ranges().at(range.last);
                    if(std::memcmp(&range,&decoded,6)) throw std::runtime_error("Font range data differs");
                    nodes[i].key=nodes[i].last=range.last; nodes[i].first=range.first; nodes[i].index=range.index;
                }
                std::memcpy(&count,bytes.data()+8+nodes.size()*6,4);
                std::vector<FontGlyph> expectedGlyphs(count);
                std::memcpy(expectedGlyphs.data(),bytes.data()+12+nodes.size()*6,count*20);
                if(count!=font->glyphs().size() || std::memcmp(expectedGlyphs.data(),font->glyphs().data(),count*20)) throw std::runtime_error("Font glyph table differs");
                head.parent=tree(nodes,0,nodes.size(),&head,&head); head.left=&nodes.front(); head.right=&nodes.back();
                std::array<unsigned char,68> object{}; Node* headPointer=&head; auto* glyphPointer=expectedGlyphs.data();
                std::memcpy(object.data()+0x28,&headPointer,4); std::memcpy(object.data()+0x34,&glyphPointer,4);
                for(unsigned character=0;character<65536;++character) {
                    if(font->isCharAvailable(character)!=available(object.data(),character)) throw std::runtime_error("Original font availability differs");
                    ++comparisons;
                    FontGlyph actual,expected; std::memset(&actual,0xcd,20); std::memset(&expected,0xcd,20);
                    const bool a=font->getGlyph(character,actual),b=glyph(object.data(),character,&expected);
                    if(a!=b || std::memcmp(&actual,&expected,20)) throw std::runtime_error("Original font glyph lookup differs");
                    ++comparisons; valid+=a;
                }
                auto* cached=manager.getFont(String(name));
                if(cached!=font || cached->referenceCount()!=2 || cached->resourceType()!=2) throw std::runtime_error("Font cache differs");
                ++cacheChecks; cached->releaseReference(); font->releaseReference();
                if(font->referenceCount()!=0) throw std::runtime_error("Font reference release differs");
                ++cacheChecks;
            }
            manager.clearUseless(); if(manager.cachedCount()) throw std::runtime_error("Unused font cache retained"); ++cacheChecks;
        }
        fontFiles()=nullptr; FreeLibrary(module);
        std::cout << "{\"fonts\":3,\"lookup_comparisons\":" << comparisons << ",\"available_glyphs\":" << valid << ",\"cache_checks\":" << cacheChecks << "}\n";
    } catch(const std::exception& error) { std::cerr << error.what() << '\n'; FreeLibrary(module); return 1; }
}
