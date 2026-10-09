#pragma once
// Original property tree/dispatch instructions are test oracles only. Numeric
// stream and ACP conversion boundaries deliberately use the diagnostic CRT.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ProfileProperties.h"
#include "StringConversion.h"
#include <array>
#include <cstdlib>
#include <cstring>
#include <float.h>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
using namespace farmar::recovered;
namespace {
unsigned char* image=nullptr;unsigned scenario=0,kind=0;std::set<void*> allocations;
template<class T> T& field(void* p,unsigned offset) { return *reinterpret_cast<T*>(static_cast<unsigned char*>(p)+offset); }
template<class T> T function(unsigned rva) { return reinterpret_cast<T>(image+rva); }
void require(bool result,const char* error) { if(!result) { std::cerr<<error<<" scenario "<<scenario<<" type "<<kind<<'\n';throw std::runtime_error(error); } }
void redirect(unsigned rva,void* target) { unsigned char bytes[5]={0xe9};const unsigned offset=reinterpret_cast<unsigned>(target)-reinterpret_cast<unsigned>(image+rva+5);std::memcpy(bytes+1,&offset,4);std::memcpy(image+rva,bytes,5); }
LONG CALLBACK diagnose(EXCEPTION_POINTERS* e) {
    if(e->ExceptionRecord->ExceptionCode==EXCEPTION_ACCESS_VIOLATION) {
        std::cerr<<"Property AV scenario "<<scenario<<" type "<<kind<<" RVA "<<std::hex
            <<reinterpret_cast<unsigned>(e->ExceptionRecord->ExceptionAddress)-reinterpret_cast<unsigned>(image)
            <<" target "<<e->ExceptionRecord->ExceptionInformation[1]<<" EIP "<<e->ContextRecord->Eip<<" image "<<reinterpret_cast<unsigned>(image)<<" main "<<reinterpret_cast<unsigned>(GetModuleHandleW(nullptr))<<'\n';TerminateProcess(GetCurrentProcess(),1);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
void* __cdecl allocate(unsigned size) { auto* p=std::malloc(size);if(p) allocations.insert(p);return p; }
void __cdecl release(void* p) { allocations.erase(p);std::free(p); }
String* __fastcall assign(String* self,void*,const String* value,unsigned first,unsigned count) { return &self->assign(*value,first,count); }
WString* __fastcall assignWide(WString* self,void*,const WString* value,unsigned first,unsigned count) { return &self->assign(*value,first,count); }
int __fastcall compare(String* self,void*,unsigned first,unsigned count,const char* value,unsigned size) { return self->compare(first,count,value,size); }
WString* __fastcall copyWide(WString* self,void*,const WString* value) { return new(self) WString(*value); }
String* __fastcall narrow(WString* self,void*,String* result) { return new(result) String(toNarrow(*self)); }
String* __fastcall assignBytes(String* self,void*,const char* text,unsigned size) { return &self->assign(text,size); }
WString* __fastcall wide(String* self,void*,WString* result) { return new(result) WString(toWide(*self)); }
WString* __cdecl concatenate(WString* output,const WString* text,const wchar_t* suffix) { new(output) WString(*text);output->append(WString(suffix));return output; }
unsigned registrations=0;
int __cdecl registerExit(void(__cdecl*)()) { ++registrations;return 0; }
std::wostringstream nativeOutput;void* expectedStream=nullptr;
void* __cdecl writeLiteral(void* stream,const wchar_t* text) { require(stream==expectedStream,"Native serializer stream pointer differs");nativeOutput<<text;return stream; }
void* __cdecl writeWide(void* stream,const WString* text) { require(stream==expectedStream,"Native WString serializer stream pointer differs");nativeOutput.write(text->data(),text->size());return stream; }
template<class T> void* __fastcall writeNumeric(void* stream,void*,T value) { require(stream==expectedStream,"Native numeric serializer stream pointer differs");nativeOutput<<value;return stream; }
unsigned __fastcall packColor(Color* self,void*) { return self->toARGB(); }
template<class T> T* __cdecl typed(T* output,const WString* text) { if(!text->empty()) { std::wistringstream stream(text->c_str());stream>>*output; }return output; }
Color* __cdecl color(Color* output,const WString* text) { if(!text->empty()) { unsigned packed=0;std::wistringstream stream(text->c_str());stream>>std::hex>>packed;*output=Color::fromARGB(packed); }return output; }
std::array<float,2>* __cdecl vector(std::array<float,2>* output,const WString* text) { std::wistringstream stream(text->c_str());stream>>(*output)[0];stream.ignore(1);stream>>(*output)[1];return output; }
struct Store { unsigned base,helper,size;const char* name;void* head=nullptr; };
Store stores[]={
    {0x25ff38,0x106a0,4,"Float"},{0x260464,0xe9a70,8,"Double"},{0x25ff20,0x10770,4,"Int"},
    {0x25ff44,0x10770,4,"UInt"},{0x260470,0xe9b40,4,"Long"},{0x26047c,0xe9b40,4,"ULong"},
    {0x260488,0xe9c00,8,"LongLong"},{0x260458,0xe9990,16,"DXColor"},{0x25ff2c,0x105e0,1,"Bool"},
    {0x260494,0xe9cc0,2,"UInt16"},{0x260434,0xe98c0,8,"Vector2"},{0x260440,0xeac20,28,"String"},
    {0x26044c,0xeb1d0,28,"WString"}};
template<class T> bool equal(const T& a,const T& b) { return std::memcmp(&a,&b,sizeof(T))==0; }
template<> bool equal(const String& a,const String& b) { return a.view()==b.view(); }
template<> bool equal(const WString& a,const WString& b) { return a.view()==b.view(); }
template<class T> void collect(void* node,unsigned nil,ProfileProperties::Values<T>& result) {
    if(field<unsigned char>(node,nil)) return;
    const unsigned keyOffset=(kind==1 || kind==6) ? 16:12;const unsigned valueOffset=(kind==1 || kind==6) ? 48:40;
    collect<T>(field<void*>(node,0),nil,result);result.emplace(std::string(field<String>(node,keyOffset).view()),field<T>(node,valueOffset));collect<T>(field<void*>(node,8),nil,result);
}
unsigned comparisons=0,accesses=0,dispatches=0,clears=0,serializations=0;std::uint64_t serializedUnits=0;
template<class T> void check(const ProfileProperties::Values<T>& actual) {
    const auto& store=stores[kind];const unsigned valueOffset=(kind==1 || kind==6) ? 48:40;
    ProfileProperties::Values<T> expected;collect<T>(field<void*>(store.head,4),valueOffset+1+((store.size+3)&~3U),expected);
    if(actual.size()!=expected.size()) {
        std::cerr<<"Map sizes actual "<<actual.size()<<" native "<<expected.size()<<'\n';
        for(const auto& row:actual) if(!expected.count(row.first)) std::cerr<<"Source-only "<<row.first<<'\n';
        for(const auto& row:expected) if(!actual.count(row.first)) std::cerr<<"Native-only "<<row.first<<'\n';
    }
    require(actual.size()==expected.size() && actual.size()==field<unsigned>(image,store.base+8),"Property map size differs");
    auto at=expected.begin();for(const auto& entry:actual) { require(entry.first==at->first && equal<T>(entry.second,at->second),"Property key ordering or value differs");++at; }++comparisons;
}
void checkAll(const ProfileProperties& actual) {
    kind=0;check(actual.floats);kind=1;check(actual.doubles);kind=2;check(actual.ints);kind=3;check(actual.uints);
    kind=4;check(actual.longs);kind=5;check(actual.ulongs);kind=6;check(actual.longLongs);kind=7;check(actual.colors);
    kind=8;check(actual.booleans);kind=9;check(actual.uint16s);kind=10;check(actual.vectors);kind=11;check(actual.strings);kind=12;check(actual.wideStrings);
}
template<class T> void access(ProfileProperties::Values<T>& actual,const String& key,const T& value,bool write) {
    auto& a=actual[std::string(key.view())];auto* b=function<T*(__thiscall*)(void*,const String*)>(stores[kind].helper)(image+stores[kind].base,&key);
    require(equal<T>(a,*b),"Default insertion or existing property value differs");if(write) { a=value;*b=value; }++accesses;
}

void initializePropertyOracle(HMODULE reference,bool pc24) {
        if(pc24) _control87(_PC_24,_MCW_PC);AddVectoredExceptionHandler(1,&diagnose);
        auto* loaded=reinterpret_cast<unsigned char*>(reference);auto* header=reinterpret_cast<IMAGE_NT_HEADERS*>(loaded+reinterpret_cast<IMAGE_DOS_HEADER*>(loaded)->e_lfanew);
        image=static_cast<unsigned char*>(VirtualAlloc(nullptr,header->OptionalHeader.SizeOfImage,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE));
        require(image && image!=loaded,"Property oracle relocation failed");std::memcpy(image,loaded,header->OptionalHeader.SizeOfImage);
        struct Fixup { unsigned rva,address; };const Fixup fixups[]={
#ifdef FARMAR_PROFILE_LIFECYCLE
#include "profile_lifecycle_oracle_fixups.inc"
#else
#include "profile_properties_oracle_fixups.inc"
#endif
        };
        for(const auto& fixup:fixups) { require(field<unsigned>(image,fixup.rva)==fixup.address,"Property oracle operand differs");field<unsigned>(image,fixup.rva)=reinterpret_cast<unsigned>(image)+fixup.address-0x400000; }
        redirect(0x17edb1,reinterpret_cast<void*>(&allocate));redirect(0x1803af,reinterpret_cast<void*>(&release));
        redirect(0x16d0,reinterpret_cast<void*>(&assign));redirect(0x17c0,reinterpret_cast<void*>(&assignWide));redirect(0x3960,reinterpret_cast<void*>(&compare));
        redirect(0x1c70,reinterpret_cast<void*>(&copyWide));redirect(0xda20,reinterpret_cast<void*>(&narrow));
        redirect(0xd95c0,reinterpret_cast<void*>(&typed<float>));redirect(0xd9a90,reinterpret_cast<void*>(&typed<double>));
        redirect(0xd9720,reinterpret_cast<void*>(&typed<int>));redirect(0xd97d0,reinterpret_cast<void*>(&typed<unsigned>));
        redirect(0xd9880,reinterpret_cast<void*>(&typed<int>));redirect(0xd94a0,reinterpret_cast<void*>(&typed<unsigned>));
        redirect(0xd9930,reinterpret_cast<void*>(&typed<std::int64_t>));redirect(0xd93b0,reinterpret_cast<void*>(&color));
        redirect(0xd9670,reinterpret_cast<void*>(&typed<bool>));redirect(0xd99e0,reinterpret_cast<void*>(&typed<std::uint16_t>));redirect(0xd9240,reinterpret_cast<void*>(&vector));
        redirect(0x19c0,reinterpret_cast<void*>(&assignBytes));redirect(0xdaf0,reinterpret_cast<void*>(&wide));
        redirect(0xe0280,reinterpret_cast<void*>(&concatenate));redirect(0x180aed,reinterpret_cast<void*>(&registerExit));
        redirect(0xae240,reinterpret_cast<void*>(&writeLiteral));redirect(0xdf0a0,reinterpret_cast<void*>(&writeWide));
        redirect(0xdeaa0,reinterpret_cast<void*>(&writeNumeric<float>));redirect(0xdf4f0,reinterpret_cast<void*>(&writeNumeric<double>));
        redirect(0xa8be0,reinterpret_cast<void*>(&writeNumeric<int>));redirect(0x6400,reinterpret_cast<void*>(&writeNumeric<unsigned>));
        redirect(0xdf6b0,reinterpret_cast<void*>(&writeNumeric<int>));redirect(0xde930,reinterpret_cast<void*>(&writeNumeric<unsigned>));
        redirect(0xdf380,reinterpret_cast<void*>(&writeNumeric<std::int64_t>));redirect(0xde7c0,reinterpret_cast<void*>(&writeNumeric<bool>));
        redirect(0xded80,reinterpret_cast<void*>(&writeNumeric<std::uint16_t>));redirect(0x455e0,reinterpret_cast<void*>(&packColor));
        for(auto& store:stores) {
            const unsigned valueOffset=(&store==&stores[1] || &store==&stores[6]) ? 48:40;
            const unsigned rounded=(store.size+3)&~3U;const unsigned size=(valueOffset+2+rounded+7)&~7U;store.head=allocate(size);std::memset(store.head,0,size);
            field<void*>(store.head,0)=field<void*>(store.head,4)=field<void*>(store.head,8)=store.head;
            field<unsigned char>(store.head,valueOffset+rounded)=field<unsigned char>(store.head,valueOffset+1+rounded)=1;
            field<unsigned>(image,store.base)=0;field<void*>(image,store.base+4)=store.head;field<unsigned>(image,store.base+8)=0;
        }
        FlushInstructionCache(GetCurrentProcess(),image,header->OptionalHeader.SizeOfImage);
}
}
