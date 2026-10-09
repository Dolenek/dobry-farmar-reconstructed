#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
namespace farmar::recovered {
class Settings;
struct TimerServices {
    DWORD (WINAPI* milliseconds)()=&timeGetTime;
    void (WINAPI* sleep)(DWORD)=&Sleep;
    MMRESULT (WINAPI* beginPeriod)(UINT)=&timeBeginPeriod;
    MMRESULT (WINAPI* endPeriod)(UINT)=&timeEndPeriod;
};
struct TimerAccumulation { float seconds=0;unsigned samples=0; };
TimerAccumulation& timerAccumulation(); // original shared globals 00660608/0c
struct TimerState {
    float total=0,previousTotal=0,frame=0.004f;
    DWORD lastMilliseconds=0;
    float minFps=15,maxFps=200,maxFrame=1.f/15,waitThreshold=1.f/200;
    float multiplier=1,fps=0,average=0;
    unsigned averageSamples=10;
};
// Recovered cTimer behavior and WinMM lifetime; modern state/service layout
// differs from the original 64-byte object and its internal function slots.
class GameTimer {
public:
    explicit GameTimer(Settings& settings,TimerServices services={}); // 00536ae0
    ~GameTimer(); // 00536430, calls 005366e0
    GameTimer(const GameTimer&)=delete;
    GameTimer& operator=(const GameTimer&)=delete;
    void process(); // 00536720 -> 00536550
    float elapsed() const noexcept; // 00536480
    float unscaledElapsed() const noexcept; // 005364a0
    float averageElapsed() const noexcept { return state_.average; } // 005364b0
    float fps() const noexcept { return state_.fps; } // 005364c0
    float multiplier() const noexcept { return state_.multiplier; } // 005364d0
    void setMultiplier(float value) noexcept { state_.multiplier=value; } // 005364e0
    float minFps() const noexcept { return state_.minFps; } // 005364f0
    void setMinFps(float value) noexcept; // 00536500
    float total() const noexcept { return state_.total; } // 00536520
    const TimerState& state() const noexcept { return state_; }
    static GameTimer* instance() noexcept;
private:
    TimerState state_;
    TimerServices services_;
};
}
