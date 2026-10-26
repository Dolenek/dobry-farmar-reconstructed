#include "GameFileSystem.h"
#include "FileIndex.h"
#include "XmlDocument.h"
#include "StringConversion.h"
#include <shlobj.h>
#include <algorithm>
#include <stdexcept>
namespace farmar::recovered {
String mountGameCommonFiles(FileIndex& files,const String& explicitRoot,GameFileSystemServices services) {
    String root(explicitRoot);
    if(root.empty()) {
        const XmlDocument definition(files,String("sys/settings.xml"));XmlNode* game=nullptr;
        if(!definition.section(String("game"),game)) throw "app_directory not defined.";
        auto directory=toNarrow(game->attribute(WString(L"app_directory")));
        std::replace(directory.data(),directory.data()+directory.size(),'\\','/');
        if(directory.empty()) directory=toNarrow(game->attribute(WString(L"window_title")));
        if(directory.empty()) throw "app_directory not defined.";
        char common[MAX_PATH]{};
        if(FAILED(services.folderPath(nullptr,0x8023,nullptr,0,common))) throw std::runtime_error("Cannot resolve original shared application data directory");
        root=String(std::string(common)+"/"+std::string(directory.view())+"/");
    }
    std::replace(root.data(),root.data()+root.size(),'\\','/');
    if(root.size() && root.data()[root.size()-1]!='/') root=String(std::string(root.view())+"/");
    FileIndex::createDirectories(root);files.mountCommon(root);return root;
}
}
