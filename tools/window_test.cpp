// Runs recovered window creation against real Win32, without a visible game UI.
#include "GameWindow.h"
#include "FileIndex.h"
#include "Settings.h"
#include <iostream>
#include <string>
#include <vector>
using namespace farmar::recovered;
namespace {
String ansi(const wchar_t* path) {
    int count=WideCharToMultiByte(CP_ACP,0,path,-1,nullptr,0,nullptr,nullptr);
    std::string result(count,'\0');
    WideCharToMultiByte(CP_ACP,0,path,-1,result.data(),count,nullptr,nullptr);
    return String(result.c_str());
}
}
int wmain(int argc,wchar_t** argv) {
    if (argc!=2) return 2;
    HWND window=nullptr;
    unsigned checks=0;
    auto check=[&](bool result,const char* message) {
        if (!result) throw std::runtime_error(message);
        ++checks;
    };
    try {
        FileIndex files(ansi(argv[1]),String("pack"));
        Settings settings(files);
        auto instance=GetModuleHandleW(nullptr);
        window=createGameWindow(instance,settings,WString());
        check(window && IsWindow(window),"create window");
        wchar_t title[128],name[128];
        GetWindowTextW(window,title,128); GetClassNameW(window,name,128);
        check(std::wstring(title)==L"Dobr\u00fd farm\u00e1\u0159" && std::wstring(name)==title,"Unicode title/class fallback");
        RECT outer;
        GetWindowRect(window,&outer);
        check(outer.right-outer.left==800 && outer.bottom-outer.top==600,"configured outer dimensions");
        check(!IsWindowVisible(window),"create helper does not show window");
        check((GetWindowLongW(window,GWL_STYLE)&0x00ffffff)==0x00ca0000,"original style bits");
        WNDCLASSEXW klass{}; klass.cbSize=sizeof(klass);
        check(GetClassInfoExW(instance,name,&klass) && klass.style==CS_CLASSDC && klass.cbClsExtra==0 &&
              klass.cbWndExtra==0 && klass.hIcon && klass.hCursor && klass.hbrBackground,"class resources/style");
        std::vector<int> calls;
        auto& target=windowTargets();
        target.acquireInput=[&]{calls.push_back(1);};
        target.releaseInput=[&]{calls.push_back(2);};
        target.pauseMain=[&](bool pause){calls.push_back(pause?3:4);};
        target.pauseSound=[&](bool pause){calls.push_back(pause?5:6);};
        target.minimizeMain=[&](bool minimize){calls.push_back(minimize?7:8);};
        target.character=[&](std::uint32_t character){calls.push_back(character);};
        gameWindowProcedure(nullptr,WM_ACTIVATE,0,0);
        check(calls==std::vector<int>({2,3,5}),"inactive order"); calls.clear();
        gameWindowProcedure(nullptr,WM_ACTIVATE,1,0);
        check(calls==std::vector<int>({1,4,6}),"active order"); calls.clear();
        gameWindowProcedure(nullptr,WM_ACTIVATE,0x10001,0);
        check(calls.empty(),"activate compares entire WPARAM");
        gameWindowProcedure(nullptr,WM_ACTIVATEAPP,2,0);
        check(calls==std::vector<int>({2,3,5}),"application active accepts only 1"); calls.clear();
        gameWindowProcedure(nullptr,WM_ACTIVATEAPP,1,0);
        check(calls==std::vector<int>({1,4,6}),"application activation order"); calls.clear();
        check(SendMessageW(window,WM_CHAR,0x159,0)==0 && calls==std::vector<int>({0x159}),"UTF-16 input forwarded"); calls.clear();
        check(gameWindowProcedure(nullptr,WM_MOUSEMOVE,0,0)==0 && calls.empty(),"mouse move consumed");
        check(gameWindowProcedure(nullptr,WM_SYSCOMMAND,SC_KEYMENU,0)==0 && calls.empty(),"system key menu consumed");
        gameWindowProcedure(nullptr,WM_SYSCOMMAND,SC_MINIMIZE,0);
        gameWindowProcedure(nullptr,WM_SYSCOMMAND,SC_RESTORE,0);
        gameWindowProcedure(nullptr,WM_SYSCOMMAND,SC_MAXIMIZE,0);
        check(calls==std::vector<int>({7,8,8}),"minimize/restore/maximize callbacks"); calls.clear();
        gameWindowProcedure(nullptr,WM_SYSCOMMAND,SC_MINIMIZE+1,0);
        check(calls.empty(),"system commands not masked");
        target={};
        DestroyWindow(window); window=nullptr;
        MSG message{};
        check(PeekMessageW(&message,nullptr,WM_QUIT,WM_QUIT,PM_REMOVE) && message.wParam==0,"destroy posts quit zero");
        unsigned ticks=0;
        processGameMessages([&]{++ticks; PostQuitMessage(42);});
        check(ticks==1,"idle process resumes message pump and exits on quit");
        // Test-owned class cleanup; createGameWindow itself retains original behavior.
        UnregisterClassW(name,instance); DeleteObject(klass.hbrBackground);
        std::cout << checks << '\n';
    } catch(const std::exception& error) {
        windowTargets()={};
        if (window) DestroyWindow(window);
        std::cerr << error.what() << '\n'; return 1;
    }
}
