#pragma once
namespace farmar::recovered {
class cChannel;class cSound;class ResourceManager;class SoundManager;class String;
// Original shared globals 0065ffdc (channel) and 0065ffe0 (resource).
// Application owns this state and clears it before SoundManager destruction.
class MusicPlayback {
public:
    ~MusicPlayback();
    MusicPlayback()=default;
    MusicPlayback(const MusicPlayback&)=delete;
    MusicPlayback& operator=(const MusicPlayback&)=delete;
    void clear();
    void replace(ResourceManager&,SoundManager&,const String&); // 00480770
    cChannel* channel() const noexcept {return channel_;}
    cSound* sound() const noexcept {return sound_;}
private:
    cChannel* channel_=nullptr;cSound* sound_=nullptr;
};
}
