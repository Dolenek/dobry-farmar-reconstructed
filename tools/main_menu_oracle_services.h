// Original navigation methods run in the diagnostic; only engine construction,
// original profile services, old CRT and manager boundaries are adapted.
#include "MainMenu.h"
namespace {
unsigned mainMenuSourceCompleted=0,mainMenuNativeCompleted=0,mainMenuNativeLogs=0;
bool mainMenuLogging=false;
void mainMenuSourceComplete() {++mainMenuSourceCompleted;}
void mainMenuNativeComplete() {++mainMenuNativeCompleted;}
bool __fastcall mainMenuSettingsBool(void* self,void* unused,const String* section,const String* key,bool* output) {
    if(section->view()=="game" && key->view()=="LogResources") {*output=mainMenuLogging;return true;}
    return getBool(self,unused,section,key,output);
}
void __cdecl mainMenuNativeDiscard(void* self,const String* origin) {nativeDiscard(static_cast<unsigned char*>(self)+4,origin);}
void __fastcall mainMenuNativeLog(void*,void*,const String* name) {
    require(name->view()=="MenuResLog.xml","MainMenu native log filename differs");++mainMenuNativeLogs;
}
void mainMenuCompareView(View& view,void* native) {
    const auto& state=view.state();compareFloat(state.transparency,field<float>(native,8),"MainMenu transparency");
    compareFloat(state.transparencySpeed,field<float>(native,12),"MainMenu speed");
    require(state.transition==field<unsigned>(native,16) && state.shown==bool(field<unsigned char>(native,0x24)) && state.parentShown==bool(field<unsigned char>(native,0x25))
        && state.enabled==bool(field<unsigned char>(native,0x26)) && state.parentEnabled==bool(field<unsigned char>(native,0x27))
        && state.absoluteEnabled==bool(field<unsigned char>(native,0x28)) && state.parentAbsoluteEnabled==bool(field<unsigned char>(native,0x29)),"MainMenu view state differs");
}
void mainMenuDeleteNative(void* view) { reinterpret_cast<void*(__thiscall*)(void*,unsigned)>(field<void**>(view,0)[0])(view,1);nativeDiscards.erase(view); }
void mainMenuNativeDiscardTree(void* node,std::map<void*,std::string>& output) {
    if(field<unsigned char>(node,45)) return;
    mainMenuNativeDiscardTree(field<void*>(node,0),output);
    output[field<void*>(node,12)]=std::string(field<String>(node,16).view());
    mainMenuNativeDiscardTree(field<void*>(node,8),output);
}
}

namespace {
MainMenuContext* newUserMainContext=nullptr;
unsigned newUserSourceCompletions=0,newUserNativeCompletions=0;
void newUserSourceComplete() {++newUserSourceCompletions;newUserMainContext->restore(true);}
void newUserNativeComplete() {++newUserNativeCompletions;function<void(__cdecl*)()>(0x8a680)();}
}

namespace {
Settings* startupSettings=nullptr;
bool __cdecl startupNativeHeadersExist() {return Profile::headersExist(*loadingFiles,*startupSettings);}
WString* __cdecl startupNativeStoredName(WString* output) {return new(output) WString(Profile::storedName(*loadingFiles,*startupSettings,[](const String& name){return resources->loadSettingsXML(name);}));}
cResource* __fastcall startupSoundResource(void* self,void* unused,const String* name) {
    String key(*name);key.lowercase();if(key.view()=="sounds/menu_click.wav" || key.view()=="sounds/menu_move.wav") return getSound(self,unused,name);
    return loadingSoundResource(self,unused,name);
}
}
