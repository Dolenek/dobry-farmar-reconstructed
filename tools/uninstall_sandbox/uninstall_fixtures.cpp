// Entirely owned fixture trees and private hive handles; no real install keys.
#include "uninstall_fixtures.h"
#include <fstream>
#include <stdexcept>
#include <string>
namespace fs=std::filesystem;
namespace {
void need(bool ok,const char* text) {if(!ok)throw std::runtime_error(text);}
void file(const fs::path& path) {fs::create_directories(path.parent_path());std::ofstream stream(path,std::ios::binary);stream<<"owned-uninstall-fixture";need(bool(stream),"Write owned uninstall fixture");}
void key(HKEY hive,const wchar_t* path,bool value) {
    HKEY result=nullptr;need(RegCreateKeyExW(hive,path,0,nullptr,0,KEY_ALL_ACCESS,nullptr,&result,nullptr)==ERROR_SUCCESS,"Create private uninstall fixture key");
    if(value){DWORD marker=91;need(RegSetValueExW(result,L"fixture",0,REG_DWORD,reinterpret_cast<BYTE*>(&marker),4)==ERROR_SUCCESS,"Seed private uninstall fixture value");}RegCloseKey(result);
}
bool exists(HKEY hive,const wchar_t* path) {HKEY result=nullptr;const auto status=RegOpenKeyExW(hive,path,0,KEY_READ,&result);if(status==ERROR_SUCCESS)RegCloseKey(result);else need(status==ERROR_FILE_NOT_FOUND,"Unexpected private fixture registry result");return status==ERROR_SUCCESS;}
void string(HKEY hive,const wchar_t* path,const wchar_t* name,const std::wstring& value) {
    HKEY result=nullptr;need(RegCreateKeyExW(hive,path,0,nullptr,0,KEY_ALL_ACCESS,nullptr,&result,nullptr)==ERROR_SUCCESS,"Create private string fixture key");
    need(RegSetValueExW(result,name,0,REG_SZ,reinterpret_cast<const BYTE*>(value.c_str()),DWORD((value.size()+1)*2))==ERROR_SUCCESS,"Seed private string fixture");RegCloseKey(result);
}
struct Scenario {
    bool missing=false,named=false,menuNonempty=false,player=false,playerNonempty=false,missingPlayerMenu=false,subkey=false,emptyMenuValue=false,valuesOnly=false,storedLanguage=false,invalidLanguage=false;
    explicit Scenario(const std::wstring& name) {
        if(name==L"empty-fallback"||name==L"bootstrap-copy-busy-a"){}
        else if(name==L"missing-registration")missing=true;
        else if(name==L"named-empty")named=true;
        else if(name==L"named-nonempty")named=menuNonempty=true;
        else if(name==L"player-empty")named=player=true;
        else if(name==L"player-nonempty")named=player=playerNonempty=true;
        else if(name==L"missing-player-menu")player=missingPlayerMenu=true;
        else if(name==L"subkey-keeps-virtualfarm")subkey=true;
        else if(name==L"empty-menu-value")emptyMenuValue=true;
        else if(name==L"registry-values-only")valuesOnly=true;
        else if(name==L"stored-language")storedLanguage=true;
        else if(name==L"invalid-stored-language")storedLanguage=invalidLanguage=true;
        else throw std::runtime_error("Unknown owned uninstall fixture scenario");
    }
    fs::path menu()const{return named?fs::path(L"Vendor")/L"Vlastní farmář":fs::path(L"Dobrý farmář");}
};
std::string quote(const std::wstring& value) {
    std::string out="\"";const char digits[]="0123456789abcdef";
    for(auto c:value)if(c=='"'||c=='\\'){out+='\\';out+=char(c);}else if(c<32||c>=127){out+="\\u";for(int shift=12;shift>=0;shift-=4)out+=digits[(c>>shift)&15];}else out+=char(c);
    return out+'"';
}
void snapshot(std::ofstream& stream,HKEY hive,const std::wstring& hiveName,const std::wstring& path) {
    HKEY opened=nullptr;need(RegOpenKeyExW(hive,path.c_str(),0,KEY_READ,&opened)==ERROR_SUCCESS,"Read private snapshot key");
    stream<<"{\"hive\":"<<quote(hiveName)<<",\"key\":"<<quote(path)<<",\"values\":[";bool first=true;
    for(DWORD i=0;;++i){wchar_t name[1024]{};DWORD chars=1024,type=0,bytes=8192;BYTE data[8192]{};const auto status=RegEnumValueW(opened,i,name,&chars,nullptr,&type,data,&bytes);if(status==ERROR_NO_MORE_ITEMS)break;need(status==ERROR_SUCCESS,"Enumerate private snapshot values");if(!first)stream<<',';first=false;stream<<"{\"name\":"<<quote(name)<<",\"type\":"<<type<<",\"data_hex\":\"";const char digits[]="0123456789abcdef";for(DWORD n=0;n<bytes;++n)stream<<digits[data[n]>>4]<<digits[data[n]&15];stream<<"\"}";}
    stream<<"]}\n";
    for(DWORD i=0;;++i){wchar_t name[1024]{};DWORD chars=1024;const auto status=RegEnumKeyExW(opened,i,name,&chars,nullptr,nullptr,nullptr,nullptr);if(status==ERROR_NO_MORE_ITEMS)break;need(status==ERROR_SUCCESS,"Enumerate private snapshot keys");snapshot(stream,hive,hiveName,path.empty()?name:path+L"\\"+name);}
    RegCloseKey(opened);
}
void files(const fs::path& root,const char* stage) {
    std::ofstream report(root/(std::string("fixture-files-")+stage+".jsonl"));const char digits[]="0123456789abcdef";
    for(const auto name:{L"payload",L"shell"}) {
        const auto base=root/name;if(!fs::exists(base))continue;std::vector<fs::path> paths{base};for(const auto& entry:fs::recursive_directory_iterator(base))paths.push_back(entry.path());
        for(const auto& path:paths){const auto attributes=GetFileAttributesW(path.c_str());need(attributes!=INVALID_FILE_ATTRIBUTES&&!(attributes&FILE_ATTRIBUTE_REPARSE_POINT),"Owned fixture must not contain reparse points");report<<"{\"path\":"<<quote(fs::relative(path,root).wstring())<<",\"attributes\":"<<attributes<<",\"bytes_hex\":\"";if(fs::is_regular_file(path)){std::ifstream input(path,std::ios::binary);for(int c=input.get();c!=EOF;c=input.get())report<<digits[(c>>4)&15]<<digits[c&15];}report<<"\"}\n";}
    }
}
void registrySnapshot(const fs::path& root,HKEY cu,HKEY lm,const char* stage) {std::ofstream report(root/(std::string("private-registry-")+stage+".jsonl"));snapshot(report,cu,L"HKCU",L"");snapshot(report,lm,L"HKLM",L"");}
}
void seedUninstallFixture(const fs::path& root,HKEY cu,HKEY lm,const std::wstring& name) {
    const Scenario scenario(name);
    file(root/L"payload"/L"Uninstall.exe");file(root/L"payload"/L"Data"/L"nested"/L"fixture.dat");file(root/L"payload"/L"game-marker.txt");
    const auto programs=root/L"shell"/L"user"/L"programs",desktop=root/L"shell"/L"user"/L"desktop";
    const auto hidden=root/L"payload"/L"Data"/L"hidden.dat";file(hidden);need(SetFileAttributesW(hidden.c_str(),FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_READONLY)!=FALSE,"Seed hidden/read-only payload");
    file(programs/scenario.menu()/L"game.lnk");file(programs/scenario.menu()/L"second.lnk");file(programs/L"unrelated.txt");
    if(scenario.menuNonempty)file(programs/scenario.menu()/L"keep.txt");
    for(const auto name:{L"Dobrý farmář.lnk",L"Špidla Data Processing, s.r.o..lnk",L"Hry zdarma ke stažení.lnk",L"Pohádky zdarma ke stažení.lnk"})file(desktop/name);
    file(desktop/L"unrelated.lnk");file(root/L"shell"/L"common"/L"programs"/L"unrelated.lnk");file(root/L"shell"/L"common"/L"desktop"/L"unrelated.lnk");
    if(!scenario.missing){key(cu,L"Software\\Alawar\\VirtualFarm",scenario.valuesOnly);key(lm,L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Dobrý farmář\\nested",true);}
    if(scenario.named)string(cu,L"Software\\Alawar\\VirtualFarm",L"Start Menu Folder",scenario.menu().wstring());
    if(scenario.emptyMenuValue)string(cu,L"Software\\Alawar\\VirtualFarm",L"Start Menu Folder",L"");
    if(scenario.storedLanguage)string(cu,L"Software\\Alawar\\VirtualFarm",L"Installer Language",scenario.invalidLanguage?L"65535":L"1029");
    if(scenario.subkey)key(cu,L"Software\\Alawar\\VirtualFarm\\keep-child",true);
    if(scenario.player) {
        string(cu,L"Software\\Alawar\\VirtualFarm",L"PlayerName",L"FixturePlayer");
        if(!scenario.missingPlayerMenu){string(cu,L"Software\\Alawar\\FixturePlayer",L"Start Menu Folder",L"Player Menu");file(programs/L"Player Menu"/L"Games"/L"Dobrý farmář.lnk");if(scenario.playerNonempty)file(programs/L"Player Menu"/L"Games"/L"unrelated.lnk");}
        else file(programs/L"Unresolved Player"/L"Games"/L"Dobrý farmář.lnk");
    }
    key(cu,L"Software\\UnrelatedFixture",true);key(lm,L"Software\\UnrelatedFixture",true);
    files(root,"before");registrySnapshot(root,cu,lm,"before");
}
void recordUninstallFixtureBefore(const fs::path& root,HKEY cu,HKEY lm) {files(root,"before");registrySnapshot(root,cu,lm,"before");}
void verifyUninstallFixture(const fs::path& root,HKEY cu,HKEY lm,const std::wstring& name) {
    const Scenario scenario(name);
    need(!fs::exists(root/L"payload"),"Uninstall did not remove owned payload recursively");
    const auto programs=root/L"shell"/L"user"/L"programs",desktop=root/L"shell"/L"user"/L"desktop";
    need(fs::exists(programs/scenario.menu())==scenario.menuNonempty,"Uninstall menu folder retention differs");
    need(!fs::exists(programs/scenario.menu()/L"game.lnk")&&!fs::exists(programs/scenario.menu()/L"second.lnk"),"Uninstall did not remove fixture menu links");
    if(scenario.named)need(fs::exists(programs/L"Vendor")==scenario.menuNonempty,"Uninstall empty-parent traversal differs");
    if(scenario.menuNonempty)need(fs::is_regular_file(programs/scenario.menu()/L"keep.txt"),"Uninstall removed non-link menu file");
    for(const auto name:{L"Dobrý farmář.lnk",L"Špidla Data Processing, s.r.o..lnk",L"Hry zdarma ke stažení.lnk",L"Pohádky zdarma ke stažení.lnk"})need(!fs::exists(desktop/name),"Uninstall did not remove fixture desktop link");
    for(const auto& path:{programs/L"unrelated.txt",desktop/L"unrelated.lnk",root/L"shell"/L"common"/L"programs"/L"unrelated.lnk",root/L"shell"/L"common"/L"desktop"/L"unrelated.lnk"})need(fs::is_regular_file(path),"Uninstall removed unrelated fixture");
    // NSIS2.28 /ifempty checks child keys, not values. Values alone are deleted.
    need(exists(cu,L"Software\\Alawar\\VirtualFarm")==scenario.subkey,"Uninstall /ifempty subkey-only semantics differ");
    if(scenario.subkey)need(exists(cu,L"Software\\Alawar\\VirtualFarm\\keep-child"),"Uninstall removed protected private subkey");
    need(!exists(lm,L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Dobrý farmář"),"Uninstall did not recursively remove private HKLM uninstall key");
    need(exists(cu,L"Software\\UnrelatedFixture")&&exists(lm,L"Software\\UnrelatedFixture"),"Uninstall removed unrelated private registry key");
    if(scenario.player&&!scenario.missingPlayerMenu) {
        need(!fs::exists(programs/L"Player Menu"/L"Games"/L"Dobrý farmář.lnk"),"Uninstall did not remove player game link");
        need(fs::is_directory(programs/L"Player Menu"),"Uninstall removed player menu parent");
        need(fs::exists(programs/L"Player Menu"/L"Games")==scenario.playerNonempty,"Uninstall player Games folder retention differs");
        if(scenario.playerNonempty)need(fs::is_regular_file(programs/L"Player Menu"/L"Games"/L"unrelated.lnk"),"Uninstall removed unrelated player link");
        need(exists(cu,L"Software\\Alawar\\FixturePlayer"),"Uninstall removed player registration");
    }
    if(scenario.missingPlayerMenu)need(fs::is_regular_file(programs/L"Unresolved Player"/L"Games"/L"Dobrý farmář.lnk"),"Uninstall guessed missing player menu");
    files(root,"after");registrySnapshot(root,cu,lm,"after");
    std::ofstream report(root/L"uninstall-fixture-result.json");report<<"{\"case\":\"";for(auto c:name)report<<char(c);report<<"\",\"payload_removed\":true,\"menu_effects_verified\":true,\"four_desktop_links_removed\":true,\"unrelated_files_retained\":true,\"hkcu_subkey_only_ifempty_verified\":true,\"hklm_tree_removed_or_absent\":true,\"unrelated_keys_retained\":true,\"player_effects_verified\":"<<(scenario.player?"true":"false")<<"}\n";
}
void verifyCancelledUninstallFixture(const fs::path& root,HKEY cu,HKEY lm,const std::wstring& name) {
    const Scenario scenario(name);need(fs::is_regular_file(root/L"payload"/L"Data"/L"nested"/L"fixture.dat"),"Cancelled uninstall removed payload");
    need(fs::is_regular_file(root/L"shell"/L"user"/L"programs"/scenario.menu()/L"game.lnk"),"Cancelled uninstall removed menu link");
    need(exists(cu,L"Software\\Alawar\\VirtualFarm")==!scenario.missing,"Cancelled uninstall changed VirtualFarm presence");
    need(exists(lm,L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Dobrý farmář")==!scenario.missing,"Cancelled uninstall changed private registration");
    files(root,"after");registrySnapshot(root,cu,lm,"after");
    std::ofstream(root/L"uninstall-fixture-result.json")<<"{\"cancelled\":true,\"fixture_presence_verified\":true}\n";
}
