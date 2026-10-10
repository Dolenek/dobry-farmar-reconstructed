#pragma once
#include "MenuView.h"
#include "ProgressBar.h"
#include <memory>
namespace farmar::recovered {
class FileIndex;class SoundManager;class MusicPlayback;class XmlResource;
// Original 196-byte cMenuView-derived screen; modern object ABI differs.
class LoadingFromLog final : public MenuView {
public:
    using Completion=void(*)();
    // Original missing-log branch invokes completion and deletes the screen
    // from its constructor. The factory performs that branch before returning
    // and returns nullptr rather than an already freed object pointer.
    static LoadingFromLog* create(FileIndex&,ResourceManager&,Settings&,StringManager&,
        SoundManager&,MusicPlayback&,const String& filename,Completion,unsigned batch,bool menuMusic,
        ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 00480770
    ~LoadingFromLog() override; // 0047fff0
    void loadXML(const XmlNode&) override; // 00480110
    void enabledStep() override; // 00481130; deliberately no base enabledStep
    ProgressBar& progress() noexcept {return *progress_;}
    unsigned nextIndex() const noexcept {return next_;}
    unsigned total() const noexcept {return total_;}
    unsigned batch() const noexcept {return batch_;}
    bool firstStep() const noexcept {return firstStep_;}
    const String& menuMusic() const noexcept {return menuMusic_;}
    const String& gameMusic() const noexcept {return gameMusic_;}
private:
    LoadingFromLog(ResourceManager&,Settings&,StringManager&,const String&,Completion,unsigned,
        ViewRegistry&,ControlRegistry&);
    ResourceManager& resources_;String filename_,unused_,menuMusic_,gameMusic_;
    Completion complete_;unsigned next_=0,total_=0,batch_;bool firstStep_=true;
    std::unique_ptr<ProgressBar> progress_;std::unique_ptr<XmlResource> log_;
};
}
