#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "FileIndex.h"
#include "Settings.h"
#include <iostream>
#include <stdexcept>
#include <string>
using namespace farmar::recovered;
namespace {
String path(const wchar_t* text) {
    int count=WideCharToMultiByte(CP_ACP,0,text,-1,nullptr,0,nullptr,nullptr);
    std::string result(count, '\0');
    WideCharToMultiByte(CP_ACP,0,text,-1,result.data(),count,nullptr,nullptr);
    return String(result.c_str());
}
}
int wmain(int argc, wchar_t** argv) {
    if (argc!=3) return 2;
    unsigned checks=0;
    auto check=[&](bool result, const char* message) {
        if (!result) throw std::runtime_error(message);
        ++checks;
    };
    try {
        FileIndex files(path(argv[1]), String("pack"));
        files.mountCommon(path(argv[2]));
        Settings settings(files);
        int width=17,height=19;
        bool cursor=true;
        WString title;
        check(settings.get(String("graphics"),String("width"),width) && width==800,"integer width");
        check(settings.get(String("graphics"),String("height"),height) && height==600,"integer height");
        check(settings.get(String("graphics"),String("ShowSysCursor"),cursor) && !cursor,"numeric bool");
        check(settings.get(String("game"),String("window_title"),title) && title.view()==L"Dobr\u00fd farm\u00e1\u0159","Unicode title");
        WString missing(L"unchanged");
        check(!settings.get(String("graphics"),String("Width"),missing) && missing.view()==L"unchanged","case-sensitive miss leaves output");
        check(settings.get(String("graphics"),String("Width"),missing) && missing.empty(),"miss inserts empty property");
        settings.load(String("extra.xml"));
        check(settings.get(String("graphics"),String("width"),width) && width==1024,"load updates existing value");
        check(settings.get(String("graphics"),String("height"),height) && height==600,"load retains absent value");
        WString duplicate;
        check(settings.get(String("duplicate"),String("value"),duplicate) && duplicate.view()==L"last","duplicate section merge");
        settings.set(String("graphics"),String("width"),1280);
        settings.set(String("graphics"),String("ShowSysCursor"),true);
        settings.set(String("game"),String("ascii"),String("abc"));
        String narrow;
        check(settings.get(String("game"),String("ascii"),narrow) && narrow.view()=="abc","ASCII conversions");
        settings.save();
        check(files.isCommonExist(String("sys/settings.xml")),"save creates common overlay");
        Settings reloaded(files);
        check(reloaded.get(String("graphics"),String("width"),width) && width==1280,"saved integer reload");
        check(reloaded.get(String("graphics"),String("ShowSysCursor"),cursor) && cursor,"saved bool reload");
        check(reloaded.get(String("game"),String("window_title"),title) && title.view()==L"Dobr\u00fd farm\u00e1\u0159","saved Unicode reload");
        std::cout << checks << '\n';
    } catch(const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
