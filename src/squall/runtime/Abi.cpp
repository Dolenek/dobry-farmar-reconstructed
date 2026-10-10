#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include "Squall.h"
#include "SoundEngine/Context.h"
#include <vorbis/vorbisfile.h>
#include <cstddef>
// Instruction-backed sizes and offsets, not assertions of complete equivalence.
static_assert(sizeof(squall_parameters_t)==52,"Init parameter ABI");
static_assert(sizeof(squall_sample_default_t)==28,"Sample default ABI");
static_assert(sizeof(SSample)==300,"Original channel allocation size");
static_assert(offsetof(SSample,ChannelWorker)==292 && offsetof(SSample,UserData)==296,"Original callback offsets");
static_assert(offsetof(context_t,_channels)==0xfc && offsetof(context_t,_channelsArray)==0x100,"Original channel lookup offsets");
static_assert(sizeof(OggVorbis_File)==0x2d0,"Original decoder allocation 100138f0");
