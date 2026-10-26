#include "GameApplication.h"
#include "StringConversion.h"
#include <shellapi.h>
#include <stdexcept>
using namespace farmar::recovered;
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,PWSTR,int) {
    try {
        if(GameApplication::activateExisting(instance)) return 0;
        GameApplicationOptions options;int count=0;auto** arguments=CommandLineToArgvW(GetCommandLineW(),&count);
        if(!arguments) throw std::runtime_error("Cannot parse application arguments");
        for(int i=1;i<count;++i) {
            const std::wstring arg(arguments[i]);
            if((arg==L"--data" || arg==L"--common-dir") && i+1<count) {
                const auto value=toNarrow(WString(arguments[++i]));if(arg==L"--data") options.dataDirectory=value;else options.commonDirectory=value;
            } else {LocalFree(arguments);throw std::invalid_argument("Expected --data PATH or --common-dir PATH");}
        }
        LocalFree(arguments);GameApplication application(instance,options);application.run();return 0;
    } catch(const wchar_t* message) {MessageBoxW(nullptr,message,L"Dobrý farmář",MB_OK|MB_ICONERROR);}
      catch(const char* message) {MessageBoxA(nullptr,message,"Dobrý farmář",MB_OK|MB_ICONERROR);}
      catch(const std::exception& error) {MessageBoxA(nullptr,error.what(),"Dobrý farmář",MB_OK|MB_ICONERROR);}
    return 1;
}
