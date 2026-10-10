// Layout probe for the pinned public source, not a reconstructed runtime.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include "Squall.h"
#include "SoundEngine/Context.h"
#include <cstdio>
#include <cstddef>
int main() {
    std::printf("{\"sample\":%u,\"context\":%u,\"parameters\":%u,\"defaults\":%u,\"channel_end_worker\":%u,\"channel_user_data\":%u,\"channel_worker\":%u,\"channel_parameters\":%u}\n",
        unsigned(sizeof(SSample)),unsigned(sizeof(context_t)),unsigned(sizeof(squall_parameters_t)),unsigned(sizeof(squall_sample_default_t)),
        unsigned(offsetof(SSample,ChannelEndWorker)),unsigned(offsetof(SSample,UserData)),unsigned(offsetof(SSample,ChannelWorker)),unsigned(offsetof(SSample,Param)));
}
