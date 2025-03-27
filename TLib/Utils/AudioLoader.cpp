#include "AudioLoader.h"
#include <AL/alext.h>
#include "TLib/Utils/Debug.h"

#define DR_WAV_IMPLEMENTATION
#include "TLib/3rdparty/audio/dr_wav.h"

#define DR_FLAC_IMPLEMENTATION
#include "TLib/3rdparty/audio/dr_flac.h"

// for now it is okay but we need to create only one function that can read both flac and wav
// we can just simple read in buffer by ourself and provide raw memory to library
// so also it need to add ogg may be for long audio tracks
// also check TotalByres value because it can be a lot a bit

static bool32 LoadWAV(const char *FileName, void *Buffer, u64 BufferSize, AudioFile *FileOutput, bool32 FloatFormatPresent)
{
    drwav WavFileInfo   = {};
    void *SoundBuffer   = NULL;
    u64 TotalBytes      = 0;
    u64 BlockAlign      = 0;
    i32 SoundFormat     = 0;
    u64 Read            = 0;

    if (!drwav_init_file(&WavFileInfo, FileName, NULL)) {
        // TODO error handling
        Assert(false); // TODO can't read file

        return 0;
    }

    if (WavFileInfo.fmt.formatTag != DR_WAVE_FORMAT_PCM || WavFileInfo.channels < 2) {
        Assert(false); // TODO need to add support for format
    }

    if (FloatFormatPresent) {
        BlockAlign      = WavFileInfo.channels * 4; // cuz we read 4 bytes numbers then we need to align it correct
        TotalBytes      = BlockAlign * WavFileInfo.totalPCMFrameCount; // TODO check is it correct number?
        SoundFormat     = AL_FORMAT_STEREO_FLOAT32;

        if (TotalBytes > BufferSize) {
            Assert(false); // TODO error handling

            return 0;
        }

        Read = drwav_read_pcm_frames_f32(&WavFileInfo, WavFileInfo.totalPCMFrameCount, (real32*)SoundBuffer);

        if (Read != WavFileInfo.totalPCMFrameCount) {
            Assert(false); // we need to read whole file now
        }
    }
    else {
        Assert(false); // not present now mb later?
    }

    drwav_uninit(&WavFileInfo);

    FileOutput->Buffer      = SoundBuffer;
    FileOutput->Size        = TotalBytes;
    FileOutput->SoundFormat = SoundFormat;
    FileOutput->SampleRate  = WavFileInfo.sampleRate;

    return 1;
}

static bool32 LoadFLAC(const char *FileName, void *Buffer, u64 BufferSize, AudioFile *FileOutput, bool32 FloatFormatPresent)
{
    drflac *FlacFileInfo = drflac_open_file(FileName, NULL);
    void *SoundBuffer   = Buffer;
    u64 TotalBytes      = 0;
    u64 BlockAlign      = 0;
    i32 SoundFormat     = 0;
    u64 Read            = 0;

    if (!FlacFileInfo) {
        Assert(false); // TODO error handling

        return 0;
    }

    if (FlacFileInfo->channels < 2) {
        Assert(false); // TODO need to add support for format
    }

    if (FloatFormatPresent) {
        BlockAlign      = FlacFileInfo->channels * 4; // cuz we read 4 bytes numbers then we need to align it correct
        TotalBytes      = BlockAlign * FlacFileInfo->totalPCMFrameCount; // TODO check is it correct number?
        SoundFormat     = AL_FORMAT_STEREO_FLOAT32;

        if (TotalBytes > BufferSize) {
            Assert(false); // TODO error handling

            return 0;
        }

        Read = drflac_read_pcm_frames_f32(FlacFileInfo, FlacFileInfo->totalPCMFrameCount, (real32*)SoundBuffer);

        if (Read != FlacFileInfo->totalPCMFrameCount) {
            Assert(false); // TODO we need to read whole file now
        }
    }

    drflac_close(FlacFileInfo);

    FileOutput->Buffer      = SoundBuffer;
    FileOutput->Size        = TotalBytes;
    FileOutput->SoundFormat = SoundFormat;
    FileOutput->SampleRate  = FlacFileInfo->sampleRate;

    return 1;
}

bool32 LoadSound(const char *FileName, void *Buffer, u64 BufferSize, AudioFile *FileOutput, bool32 FloatFormatPresent)
{
    switch (FileOutput->Fmt)
    {
    
    case AudioFormat::WAV: {
        return LoadWAV(FileName, Buffer, BufferSize, FileOutput, FloatFormatPresent);
    }

    case AudioFormat::FLAC: {
        return LoadFLAC(FileName, Buffer, BufferSize, FileOutput, FloatFormatPresent);
    }

    default: {
        return 0;
    }
        
    }
}