// Native profile getters are explicit diagnostic service boundaries. Production
// Awards accesses the recovered real typed profile properties.
namespace {
unsigned awardsSourceReturns=0,awardsNativeReturns=0;
unsigned awardsExitRegistrations=0;
int __cdecl awardsRegisterExit(void(*callback)()) {
    require(reinterpret_cast<unsigned char*>(callback)==image+0x1eda20,"Awards indexed key destructor registration differs");
    ++awardsExitRegistrations;return 0; // Diagnostic CRT service; PE entry point and CRT lifecycle are disabled.
}
String awardsKey;
unsigned *panelSourceCounter=nullptr,*panelNativeCounter=nullptr;
void panelAcceptedSource(ButtonPrototype*) {++*panelSourceCounter;}
void __cdecl panelAcceptedNative(void*) {++*panelNativeCounter;}
cResource* __fastcall panelResourceFactory(void*,void*,const String* name) {
    if(name->view()=="Sounds/Award.wav") return resources->getSound(*name);
    return loadSound(*name); // Existing Button diagnostic resource boundary.
}
String* __cdecl awardsProfileKey(const char* prefix,unsigned index) {awardsKey=String(std::string(prefix)+std::to_string(index));return &awardsKey;}
bool* __fastcall awardsProfileBool(void*,void*,const String* name) {return &profileProperties().booleans[std::string(name->view())];}
void awardsReturned() {
    auto* original=field<void*>(image,0x25ffcc);auto* view=static_cast<unsigned char*>(original)+4;
    require(field<unsigned>(view,0x10)==3,"Awards native callback preceded discard transition");++awardsNativeReturns;
}
}
