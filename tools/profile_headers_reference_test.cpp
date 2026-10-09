// cProfile header oracle only; whole profile property/persistence lifecycle is
// deliberately not represented by these constructor and XML fixtures.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ProfileHeaders.h"
#include "FileIndex.h"
#include "XmlDocument.h"
#include "StringConversion.h"
#include <array>
#include <cstdlib>
#include <cstring>
#include <float.h>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
using namespace farmar::recovered;
namespace {
unsigned char* image=nullptr;unsigned scenario=0,repeat=0;
template<class T> T& field(void* p,unsigned offset) { return *reinterpret_cast<T*>(static_cast<unsigned char*>(p)+offset); }
template<class T> T function(unsigned rva) { return reinterpret_cast<T>(image+rva); }
void require(bool result,const char* error) { if(!result) { std::cerr<<error<<" scenario "<<scenario<<" repeat "<<repeat<<'\n';throw std::runtime_error(error); } }
void pointer(unsigned rva,void* value) { std::memcpy(image+rva,&value,4); }
void redirect(unsigned rva,void* value) { unsigned char bytes[5]={0xe9};const unsigned offset=reinterpret_cast<unsigned>(value)-reinterpret_cast<unsigned>(image+rva+5);std::memcpy(bytes+1,&offset,4);std::memcpy(image+rva,bytes,5); }
LONG CALLBACK diagnose(EXCEPTION_POINTERS* e) {
    if(e->ExceptionRecord->ExceptionCode==EXCEPTION_ACCESS_VIOLATION) {
        std::cerr<<"Profile AV scenario "<<scenario<<" repeat "<<repeat<<" RVA "<<std::hex
            <<reinterpret_cast<unsigned>(e->ExceptionRecord->ExceptionAddress)-reinterpret_cast<unsigned>(image)
            <<" target "<<e->ExceptionRecord->ExceptionInformation[1]<<'\n';TerminateProcess(GetCurrentProcess(),1);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
std::set<void*> allocations;
void* __cdecl allocate(unsigned bytes) { auto* p=std::malloc(bytes);if(p) allocations.insert(p);return p; }
void __cdecl release(void* p) { allocations.erase(p);std::free(p); }
WString* __fastcall assign(WString* self,void*,const WString* value,unsigned first,unsigned count) { return &self->assign(*value,first,count); }
int __fastcall compare(WString* self,void*,unsigned first,unsigned count,const wchar_t* value,unsigned size) { return self->compare(first,count,value,size); }
template<class T> T* __cdecl typed(T* output,const WString* value) { if(!value->empty()) { std::wistringstream stream(value->c_str());stream>>*output; } return output; }
String* __cdecl formatID(String* output,unsigned id) { return new(output) String(formatProfileID(id)); }
struct XmlAdapter { void** table;const XmlNode* node; };
std::array<void*,35> xmlTable{};std::map<const XmlNode*,XmlAdapter> adapters;
XmlAdapter* wrap(const XmlNode* node) { auto& adapter=adapters[node];adapter={xmlTable.data(),node};return &adapter; }
WString* __fastcall attribute(XmlAdapter* self,void*,WString* output,const wchar_t* name) { return new(output) WString(self->node->attribute(WString(name))); }
unsigned __fastcall count(XmlAdapter* self,void*) { return unsigned(self->node->children().size()); }
bool __fastcall section(XmlAdapter* self,void*,unsigned index,XmlAdapter** output,String* name) {
    XmlNode* child=nullptr;if(!self->node->section(index,child,name)) return false;*output=wrap(child);return true;
}
void collect(void* node,ProfileHeaders::Names& output) {
    if(field<unsigned char>(node,0x2d)) return;
    collect(field<void*>(node,0),output);output.emplace(field<WString>(node,12),field<unsigned>(node,40));collect(field<void*>(node,8),output);
}
void compareHeaders(const ProfileHeaders& actual,void* object) {
    require(actual.nextID()==field<unsigned>(object,0x44) && actual.nowPlaying()==bool(field<unsigned char>(object,4)) && actual.dirty()==bool(field<unsigned char>(object,0x54)),"Profile header scalars differ");
    require(actual.currentName().view()==field<WString>(object,0x24).view() && actual.storedCurrentName().view()==field<WString>(object,8).view(),"Profile current/stored user name differs");
    ProfileHeaders::Names expected;auto* head=field<void*>(object,0x4c);collect(field<void*>(head,4),expected);
    require(actual.names().size()==expected.size() && actual.names().size()==field<unsigned>(object,0x50),"Profile name count differs");
    auto native=expected.begin();for(const auto& entry:actual.names()) { require(entry.first.view()==native->first.view() && entry.second==native->second,"Profile first-insert ID or name ordering differs");++native; }
}
struct ListNode { ListNode *next,*previous;WString name; };
struct NameList {
    unsigned allocator=0;ListNode* head=nullptr;unsigned size=0;ListNode sentinel{};
    NameList() { sentinel.next=sentinel.previous=&sentinel;head=&sentinel; }
    ~NameList() { auto* node=head->next;while(node!=head) { auto* next=node->next;node->name.~WString();release(node);node=next; } }
    std::vector<WString> values() const { std::vector<WString> result;for(auto* n=head->next;n!=head;n=n->next) result.push_back(n->name);return result; }
};
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=3 && argc!=4) return 2;
    auto reference=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES);if(!reference) return 2;
    try {
        if(argc==4) _control87(_PC_24,_MCW_PC);AddVectoredExceptionHandler(1,&diagnose);
        auto* loaded=reinterpret_cast<unsigned char*>(reference);
        auto* headers=reinterpret_cast<IMAGE_NT_HEADERS*>(loaded+reinterpret_cast<IMAGE_DOS_HEADER*>(loaded)->e_lfanew);
        image=static_cast<unsigned char*>(VirtualAlloc(nullptr,headers->OptionalHeader.SizeOfImage,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE));
        require(image && image!=loaded,"Profile oracle relocation failed");std::memcpy(image,loaded,headers->OptionalHeader.SizeOfImage);
        struct Fixup { unsigned rva,address; };const Fixup fixups[]={
#include "profile_headers_oracle_fixups.inc"
        };
        for(const auto& fixup:fixups) { require(field<unsigned>(image,fixup.rva)==fixup.address,"Profile oracle operand differs");const unsigned value=reinterpret_cast<unsigned>(image)+fixup.address-0x400000;std::memcpy(image+fixup.rva,&value,4); }
        redirect(0x17edb1,reinterpret_cast<void*>(&allocate));redirect(0x1803af,reinterpret_cast<void*>(&release));
        redirect(0x17c0,reinterpret_cast<void*>(&assign));redirect(0x1050,reinterpret_cast<void*>(&compare));
        redirect(0xd94a0,reinterpret_cast<void*>(&typed<unsigned>));redirect(0xd9670,reinterpret_cast<void*>(&typed<bool>));redirect(0xe17e0,reinterpret_cast<void*>(&formatID));
        xmlTable[5]=reinterpret_cast<void*>(&section);xmlTable[31]=reinterpret_cast<void*>(&attribute);xmlTable[34]=reinterpret_cast<void*>(&count);
        FlushInstructionCache(GetCurrentProcess(),image,headers->OptionalHeader.SizeOfImage);
        FileIndex files(toNarrow(WString(argv[2])),String("pack"));
        files.mountCommon(toNarrow(WString(argv[2])));
        std::map<unsigned,std::unique_ptr<XmlDocument>> documents;
        const WString empty;unsigned loads=0,lookupCount=0,listComparisons=0,currentIDs=0;
        const wchar_t* names[]={L"",L"A",L"a",L"Žofie",L"Řepka",L"Farmář s dlouhým jménem",L"日本語",L"Z",L"A &amp; B",L"&quot;quoted&quot;"};
        for(scenario=0;scenario<100;++scenario) {
            ProfileHeaders actual;std::array<unsigned char,88> object{};
            function<void*(__thiscall*)(void*,const WString*)>(0xecb20)(object.data(),&empty);compareHeaders(actual,object.data());
            {
                NameList nativeList;std::vector<WString> actualList;
                for(repeat=0;repeat<80;++repeat) {
                    const unsigned fixture=(scenario%8)*80+repeat;
                    auto& cached=documents[fixture];
                    if(!cached) {
                    std::wostringstream xml;xml<<L"<Profiles";
                    if(repeat%4!=0) xml<<L" NextID=\""<<fixture<<L"\"";
                    if(repeat%5!=0) xml<<L" CurrentUser=\""<<names[(scenario%8+repeat)%10]<<L"\"";
                    if(repeat%6!=0) xml<<L" NowPlaying=\""<<(repeat%2)<<L"\"";
                    xml<<L">";
                    for(unsigned i=0;i<10;++i) {
                        xml<<(i%3 ? L"<Profile":L"<IgnoredTag");
                        if(i%7!=0) xml<<L" Name=\""<<names[(scenario%8+i+repeat)%10]<<L"\"";
                        if(i%8!=0) xml<<L" ID=\""<<(i==1 ? 0xffffffffU:fixture*10+i)<<L"\"";
                        xml<<L"/>";
                    }
                    xml<<L"</Profiles>";
                    const String filename(("fixture-"+std::to_string(fixture)+".xml").c_str());
                    auto output=files.create(filename);const wchar_t bom=0xfeff;output->write(&bom,2);
                    const auto bytes=xml.str();output->write(bytes.data(),unsigned(bytes.size()*2));output.reset();
                    cached=std::make_unique<XmlDocument>(files,filename);
                    }
                    auto& document=*cached;adapters.clear();
                    actual.loadXML(document);function<void(__thiscall*)(void*,void*)>(0xea000)(object.data(),wrap(&document));compareHeaders(actual,object.data());++loads;
                    for(unsigned i=0;i<10;++i) {
                        const WString name(names[i]);unsigned value=1234;const bool found=actual.findID(name,value);void* nativeNode=nullptr;
                        function<void(__thiscall*)(void*,void**,const WString*)>(0xdfd90)(object.data()+0x48,&nativeNode,&name);
                        require(found==(nativeNode!=field<void*>(object.data(),0x4c)),"Profile lookup existence differs");
                        if(found) require(value==field<unsigned>(nativeNode,40),"Profile lookup ID differs");else require(value==1234,"Missing profile lookup changed output");++lookupCount;
                    }
                    if(repeat%20==0) {
                        actual.appendNames(actualList);function<void(__thiscall*)(void*,void*)>(0xe8900)(object.data(),&nativeList);
                        const auto expected=nativeList.values();require(expected.size()==actualList.size() && expected.size()==nativeList.size,"Profile getNames append count differs");
                        for(unsigned i=0;i<expected.size();++i) require(expected[i].view()==actualList[i].view(),"Profile getNames order differs");++listComparisons;
                    }
                    // Empty selected name is present in these fixtures. Original
                    // getProfileID lookup executes; its CRT formatter is a boundary.
                    unsigned id=0;if(actual.findID(actual.currentName(),id)) {
                        alignas(4) unsigned char nativeID[28];function<String*(__thiscall*)(void*,String*)>(0xe4110)(object.data(),reinterpret_cast<String*>(nativeID));
                        auto* expected=reinterpret_cast<String*>(nativeID);require(actual.currentID().view()==expected->view(),"Profile current ID lookup differs");expected->~String();++currentIDs;
                    }
                }
            }
            // Cleanup only the exercised name tree; full cProfile destruction
            // also serializes/clears thirteen unrecovered global property stores.
            auto* map=object.data()+0x48;auto* head=field<void*>(map,4);void* result=nullptr;
            function<void(__thiscall*)(void*,void**,void*,void*)>(0xe75b0)(map,&result,field<void*>(head,0),head);
            release(head);field<WString>(object.data(),8).~WString();field<WString>(object.data(),0x24).~WString();pointer(0x25f738,nullptr);
            require(allocations.empty(),"Profile header fixture allocation leaked");
        }
        std::cout<<"{\"constructors\":100,\"header_loads\":"<<loads<<",\"lookups\":"<<lookupCount<<",\"name_lists\":"<<listComparisons<<",\"current_ids\":"<<currentIDs<<",\"xml_fixtures\":"<<documents.size()<<"}\n";
        VirtualFree(image,0,MEM_RELEASE);FreeLibrary(reference);
    } catch(const std::exception& error) { std::cerr<<error.what()<<'\n';if(image) VirtualFree(image,0,MEM_RELEASE);FreeLibrary(reference);return 1; }
    return 0;
}
