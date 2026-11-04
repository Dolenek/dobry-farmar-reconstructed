#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <cstddef>
#include <iostream>
using byte=unsigned char;
#include "SoundFile/track/stdafx.h"
#include "SoundFile/track/TrackDecoder.h"
int main(){std::cout<<"{\"size\":"<<sizeof(CDecompressTrack)<<",\"offsets\":{";
#define FIELD(name) std::cout<<"\"" #name "\":"<<offsetof(CDecompressTrack,name)<<','
FIELD(Chn);FIELD(ChnMix);FIELD(OrderInfo);FIELD(SamplesInFile);FIELD(Order);FIELD(m_nType);FIELD(m_nChannels);FIELD(m_nMixChannels);FIELD(m_nDefaultSpeed);FIELD(m_nDefaultTempo);FIELD(m_dwSongFlags);FIELD(m_nDefaultGlobalVolume);FIELD(MaxSamplesInPattern);FIELD(PatternSamplePos);FIELD(SamplesInPattern);FIELD(PatternSampleBuffer);FIELD(legacy_Order);FIELD(legacy_m_nChannels);FIELD(legacy_m_dwSongFlags);
#undef FIELD
std::cout<<"\"FileBuffer\":"<<offsetof(CDecompressTrack,FileBuffer)<<"}}\n";}
