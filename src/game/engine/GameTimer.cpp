#include "GameTimer.h"
#include "Settings.h"
#include "OriginalArithmetic.h"
namespace farmar::recovered {
namespace { GameTimer* currentTimer=nullptr; }
TimerAccumulation& timerAccumulation() { static TimerAccumulation accumulation;return accumulation; }
GameTimer* GameTimer::instance() noexcept { return currentTimer; }
GameTimer::GameTimer(Settings& settings,TimerServices services):services_(services) {
    if(currentTimer) throw "Object already exist.";
    currentTimer=this;
    const String section("Time"),minimum("MinFps"),maximum("MaxFps");
    if(!settings.get(section,minimum,state_.minFps)) settings.set(section,minimum,state_.minFps);
    // Native 00536ae0 also writes MinFps when MaxFps is absent.
    if(!settings.get(section,maximum,state_.maxFps)) settings.set(section,minimum,state_.maxFps);
    state_.maxFrame=static_cast<float>(originalDivide(1,state_.minFps));
    state_.waitThreshold=static_cast<float>(originalDivide(1,state_.maxFps));
    services_.beginPeriod(1);state_.lastMilliseconds=services_.milliseconds(); // 00536690
}
GameTimer::~GameTimer() { services_.endPeriod(1);currentTimer=nullptr; }
float GameTimer::elapsed() const noexcept { return state_.average>0 ? state_.average:state_.frame; }
float GameTimer::unscaledElapsed() const noexcept { return static_cast<float>(originalDivide(state_.frame,state_.multiplier)); }
void GameTimer::setMinFps(float value) noexcept { state_.minFps=value;state_.maxFrame=static_cast<float>(originalDivide(1,value)); }
void GameTimer::process() {
    auto& accumulation=timerAccumulation();
    ++accumulation.samples;
    if(accumulation.samples%state_.averageSamples==0) {
        // The average is computed BEFORE sampling this frame. FPS uses the
        // unspilled quotient; elapsed stores a clamped, scaled float copy.
        const auto average=originalDivide(accumulation.seconds,originalUnsigned(state_.averageSamples));
        state_.average=static_cast<float>(average);state_.fps=static_cast<float>(originalDivide(1,average));
        accumulation={};
        if(state_.maxFrame<state_.average) state_.average=state_.maxFrame;
        state_.average=static_cast<float>(originalMultiply(state_.multiplier,state_.average));
    }
    auto milliseconds=services_.milliseconds();auto difference=milliseconds-state_.lastMilliseconds;
    // Original compares milliseconds with 1/MaxFps (seconds), sleeps once,
    // and samples again. Preserve that unit mismatch and unsigned wraparound.
    if(originalUnsigned(difference)<state_.waitThreshold) {
        services_.sleep(1);milliseconds=services_.milliseconds();difference=milliseconds-state_.lastMilliseconds;
    }
    state_.lastMilliseconds=milliseconds;
    const auto seconds=originalMultiply(originalUnsigned(difference),0.001f);
    state_.previousTotal=state_.total;state_.frame=static_cast<float>(seconds);
    state_.total=static_cast<float>(originalAdd(seconds,state_.total));
    accumulation.seconds=static_cast<float>(originalAdd(accumulation.seconds,seconds));
    if(state_.maxFrame<seconds) state_.frame=state_.maxFrame;
    state_.frame=static_cast<float>(originalMultiply(state_.multiplier,state_.frame));
}
}
