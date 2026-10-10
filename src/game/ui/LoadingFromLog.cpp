#include "LoadingFromLog.h"
#include "FileIndex.h"
#include "ResourceManager.h"
#include "XmlResource.h"
#include "MusicPlayback.h"
#include "StringConversion.h"
#include "OriginalArithmetic.h"
#include <stdexcept>
namespace farmar::recovered {
namespace {bool maximumInitialized=false;float sharedMaximum;}
LoadingFromLog::LoadingFromLog(ResourceManager& resources,Settings& settings,StringManager& strings,
    const String& filename,Completion complete,unsigned batch,ViewRegistry& views,ControlRegistry& controls)
    :MenuView(nullptr,resources,settings,strings,views,controls),resources_(resources),filename_(filename),complete_(complete),batch_(batch),
     progress_(std::make_unique<ProgressBar>(nullptr,resources,views,controls)) {
    const auto document=resources.loadSettingsXML(String("Ui/Load.xml"));loadXML(*document);
}
LoadingFromLog* LoadingFromLog::create(FileIndex& files,ResourceManager& resources,Settings& settings,StringManager& strings,
    SoundManager& sound,MusicPlayback& music,const String& filename,Completion complete,unsigned batch,bool menuMusic,
    ViewRegistry& views,ControlRegistry& controls) {
    if(!complete) throw std::invalid_argument("LoadingFromLog requires its recovered completion callback");
    std::unique_ptr<LoadingFromLog> screen(new LoadingFromLog(resources,settings,strings,filename,complete,batch,views,controls));
    music.replace(resources,sound,menuMusic ? screen->menuMusic_:screen->gameMusic_);
    if(!files.isExist(filename)) {complete();return nullptr;}
    screen->log_.reset(resources.getXML(filename));screen->total_=unsigned(screen->log_->document().children().size());
    return screen.release();
}
LoadingFromLog::~LoadingFromLog() {
    // Native destruction deletes independent ProgressBar before the iXML.
    progress_.reset();log_.reset();
}
void LoadingFromLog::loadXML(const XmlNode& node) {
    MenuView::loadXML(node);XmlNode* section=nullptr;
    if(progress_ && node.section(String("Progress"),section)) progress_->loadXML(*section);
    if(node.section(String("MainMenuMusic"),section)) menuMusic_=toNarrow(section->attribute(WString(L"Sound")));
    if(node.section(String("GameMusic"),section)) gameMusic_=toNarrow(section->attribute(WString(L"Sound")));
}
void LoadingFromLog::enabledStep() {
    if(firstStep_) {firstStep_=false;resources_.clearUseless();}
    if(!maximumInitialized) {sharedMaximum=progress_->maximum;maximumInitialized=true;}
    const unsigned sum=next_+batch_,end=sum<=total_ ? sum:total_; // retain 32-bit wrapping
    resources_.loadFromLog(log_->document(),next_,end);
    progress_->current=static_cast<float>(originalDivide(originalMultiply(originalUnsigned(next_),sharedMaximum),originalUnsigned(total_)));
    progress_->updateLid();next_=end;
    if(next_>=total_) {complete_();hide();registry_.requestDiscard(this,String(".\\Game\\Ui\\LoadingFromLog.cpp122"));}
}
}
