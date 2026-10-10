// Explicit disabled-audio/service fixtures, used only by the native UI oracle.
#include "LoadingFromLog.h"
#include "MusicPlayback.h"
#include "SoundManager.h"
#include "Channel.h"
#include "Sound.h"
class LoadingDisabledAudio final : public SoundApi {
    static int unexpected() {throw std::runtime_error("Disabled Loading audio unexpectedly called a playback API");}
public:
    int initialize(const SoundSetup&) override {return -1;}
    void free() override {}
    int setFileCallbacks(SoundOpen,SoundSeek,SoundRead,SoundClose) override {return unexpected();}
    int setEaxPreset(int) override {return unexpected();}
    int pause(int) override {return unexpected();}
    int stop() override {return unexpected();}
    int setGroupVolume(int,int) override {return unexpected();}
    int loadSample(const char*,int,const void*) override {return unexpected();}
    void unloadSample(int) override {unexpected();}
    int sampleFrequency(int) override {return unexpected();}
    int playSample(int,int,int,int,int,int,int,int) override {return unexpected();}
    int stopChannel(int) override {return unexpected();}
    int channelStatus(int) override {return unexpected();}
    int pauseChannel(int,int) override {return unexpected();}
    int startChannel(int) override {return unexpected();}
};
namespace {
SoundManager* loadingSoundManager=nullptr;FileIndex* loadingFiles=nullptr;
unsigned loadingSourceCallbacks=0,loadingNativeCallbacks=0,loadingNativeClears=0,loadingNativeXMLDeletes=0;
std::map<std::string,std::unique_ptr<XmlDocument>> loadingDocuments;
void loadingSourceComplete() {++loadingSourceCallbacks;}
void loadingNativeComplete() {++loadingNativeCallbacks;}
cResource* __fastcall loadingSoundResource(void*,void*,const String* name) {return resources->getSound(*name);}
cChannel* __fastcall loadingMusicChannel(void*,void*,cSound* resource,bool loop,bool start) {
    require(loop && start,"Loading music flags differ");return loadingSoundManager->playMusic(resource,loop,start);
}
XmlAdapter* __fastcall loadingXML(void*,void*,const String* name) {
    auto& document=loadingDocuments[std::string(name->view())];if(!document) document=std::make_unique<XmlDocument>(*loadingFiles,*name);
    return wrap(document.get());
}
void __fastcall loadingDeleteXML(void*,void*) {++loadingNativeXMLDeletes;}
bool __fastcall loadingExists(void*,void*,const String* name) {return loadingFiles->isExist(*name);}
void __fastcall loadingClear(void*,void*) {++loadingNativeClears;resources->clearUseless();}
String* __fastcall loadingOriginLine(String* self,void*,unsigned number) {return new(self) String(std::to_string(number));}
String* __cdecl loadingConcat(String* output,const String* left,const String* right) {return new(output) String(std::string(left->view())+std::string(right->view()));}
}
