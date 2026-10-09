#include "ControlAnimation.h"
#include "OriginalArithmetic.h"
#include <cstdint>
namespace farmar::recovered {
float normalizeControlAnimationTime(float time,float length,bool loop) noexcept {
    double value=time;
    if(value>length) {
        if(loop) value=originalSubtract(value,originalMultiply(static_cast<std::int32_t>(originalDivide(value,length)),length));
        else value=length;
    }
    if(value<0) {
        if(loop) value=originalSubtract(value,originalMultiply(static_cast<std::int32_t>(originalDivide(value,length))-1,length));
        else value=0;
    }
    return static_cast<float>(value);
}
unsigned controlAnimationFrame(float time,float length,unsigned frames) noexcept {
    if(!(time<length)) return frames-1;
    return static_cast<unsigned>(static_cast<std::int64_t>(originalMultiply(originalDivide(time,length),originalUnsigned(frames))));
}
ControlAnimationFrame resolveControlAnimation(ControlAnimationState& animation) noexcept {
    ControlAnimationFrame result; result.ended=animation.time>animation.length && !animation.loop;
    animation.time=normalizeControlAnimationTime(animation.time,animation.length,animation.loop);
    result.first=controlAnimationFrame(animation.time,animation.length,animation.frames);
    if(!animation.doubleRender) return result;
    result.count=2;
    const double frameLength=originalDivide(animation.length,originalUnsigned(animation.frames));
    const float fraction=static_cast<float>(originalDivide(originalSubtract(animation.time,originalMultiply(originalUnsigned(result.first),frameLength)),frameLength));
    const double a=originalMultiply(originalSubtract(1.0,fraction),2); result.firstOpacity=a>1 ? 1:static_cast<float>(a);
    const double b=originalMultiply(fraction,2); result.secondOpacity=b>1 ? 1:static_cast<float>(b);
    result.second=result.first+1;
    if(result.second>=animation.frames) result.second=animation.loop ? result.second%animation.frames:animation.frames-1;
    return result;
}
void updateAnimatedControlSprites(SpriteDefinition& first,SpriteDefinition& second,
    const ControlSpriteProperties& properties,const ControlAnimationFrame& frame) noexcept {
    updateControlSprite(first,properties,frame.first,frame.firstOpacity);
    if(frame.count==2) updateControlSprite(second,properties,frame.second,frame.secondOpacity);
}
}
