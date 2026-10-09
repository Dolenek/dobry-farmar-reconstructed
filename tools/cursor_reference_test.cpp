// Original cursor/window coordinate methods are a test oracle only.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "GameCursor.h"
#include "OriginalArithmetic.h"
#include <array>
#include <cstring>
#include <float.h>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>
using namespace farmar::recovered;
namespace {
unsigned char* image=nullptr;unsigned scenario=0;
template<class T> T& field(void* p,unsigned offset) { return *reinterpret_cast<T*>(static_cast<unsigned char*>(p)+offset); }
template<class T> T function(unsigned rva) { return reinterpret_cast<T>(image+rva); }
void require(bool result,const char* error) { if(!result) { std::cerr<<error<<" scenario "<<scenario<<'\n';throw std::runtime_error(error); } }
LONG CALLBACK diagnose(EXCEPTION_POINTERS* exception) {
    if(exception->ExceptionRecord->ExceptionCode==EXCEPTION_ACCESS_VIOLATION) {
        std::cerr<<"Cursor AV scenario "<<scenario<<" RVA "<<std::hex
            <<reinterpret_cast<unsigned>(exception->ExceptionRecord->ExceptionAddress)-reinterpret_cast<unsigned>(image)
            <<" target "<<exception->ExceptionRecord->ExceptionInformation[1]<<'\n';
        TerminateProcess(GetCurrentProcess(),1);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
void write(unsigned rva,const void* value,unsigned count) { std::memcpy(image+rva,value,count); }
void pointer(unsigned rva,void* value) { write(rva,&value,4); }
struct Event { unsigned kind;std::array<unsigned,15> input;bool operator==(const Event& other) const { return kind==other.kind && input==other.input; } };
std::vector<Event> actualTrace,expectedTrace;std::vector<Event>* trace=&actualTrace;
WINDOWINFO windowReply{};POINT screenReply{};BOOL windowResult=TRUE,cursorResult=TRUE;
BOOL WINAPI getWindow(HWND window,PWINDOWINFO info) {
    require(window==reinterpret_cast<HWND>(0x1234),"Cursor window handle differs");
    Event call{0,{}};std::memcpy(call.input.data(),info,sizeof(WINDOWINFO));trace->push_back(call);
    *info=windowReply;return windowResult;
}
BOOL WINAPI getCursor(LPPOINT output) {
    Event call{1,{}};std::memcpy(call.input.data(),output,sizeof(POINT));trace->push_back(call);
    *output=screenReply;return cursorResult;
}
void same(const void* actual,const void* expected,unsigned count,const char* error) { require(!std::memcmp(actual,expected,count),error); }
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=2 && argc!=3) return 2;
    const auto reference=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES);if(!reference) return 2;
    HWND window=nullptr;
    try {
        if(argc==3) _control87(_PC_24,_MCW_PC);
        AddVectoredExceptionHandler(1,&diagnose);
        auto* loaded=reinterpret_cast<unsigned char*>(reference);
        auto* headers=reinterpret_cast<IMAGE_NT_HEADERS*>(loaded+reinterpret_cast<IMAGE_DOS_HEADER*>(loaded)->e_lfanew);
        image=static_cast<unsigned char*>(VirtualAlloc(nullptr,headers->OptionalHeader.SizeOfImage,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE));
        require(image && image!=loaded,"Cursor oracle private relocation failed");std::memcpy(image,loaded,headers->OptionalHeader.SizeOfImage);
        struct Fixup { unsigned rva,address; };const Fixup fixups[]={
#include "cursor_oracle_fixups.inc"
        };
        for(const auto& fixup:fixups) {
            require(field<unsigned>(image,fixup.rva)==fixup.address,"Cursor oracle operand differs");
            const auto value=reinterpret_cast<unsigned>(image)+fixup.address-0x400000;write(fixup.rva,&value,4);
        }
        pointer(0x1ee23c,reinterpret_cast<void*>(&getWindow));pointer(0x1ee258,reinterpret_cast<void*>(&getCursor));
        FlushInstructionCache(GetCurrentProcess(),image,headers->OptionalHeader.SizeOfImage);
        std::array<unsigned char,680> graphics{};std::array<void*,20> table{};
        table[14]=image+0x112330;table[19]=image+0x10d260;
        field<void*>(graphics.data(),0)=table.data();field<HWND>(graphics.data(),8)=reinterpret_cast<HWND>(0x1234);
        pointer(0x25ff8c,graphics.data());
        std::array<unsigned char,24> logic{};GameCursor cursor(reinterpret_cast<HWND>(0x1234),CursorServices{&getWindow,&getCursor});
        std::mt19937 random(0x43555253);unsigned failures=0,aliased=0;
        const float offsets[]={0,0.5f,-120,231.875f,16777216.f};
        const float scales[]={1,0.5f,1.333333373f,-2,0.001f,0};
        for(scenario=0;scenario<20000;++scenario) {
            windowReply={};windowReply.cbSize=random();
            auto values=reinterpret_cast<LONG*>(&windowReply.rcWindow);
            for(unsigned i=0;i<8;++i) values[i]=static_cast<LONG>(random());
            screenReply={static_cast<LONG>(random()),static_cast<LONG>(random())};
            windowResult=scenario%7!=0;cursorResult=scenario%11!=0;
            failures+=!windowResult || !cursorResult;
            const bool fullscreen=scenario%2!=0;DesktopImagePlacement placement;
            placement.offsetX=offsets[scenario%5];placement.offsetY=offsets[(scenario/5)%5];
            placement.scaleX=scales[scenario%6];placement.scaleY=scales[(scenario/6)%6];
            field<unsigned char>(graphics.data(),0x110)=fullscreen;
            field<float>(graphics.data(),0x248)=placement.scaleX;field<float>(graphics.data(),0x24c)=placement.scaleY;
            field<float>(graphics.data(),0x250)=placement.offsetX;field<float>(graphics.data(),0x254)=placement.offsetY;
            // Direct getWindowBounds must retain the caller's output on failure.
            CursorBounds actual{11,22,33,44},expected=actual;
            actualTrace.clear();expectedTrace.clear();trace=&actualTrace;
            cursorWindowBounds(reinterpret_cast<HWND>(0x1234),fullscreen,actual,CursorServices{&getWindow,&getCursor});
            trace=&expectedTrace;function<void(__thiscall*)(void*,float*)>(0x112330)(graphics.data(),expected.data());
            same(actual.data(),expected.data(),16,"Cursor window bounds differ");require(actualTrace==expectedTrace,"Window query trace differs");
            actualTrace.clear();expectedTrace.clear();trace=&actualTrace;cursor.update(fullscreen,placement);
            trace=&expectedTrace;function<void(__thiscall*)(void*)>(0x130290)(logic.data());
            same(cursor.position().data(),logic.data()+0x10,8,"Cursor position differs");
            same(&cursorSharedState().window,image+0x260570,60,"Shared window information differs");
            same(&cursorSharedState().screen,image+0x2605f4,8,"Shared screen cursor differs");
            require(actualTrace==expectedTrace,"Cursor Win32 call order/input differs");
            require(function<const float*(__thiscall*)(void*)>(0x131a40)(logic.data())==reinterpret_cast<float*>(logic.data()+0x10),"Native cursor getter pointer differs");
            // Also test standalone transform with distinct and aliased buffers.
            CursorPoint input{float(static_cast<int>(random())),float(static_cast<int>(random()))};CursorPoint transformed{},native{};
            cursorToGame(input,transformed,fullscreen,placement);
            function<void(__thiscall*)(void*,const float*,float*)>(0x10d260)(graphics.data(),input.data(),native.data());
            same(transformed.data(),native.data(),8,"Separate cursor transform differs");
            auto actualAlias=input,expectedAlias=input;cursorToGame(actualAlias,actualAlias,fullscreen,placement);
            function<void(__thiscall*)(void*,const float*,float*)>(0x10d260)(graphics.data(),expectedAlias.data(),expectedAlias.data());
            same(actualAlias.data(),expectedAlias.data(),8,"Aliased cursor transform differs");++aliased;
        }
        // Physical Win32 API check in an owned hidden window, without moving
        // the user's cursor or changing their display mode.
        window=CreateWindowExW(0,L"STATIC",L"Recovered cursor verification",WS_POPUP,32,48,320,200,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
        require(window!=nullptr,"Cursor diagnostic window failed");
        WINDOWINFO expectedInfo{};expectedInfo.cbSize=sizeof(expectedInfo);require(GetWindowInfo(window,&expectedInfo)!=FALSE,"Physical window information failed");
        cursorSharedState()={};CursorBounds bounds{-1,-1,-1,-1};cursorWindowBounds(window,false,bounds);
        const CursorBounds expectedBounds{float(expectedInfo.rcClient.left),float(expectedInfo.rcClient.top),float(expectedInfo.rcClient.right),float(expectedInfo.rcClient.bottom)};
        same(bounds.data(),expectedBounds.data(),16,"Physical client bounds differ");
        GameCursor physical(window);physical.update(false,DesktopImagePlacement{});
        const auto screen=cursorSharedState().screen;
        const CursorPoint expectedPoint{static_cast<float>(originalSubtract(double(screen.x),bounds[0])),static_cast<float>(originalSubtract(double(screen.y),bounds[1]))};
        same(physical.position().data(),expectedPoint.data(),8,"Physical screen-to-client cursor differs");
        DestroyWindow(window);window=nullptr;
        std::cout<<"{\"frames\":20000,\"bounds\":20000,\"transforms\":40000,\"aliased\":"<<aliased<<",\"failure_frames\":"<<failures<<",\"physical_windows\":1}\n";
        VirtualFree(image,0,MEM_RELEASE);FreeLibrary(reference);
    } catch(const std::exception& error) { std::cerr<<error.what()<<" scenario "<<scenario<<'\n';if(window) DestroyWindow(window);if(image) VirtualFree(image,0,MEM_RELEASE);FreeLibrary(reference);return 1; }
    return 0;
}
