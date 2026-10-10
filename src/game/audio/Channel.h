#pragma once
namespace farmar::recovered {
class cSound;
class cChannel {
public:
    cChannel(int channel,cSound* sound); // 004fb8b0
    virtual ~cChannel(); // 004fb700
    cChannel(const cChannel&)=delete;
    cChannel& operator=(const cChannel&)=delete;
    void pause(); // 004fbda0
    void start(); // 004fc1d0
    int channelID() const noexcept { return channel_; }
    unsigned group() const noexcept { return group_; }
    void setGroup(unsigned value) noexcept { group_=value; }
private:
    int channel_;cSound* sound_;unsigned group_=0;
};
static_assert(sizeof(cChannel)==16,"Original cChannel layout");
}
