#ifndef _TEARA_LIB_UTILS_AUDIO_LOADER_H_
#define _TEARA_LIB_UTILS_AUDIO_LOADER_H_

#include "TLib/Utils/Types.h"
#include "TLib/Core/Engine.h"

enum AudioFormat {
    WAV,
    FLAC
};

struct AudioFile {
    void*           Buffer;
    u64             Size;
    AudioFormat     Fmt;
    u32             SampleRate;
    i32             SoundFormat;
};

bool32 LoadSound(const char *FileName, void *Buffer, u64 BufferSize, AudioFile *FileOutput, bool32 FloatFormatPresent);

#endif
