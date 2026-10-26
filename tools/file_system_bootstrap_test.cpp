#include "GameFileSystem.h"
#include "FileIndex.h"
#include "Settings.h"
#include "StringConversion.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstring>
#include <algorithm>
using namespace farmar::recovered;
namespace {
String folder;unsigned calls=0;bool fail=false;
HRESULT WINAPI sharedFolder(HWND window,int csidl,HANDLE token,DWORD flags,LPSTR output) {
    if(window || csidl!=0x8023 || token || flags) throw std::runtime_error("Original shared folder call arguments differ");
    ++calls;if(fail)return E_FAIL;std::strcpy(output,folder.c_str());return S_OK;
}
void require(bool test,const char* message) {if(!test)throw std::runtime_error(message);}
void write(const std::filesystem::path& file,const std::string& bytes) {std::filesystem::create_directories(file.parent_path());std::ofstream stream(file,std::ios::binary);stream<<bytes;require(bool(stream),"Fixture write failed");}
String ansi(const std::filesystem::path& path) {return toNarrow(WString(path.wstring()));}
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=2)return 2;
    try {
        const auto root=std::filesystem::absolute(argv[1])/std::to_string(GetCurrentProcessId());unsigned cases=0,overlays=0,rejected=0;
        for(unsigned n=0;n<6;++n) {
            const auto base=root/std::to_string(n),data=base/"Data",shared=base/"shared";std::filesystem::create_directories(data);
            folder=ansi(shared);calls=0;fail=n==5;
            const char* game=n==0 ? "app_directory=\"ConfiguredFarm\" window_title=\"Ignored\"":n==1 ? "app_directory=\"\" window_title=\"WindowFarm\"":n==2 ? "app_directory=\"\" window_title=\"\"":"app_directory=\"ConfiguredFarm\"";
            if(n!=4) write(data/"sys/settings.xml",std::string("<main>")+(n==3 ? "":"<game "+std::string(game)+"/>")+"<Sound Music=\"9\"/></main>");
            const auto expected=n==4 ? base/"explicit":shared/(n==1 ? "WindowFarm":"ConfiguredFarm");
            if(n<2 || n==4) {
                write(expected/"sys/settings.xml","<main><game app_directory=\"MustNotRedirectBaseBootstrap\"/><Sound Music=\"7\"/></main>");write(expected/"overlay.marker","shared");
            }
            FileIndex files(ansi(data),String("pack"));bool caught=false;String result;
            try {result=mountGameCommonFiles(files,n==4 ? ansi(expected):String(),GameFileSystemServices{&sharedFolder});}
            catch(const char*) {caught=true;}
            catch(const std::runtime_error&) {caught=true;}
            const bool negative=n==2 || n==3 || n==5;
            require(caught==negative,"Bootstrap exception branch differs");
            if(caught) ++rejected;
            else {
                String wanted=ansi(expected);std::string normal(wanted.view());std::replace(normal.begin(),normal.end(),char(92),'/');normal+='/';
                require(result.view()==normal,"Bootstrap physical directory differs");
                require(std::filesystem::is_directory(expected),"Bootstrap directory absent");
                require(files.isCommonExist(String("overlay.marker")),"Bootstrap did not mount common overlay");
                auto marker=files.open(String("overlay.marker"));char content[6]{};require(marker->read(content,6)==6 && !std::memcmp(content,"shared",6),"Common overlay read differs");++overlays;
                Settings settings(files);int music=0;require(settings.get(String("Sound"),String("Music"),music) && music==7,"Settings constructor did not observe mounted common settings");
            }
            require(calls==(n<2 || n==5 ? 1U:0U),"Shared folder API invocation count differs");++cases;
        }
        std::cout<<"{\"bootstrap_cases\":"<<cases<<",\"mounted_overlays\":"<<overlays<<",\"rejected_cases\":"<<rejected<<",\"native_constructor_executed\":false}\n";
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(const char* e){std::cerr<<e<<'\n';return 1;}
}
