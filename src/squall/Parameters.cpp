#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "Parameters.h"
#include <cstring>

namespace farmar::recovered {
void normalizeSquallParameters(const SquallParameters* input,
                              SquallParameters* output,SquallDeviceCount deviceCount) {
    if(IsBadReadPtr(input,sizeof(SquallParameters))) {
        std::memset(output,0,sizeof(SquallParameters));
    } else {
        // Native rep movsd copies forwards, including partially overlapping input.
        for(unsigned i=0;i<13;++i) {
            std::uint32_t word;
            std::memcpy(&word,reinterpret_cast<const unsigned char*>(input)+4*i,4);
            std::memcpy(reinterpret_cast<unsigned char*>(output)+4*i,&word,4);
        }
    }
    if(output->rate<=0) output->rate=44100;
    else if(output->rate>48000) output->rate=48000;
    if(output->bits!=8 && output->bits!=16) output->bits=16;
    if(output->channels<=0) output->channels=16;
    else if(output->channels>256) output->channels=256;
    if(output->bufferMilliseconds<=0) output->bufferMilliseconds=200;
    else if(output->bufferMilliseconds>5000) output->bufferMilliseconds=5000;
    if(output->algorithm<0 || output->algorithm>3) output->algorithm=0;
    if(output->listenerMode!=0 && output->listenerMode!=1) output->listenerMode=0;
    // The original fcomp/test ah,41 treats unordered inputs like nonpositive
    // inputs. All three fields are limited to ten, including DistanceFactor.
    auto factor=[](float& value) {
        if(!(value>0.0f)) value=1.0f;
        else if(value>10.0f) value=10.0f;
    };
    factor(output->distanceFactor);
    factor(output->rolloffFactor);
    factor(output->dopplerFactor);
    if(output->hardware2D!=0 && output->hardware2D!=1) output->hardware2D=1;
    if(output->hardware3D!=0 && output->hardware3D!=1) output->hardware3D=1;
    if(!IsWindow(reinterpret_cast<HWND>(output->window))) output->window=0;
    if(output->device<=0 || output->device>=deviceCount()) output->device=0;
}
}
