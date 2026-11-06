// Read-only compiler metadata; does not load or invoke either Squall DLL.
#include <cstddef>
#include <cstdio>
#include <type_traits>
#define _USRDLL
#define DLL_API
#include "Squall.h"
static_assert(sizeof(void*)==4 && sizeof(int)==4 && sizeof(long)==4 && sizeof(float)==4);
namespace {
bool firstType=true,firstField=true;
template<class T> void beginType(const char* name) {
    static_assert(std::is_standard_layout_v<T>);
    std::printf("%s\"%s\":{\"size\":%zu,\"alignment\":%zu,\"fields\":{",firstType?"":",",name,sizeof(T),alignof(T));
    firstType=false;firstField=true;
}
template<class F> void field(const char* name,std::size_t offset) {
    using E=std::remove_all_extents_t<F>;
    const char* kind=std::is_pointer_v<E>?"pointer":std::is_floating_point_v<E>?"float":std::is_integral_v<E>?(std::is_signed_v<E>?"signed":"unsigned"):"aggregate";
    std::printf("%s\"%s\":{\"offset\":%zu,\"bytes\":%zu,\"kind\":\"%s\",\"elements\":%zu}",firstField?"":",",name,offset,sizeof(F),kind,sizeof(F)/sizeof(E));
    firstField=false;
}
void endType(){std::printf("}}");}
}
#define FIELD(T,M) field<decltype(((T*)nullptr)->M)>(#M,offsetof(T,M))
int main(){
    std::printf("{\"pointer_bytes\":4,\"int_bytes\":4,\"long_bytes\":4,\"float_bytes\":4,\"types\":{");
    beginType<squall_parameters_t>("parameters");
    FIELD(squall_parameters_t,Window);
    FIELD(squall_parameters_t,Device);
    FIELD(squall_parameters_t,SampleRate);
    FIELD(squall_parameters_t,BitPerSample);
    FIELD(squall_parameters_t,Channels);
    FIELD(squall_parameters_t,UseHW2D);
    FIELD(squall_parameters_t,UseHW3D);
    FIELD(squall_parameters_t,UseAlg);
    FIELD(squall_parameters_t,BufferSize);
    FIELD(squall_parameters_t,ListenerMode);
    FIELD(squall_parameters_t,DistanceFactor);
    FIELD(squall_parameters_t,RolloffFactor);
    FIELD(squall_parameters_t,DopplerFactor);
    endType();
    beginType<squall_sample_default_t>("sample_default");
    FIELD(squall_sample_default_t,SampleGroupID);
    FIELD(squall_sample_default_t,Priority);
    FIELD(squall_sample_default_t,Frequency);
    FIELD(squall_sample_default_t,Volume);
    FIELD(squall_sample_default_t,Pan);
    FIELD(squall_sample_default_t,MinDist);
    FIELD(squall_sample_default_t,MaxDist);
    endType();
    beginType<squall_channels_t>("channels");
    FIELD(squall_channels_t,Play);
    FIELD(squall_channels_t,Pause);
    FIELD(squall_channels_t,Prepare);
    FIELD(squall_channels_t,Play3D);
    FIELD(squall_channels_t,Pause3D);
    FIELD(squall_channels_t,Prepare3D);
    endType();
    beginType<squall_device_caps_t>("device_caps");
    FIELD(squall_device_caps_t,Flags);
    FIELD(squall_device_caps_t,HardwareChannels);
    FIELD(squall_device_caps_t,Hardware3DChannels);
    endType();
    beginType<squall_zoomfx_channel_t>("zoomfx_channel");
    FIELD(squall_zoomfx_channel_t,Min);
    FIELD(squall_zoomfx_channel_t,Max);
    FIELD(squall_zoomfx_channel_t,Front);
    FIELD(squall_zoomfx_channel_t,Top);
    FIELD(squall_zoomfx_channel_t,MacroFX);
    endType();
    beginType<squall_eax_listener_t>("eax_listener");
    FIELD(squall_eax_listener_t,eax1);
    FIELD(squall_eax_listener_t,eax2);
    FIELD(squall_eax_listener_t,eax3);
    endType();
    using eax_listener_eax1=decltype(squall_eax_listener_t{}.eax1);
    beginType<eax_listener_eax1>("eax_listener_eax1");
    FIELD(eax_listener_eax1,Environment);
    FIELD(eax_listener_eax1,Volume);
    FIELD(eax_listener_eax1,DecayTime_sec);
    FIELD(eax_listener_eax1,Damping);
    endType();
    using eax_listener_eax2=decltype(squall_eax_listener_t{}.eax2);
    beginType<eax_listener_eax2>("eax_listener_eax2");
    FIELD(eax_listener_eax2,Room);
    FIELD(eax_listener_eax2,RoomHF);
    FIELD(eax_listener_eax2,RoomRolloffFactor);
    FIELD(eax_listener_eax2,DecayTime);
    FIELD(eax_listener_eax2,DecayHFRatio);
    FIELD(eax_listener_eax2,Reflections);
    FIELD(eax_listener_eax2,ReflectionsDelay);
    FIELD(eax_listener_eax2,Reverb);
    FIELD(eax_listener_eax2,ReverbDelay);
    FIELD(eax_listener_eax2,Environment);
    FIELD(eax_listener_eax2,EnvironmentSize);
    FIELD(eax_listener_eax2,EnvironmentDiffusion);
    FIELD(eax_listener_eax2,AirAbsorptionHF);
    FIELD(eax_listener_eax2,Flags);
    endType();
    using eax_listener_eax3=decltype(squall_eax_listener_t{}.eax3);
    beginType<eax_listener_eax3>("eax_listener_eax3");
    FIELD(eax_listener_eax3,Environment);
    FIELD(eax_listener_eax3,EnvironmentSize);
    FIELD(eax_listener_eax3,EnvironmentDiffusion);
    FIELD(eax_listener_eax3,Room);
    FIELD(eax_listener_eax3,RoomHF);
    FIELD(eax_listener_eax3,RoomLF);
    FIELD(eax_listener_eax3,DecayTime);
    FIELD(eax_listener_eax3,DecayHFRatio);
    FIELD(eax_listener_eax3,DecayLFRatio);
    FIELD(eax_listener_eax3,Reflections);
    FIELD(eax_listener_eax3,ReflectionsDelay);
    FIELD(eax_listener_eax3,ReflectionsPan);
    FIELD(eax_listener_eax3,Reverb);
    FIELD(eax_listener_eax3,ReverbDelay);
    FIELD(eax_listener_eax3,ReverbPan);
    FIELD(eax_listener_eax3,EchoTime);
    FIELD(eax_listener_eax3,EchoDepth);
    FIELD(eax_listener_eax3,ModulationTime);
    FIELD(eax_listener_eax3,ModulationDepth);
    FIELD(eax_listener_eax3,AirAbsorptionHF);
    FIELD(eax_listener_eax3,HFReference);
    FIELD(eax_listener_eax3,LFReference);
    FIELD(eax_listener_eax3,RoomRolloffFactor);
    FIELD(eax_listener_eax3,Flags);
    endType();
    beginType<squall_eax_channel_t>("eax_channel");
    FIELD(squall_eax_channel_t,eax1);
    FIELD(squall_eax_channel_t,eax2);
    FIELD(squall_eax_channel_t,eax3);
    endType();
    using eax_channel_eax1=decltype(squall_eax_channel_t{}.eax1);
    beginType<eax_channel_eax1>("eax_channel_eax1");
    FIELD(eax_channel_eax1,Mix);
    endType();
    using eax_channel_eax2=decltype(squall_eax_channel_t{}.eax2);
    beginType<eax_channel_eax2>("eax_channel_eax2");
    FIELD(eax_channel_eax2,Direct);
    FIELD(eax_channel_eax2,DirectHF);
    FIELD(eax_channel_eax2,Room);
    FIELD(eax_channel_eax2,RoomHF);
    FIELD(eax_channel_eax2,RoomRolloffFactor);
    FIELD(eax_channel_eax2,Obstruction);
    FIELD(eax_channel_eax2,ObstructionLFRatio);
    FIELD(eax_channel_eax2,Occlusion);
    FIELD(eax_channel_eax2,OcclusionLFRatio);
    FIELD(eax_channel_eax2,OcclusionRoomRatio);
    FIELD(eax_channel_eax2,OutsideVolumeHF);
    FIELD(eax_channel_eax2,AirAbsorptionFactor);
    FIELD(eax_channel_eax2,Flags);
    endType();
    using eax_channel_eax3=decltype(squall_eax_channel_t{}.eax3);
    beginType<eax_channel_eax3>("eax_channel_eax3");
    FIELD(eax_channel_eax3,Direct);
    FIELD(eax_channel_eax3,DirectHF);
    FIELD(eax_channel_eax3,Room);
    FIELD(eax_channel_eax3,RoomHF);
    FIELD(eax_channel_eax3,Obstruction);
    FIELD(eax_channel_eax3,ObstructionLFRatio);
    FIELD(eax_channel_eax3,Occlusion);
    FIELD(eax_channel_eax3,OcclusionLFRatio);
    FIELD(eax_channel_eax3,OcclusionRoomRatio);
    FIELD(eax_channel_eax3,OcclusionDirectRatio);
    FIELD(eax_channel_eax3,Exclusion);
    FIELD(eax_channel_eax3,ExclusionLFRatio);
    FIELD(eax_channel_eax3,OutsideVolumeHF);
    FIELD(eax_channel_eax3,DopplerFactor);
    FIELD(eax_channel_eax3,RolloffFactor);
    FIELD(eax_channel_eax3,RoomRolloffFactor);
    FIELD(eax_channel_eax3,AirAbsorptionFactor);
    FIELD(eax_channel_eax3,Flags);
    endType();
    std::printf("}}\n");
}
