#include "SquallSoundApi.h"
#include "../../squall/runtime/Squall.h"
#include <cstring>
#include <type_traits>
namespace farmar::recovered {
static_assert(sizeof(SoundSetup)==sizeof(squall_parameters_t));
static_assert(std::is_same_v<SoundOpen,squall_callback_open_t>);
static_assert(std::is_same_v<SoundSeek,squall_callback_seek_t>);
static_assert(std::is_same_v<SoundRead,squall_callback_read_t>);
static_assert(std::is_same_v<SoundClose,squall_callback_close_t>);
int SquallSoundApi::initialize(const SoundSetup& setup) {
    squall_parameters_t parameters;
    std::memcpy(&parameters,&setup,sizeof(parameters));
    return SQUALL_Init(&parameters);
}
void SquallSoundApi::free() { SQUALL_Free(); }
int SquallSoundApi::setFileCallbacks(SoundOpen a,SoundSeek b,SoundRead c,SoundClose d) { return SQUALL_SetFileCallbacks(a,b,c,d); }
int SquallSoundApi::setEaxPreset(int value) { return SQUALL_Listener_EAX_SetPreset(value); }
int SquallSoundApi::pause(int value) { return SQUALL_Pause(value); }
int SquallSoundApi::stop() { return SQUALL_Stop(); }
int SquallSoundApi::setGroupVolume(int group,int volume) { return SQUALL_ChannelGroup_SetVolume(group,volume); }
int SquallSoundApi::loadSample(const char* name,int memory,const void* defaults) {
    return SQUALL_Sample_LoadFile(const_cast<char*>(name),memory,
        const_cast<squall_sample_default_t*>(static_cast<const squall_sample_default_t*>(defaults)));
}
void SquallSoundApi::unloadSample(int sample) { SQUALL_Sample_Unload(sample); }
int SquallSoundApi::sampleFrequency(int sample) { return SQUALL_Sample_GetFileFrequency(sample); }
int SquallSoundApi::playSample(int sample,int loop,int group,int start,int priority,int volume,int frequency,int pan) {
    return SQUALL_Sample_PlayEx(sample,loop,group,start,priority,volume,frequency,pan);
}
int SquallSoundApi::stopChannel(int channel) { return SQUALL_Channel_Stop(channel); }
int SquallSoundApi::channelStatus(int channel) { return SQUALL_Channel_Status(channel); }
int SquallSoundApi::pauseChannel(int channel,int value) { return SQUALL_Channel_Pause(channel,value); }
int SquallSoundApi::startChannel(int channel) { return SQUALL_Channel_Start(channel); }
}
