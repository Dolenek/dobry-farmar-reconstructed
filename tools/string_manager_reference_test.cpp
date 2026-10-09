// Original StringManager getString, dump-disabled path, on owned tree fixtures.
// Its WString copy boundary uses the separately reconstructed string allocator.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "StringManager.h"
#include "FileIndex.h"
#include "Settings.h"
#include "ResourceManager.h"
#include <array>
#include <cstring>
#include <iostream>
#include <new>
#include <random>
#include <stdexcept>
using namespace farmar::recovered;
namespace {
struct Fixup { std::uint32_t rva,address; };
const Fixup fixups[]={
#include "string_manager_oracle_fixups.inc"
};
struct Node { Node *left,*parent,*right; WString key,value; unsigned char color=0,nil=0,pad[2]{}; };
static_assert(sizeof(Node)==72,"Original translation map node");
template<class T,std::size_t N> void put(std::array<unsigned char,N>& object,unsigned offset,const T& value) { std::memcpy(object.data()+offset,&value,sizeof(value)); }
String path(const wchar_t* input) {
    int count=WideCharToMultiByte(CP_ACP,0,input,-1,nullptr,0,nullptr,nullptr); std::string output(count,'\0');
    WideCharToMultiByte(CP_ACP,0,input,-1,output.data(),count,nullptr,nullptr); return String(output.c_str());
}
Node* tree(std::vector<Node>& nodes,std::size_t a,std::size_t b,Node* parent,Node* head) {
    if(a==b) return head; auto* n=&nodes[(a+b)/2]; n->parent=parent;
    n->left=tree(nodes,a,(a+b)/2,n,head); n->right=tree(nodes,(a+b)/2+1,b,n,head); return n;
}
WString* __fastcall copy(WString* self,void*,const WString* text,unsigned start,unsigned count) { self->assign(*text,start,count); return self; }
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=4) return 2;
    auto module=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES); if(!module) return 2;
    auto* image=reinterpret_cast<unsigned char*>(module); WString* fallback=nullptr;
    try {
        for(const auto& fixup:fixups) {
            unsigned address; std::memcpy(&address,image+fixup.rva,4); if(address!=fixup.address) throw std::runtime_error("StringManager oracle operand differs");
            DWORD old; if(!VirtualProtect(image+fixup.rva,4,PAGE_EXECUTE_READWRITE,&old)) throw std::runtime_error("Oracle protection failed");
            address=reinterpret_cast<std::uintptr_t>(image)+(address-0x400000U); std::memcpy(image+fixup.rva,&address,4);
            DWORD restored; VirtualProtect(image+fixup.rva,4,old,&restored);
        }
        DWORD old; VirtualProtect(image+0x17c0,5,PAGE_EXECUTE_READWRITE,&old); image[0x17c0]=0xe9;
        const unsigned distance=reinterpret_cast<std::uintptr_t>(&copy)-reinterpret_cast<std::uintptr_t>(image+0x17c5);
        std::memcpy(image+0x17c1,&distance,4); DWORD restored; VirtualProtect(image+0x17c0,5,old,&restored);
        // Initialize only this oracle's fallback buffer and skip original atexit.
        image[0x2605e4]=1; fallback=new (image+0x2605c8) WString();
        FlushInstructionCache(GetCurrentProcess(),image,0x200000);
        using Get=const WString*(__thiscall*)(void*,const WString*); const auto get=reinterpret_cast<Get>(image+0x12ed20);
        FileIndex files(path(argv[2]),String("pack")); files.mountCommon(path(argv[3])); ResourceManager resources(files);
        Settings settings(files,[&](const String& name) { return resources.loadSettingsXML(name); }); settings.set(String("game"),String("DumpStrings"),false);
        unsigned translations=0,checks=0,dumpChecks=0;
        {
            StringManager strings(files,settings,resources);
            std::vector<Node> nodes(strings.translations().size()); unsigned at=0;
            for(const auto& pair:strings.translations()) { nodes[at].key=WString(pair.first); nodes[at].value=pair.second; ++at; }
            translations=at; Node head{}; head.nil=1; head.parent=tree(nodes,0,nodes.size(),&head,&head);
            head.left=nodes.empty() ? &head:&nodes.front(); head.right=nodes.empty() ? &head:&nodes.back();
            std::array<unsigned char,32> object{}; Node* headPointer=&head; put(object,8,headPointer);
            const auto compare=[&](const WString& key) {
                const auto& actual=strings.getString(key); const auto* expected=get(object.data(),&key);
                if(actual.view()!=expected->view()) throw std::runtime_error("Original localized text differs");
                if(key.empty() && (&actual!=&key || expected!=&key)) throw std::runtime_error("Empty localization reference differs");
                ++checks;
            };
            for(const auto& node:nodes) compare(node.key);
            compare(WString()); compare(WString(L"missing"));
            std::mt19937 random(0x4c4f4341);
            for(unsigned i=0;i<10000;++i) {
                std::wstring value;
                for(unsigned j=0,n=random()%45;j<n;++j) value.push_back(static_cast<wchar_t>(random()));
                compare(WString(value));
            }
            const WString a(L"unknown-first"),b(L"unknown-second"); const auto& held=strings.getString(a); strings.getString(b);
            if(held.view()!=b.view() || !strings.missingStrings().empty()) throw std::runtime_error("Shared fallback or dump-disabled collection differs");
            ++checks;
        }
        settings.set(String("game"),String("DumpStrings"),true);
        {
            StringManager strings(files,settings,resources); const WString key(L"fixture-missing");
            const auto& first=strings.getString(key); if(first.view()!=L"!!!!!!!fixture-missing") throw std::runtime_error("Dump marker differs"); ++dumpChecks;
            const auto& second=strings.getString(key); if(second.view()!=first.view() || &second==&first) throw std::runtime_error("Dump insertion reference differs"); ++dumpChecks;
            strings.getString(WString(L"another-fixture")); if(strings.missingStrings().size()!=2) throw std::runtime_error("Missing key set differs"); ++dumpChecks;
        }
        auto dump=files.open(String("strdubp.xml")); std::vector<unsigned char> bytes(dump->size()); dump->read(bytes.data(),bytes.size());
        const std::wstring expected=L"<?xml version=\"1.0\" encoding=\"UTF-16\"?>\n<Strings><s>\n<text>another-fixture</text>\n<translation>another-fixture</translation>\n</s><s>\n<text>fixture-missing</text>\n<translation>fixture-missing</translation>\n</s></Strings>";
        if(bytes.size()!=expected.size()*2 || std::memcmp(bytes.data(),expected.data(),bytes.size())) throw std::runtime_error("Dump bytes differ"); ++dumpChecks;
        fallback->~WString(); fallback=nullptr; FreeLibrary(module);
        std::cout << "{\"translations\":" << translations << ",\"original_lookup_comparisons\":" << checks << ",\"dump_fixture_checks\":" << dumpChecks << "}\n";
    } catch(const std::exception& error) { std::cerr << error.what() << '\n'; if(fallback) fallback->~WString(); FreeLibrary(module); return 1; }
}
