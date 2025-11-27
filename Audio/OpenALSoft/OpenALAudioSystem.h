#ifndef _TEARA_AUDIO_OPENAL_SOFT_H_
#define _TEARA_AUDIO_OPENAL_SOFT_H_

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include "Core/Types.h"
#include "Utils/AudioLoader.h"

#ifndef AUDIO_COMPONENTS_MAX_SFX_BUFFER
    #define AUDIO_COMPONENTS_MAX_SFX_BUFFER 20
#endif

struct AudioSystem {
    ALCdevice   *Device;
    ALCcontext  *Context;

    void        *SystemBuffer;
    u64         SystemBufferSize;

    bool32      FloatFormatPresent  : 1;
    bool32      EFXPresent          : 1;
};

struct SFX {
    u32 BufferHandle;
};

struct AudioSource {
    u32 SourceHandle;
};

// init OpenAL open device and create context and fill AudioSystem struct
Statuses AudioSystemInit(AudioSystem* AudioSys, void* SystemBuffer, u64 SystemBufferSize);
Statuses AudioSystemReinit(AudioSystem* AudioSys);
// free Device and Context
void AudioDestroy(AudioSystem *AudioSys);

// load audio file and generate al buffer for them
// @AudioSys audio system instance in which context we would like to load
// @FileName it is sfx file name
SFX LoadSFX(AudioSystem *AudioSys, const char *FileName, AudioFormat Fmt);

AudioSource AudioSourceInit();

void PlaySFX(AudioSource Source, SFX Sound);

#endif