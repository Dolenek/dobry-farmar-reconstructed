#include "FileIndex.h"
#include "ResourceManager.h"
#include "XmlResource.h"
#include "Settings.h"
#include <windows.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
using namespace farmar::recovered;
namespace {
String path(const wchar_t* input) {
    int count=WideCharToMultiByte(CP_ACP,0,input,-1,nullptr,0,nullptr,nullptr);
    std::string output(count,'\0');
    WideCharToMultiByte(CP_ACP,0,input,-1,output.data(),count,nullptr,nullptr);
    return String(output.c_str());
}
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=3) return 2;
    unsigned checks=0;
    auto check=[&](bool result,const char* message) { if(!result) throw std::runtime_error(message); ++checks; };
    try {
        FileIndex files(path(argv[1]),String("pack")); files.mountCommon(path(argv[2]));
        {
            ResourceManager manager(files);
            bool rejected=false;
            try { ResourceManager duplicate(files); } catch(const char* text) { rejected=std::strcmp(text,"Object already exist.")==0; }
            check(rejected,"duplicate singleton accepted");
            auto* first=manager.getXML(String("SYS/SETTINGS.XML"));
            check(manager.getResourceName(first).view()=="sys/settings.xml","normalized reverse name");
            manager.releaseResource(String("SYS/SETTINGS.XML"));
            check(manager.cachedCount()==1,"name release incorrectly folds case");
            manager.releaseResource(String("sys/settings.xml"));
            check(manager.cachedCount()==0 && first->document().children().size()==1,"name release destroyed resource");
            auto* second=manager.getXML(String("sys/settings.xml"));
            check(second!=first,"detached object reused");
            delete first;
            check(manager.cachedCount()==1 && manager.getResourceName(second).view()=="sys/settings.xml","detached destructor evicted replacement");
            check(second->releaseReference() && second->referenceCount()==1,"XML release decremented count");
            manager.clear(); manager.clearUseless();
            check(manager.cachedCount()==1,"clear deleted positive references");
            auto writer=files.create(String("SYS/SETTINGS.XML"));
            const char replacement[]="<main><graphics width=\"1024\"/></main>";
            check(manager.cachedCount()==0,"file creation did not evict XML");
            writer->write(replacement,sizeof(replacement)-1); writer.reset();
            auto* third=manager.getXML(String("sys/settings.xml"));
            XmlNode* graphics=nullptr;
            check(third->document().section(String("graphics"),graphics) && graphics->attribute(WString(L"width")).view()==L"1024","new XML did not read common overwrite");
            XmlNode* oldGraphics=nullptr;
            check(second->document().section(String("graphics"),oldGraphics) && oldGraphics->attribute(WString(L"width")).view()==L"800","detached old XML changed");
            delete second; delete third;
            Settings settings(files,[&](const String& name) { return manager.loadSettingsXML(name); });
            unsigned width=0; int signedWidth=0;
            check(settings.get(String("graphics"),String("width"),signedWidth) && signedWidth==1024 && manager.cachedCount()==0,"managed settings load lifecycle");
            (void)width;
            auto* held=manager.getXML(String("sys/settings.xml"));
            held->addReference(); manager.deleteAllSound();
            check(manager.cachedCount()==1,"deleteAllSound removed XML");
        } // manager destroys cached XML despite positive reference count
        check(!resourceDestroyedCallback(),"manager destruction left singleton callback");
        ResourceManager fresh(files);
        check(fresh.cachedCount()==0,"singleton was not reset");
        std::cout << checks << '\n';
    } catch(const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
      catch(const char* error) { std::cerr << error << '\n'; return 1; }
}
