#include "OpenALAudioSystem.h"
#include "TLib/Utils/Debug.h"
#include "TLib/Utils/AudioLoader.h"

Statuses AudioSystemInit(AudioSystem* AudioSys, void* SystemBuffer, u64 SystemBufferSize)
{
    ALCdevice *Device;
    ALCcontext *Context;

    Device = alcOpenDevice(NULL);
    if (!Device) {
        // TODO error info!
        return Statuses::OALOpenDeviceFailed;
    }

    Context = alcCreateContext(Device, NULL);
    if (!Context) {
        // TODO error info!
        alcCloseDevice(Device);

        return Statuses::OALCreateContextFailed;
    }

    if (alcMakeContextCurrent(Context) == ALC_FALSE) {
        // TODO error info!
        alcDestroyContext(Context);
        alcCloseDevice(Device);

        return Statuses::OALMakeContextCurrentFailed;
    }

    AudioSys->Device                = Device;
    AudioSys->Context               = Context;

    AudioSys->SystemBuffer          = SystemBuffer;
    AudioSys->SystemBufferSize      = SystemBufferSize;

    AudioSys->FloatFormatPresent    = alIsExtensionPresent("AL_EXT_FLOAT32");
    AudioSys->EFXPresent            = alIsExtensionPresent("ALC_EXT_EFX");

    return Statuses::Success;
}


Statuses AudioSystemReinit(AudioSystem*)
{
    Assert(false); // not implemented check what was if we switch headphones on another headphones and etc.

    return Statuses::OALOpenDeviceFailed;
}


void AudioDestroy(AudioSystem *AudioSys)
{
    if (AudioSys->Context) {
        alcDestroyContext(AudioSys->Context);
    }

    if (AudioSys->Device) {
        alcCloseDevice(AudioSys->Device);
    }
}

SFX LoadSFX(AudioSystem *AudioSys, const char *FileName, AudioFormat Fmt)
{
    SFX Result          = {};
    AudioFile SoundFile = {};
    i32 Err             = 0;

    SoundFile.Fmt = Fmt;

    if (!LoadSound(FileName, AudioSys->SystemBuffer, AudioSys->SystemBufferSize, &SoundFile, AudioSys->FloatFormatPresent)) {
        Assert(false);
    }

    alGenBuffers(1, &Result.BufferHandle);
    alBufferData(Result.BufferHandle, SoundFile.SoundFormat, SoundFile.Buffer, SoundFile.Size, SoundFile.SampleRate);

    Err = alGetError();
    if(Err != AL_NO_ERROR) {
        // TODO error handling
        Assert(false);
    }

    return Result;
}

AudioSource AudioSourceInit()
{
    i32 Err;
    AudioSource Result = {};

    alGenSources(1, &Result.SourceHandle);

    Err = alGetError();
    if(Err != AL_NO_ERROR) {
        // TODO error handling
        Assert(false);
    }

    return Result;
}

// TODO case when we unable to play sound
void PlaySFX(AudioSource Source, SFX Sound)
{
    i32 Err;

    alSourcei(Source.SourceHandle, AL_BUFFER, (ALint)Sound.BufferHandle);

    Err = alGetError();
    if(Err != AL_NO_ERROR) {
        // TODO error handling
        Assert(false);
    }

    alSourcePlay(Source.SourceHandle);

    Err = alGetError();
    if(Err != AL_NO_ERROR) {
        // TODO error handling
        Assert(false);
    }
}