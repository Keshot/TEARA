#ifndef _TEARA_CORE_ENGINE_H_
#define _TEARA_CORE_ENGINE_H_

#include "TLib/Utils/Types.h"
#include "TLib/Utils/Debug.h"
#include "TLib/Math/Vector.h"

// TODO (ismail): may be move Engine.h from TCore to TLib?
#define TEARA_PLATFORM_ALLOCATE_MEMORY(Name) void* (Name)(u64 Size)
typedef TEARA_PLATFORM_ALLOCATE_MEMORY(*TEARA_PlatformAllocateMemory);

struct File {
    u64      Size;
    byte    *Data;
};

#define TEARA_PLATFORM_READ_FILE(Name) File (Name)(const char *FileName)
typedef TEARA_PLATFORM_READ_FILE(*TEARA_PlatformReadFile);

#define TEARA_PLATFORM_FREE_FILE_DATA(Name) void (Name)(File *FileData)
typedef TEARA_PLATFORM_FREE_FILE_DATA(*TEARA_PlatformFreeFileData);

struct EnginePlatform {
    TEARA_PlatformAllocateMemory    AllocMem;
    TEARA_PlatformReadFile          ReadFile;
    TEARA_PlatformFreeFileData      FreeFileData;
};

enum KeyState {
    Released    = 0,
    Pressed     = 1,
};

struct Key {
    i32         TransactionCount;
    KeyState    State;
};

struct Mouse {
    Vec2    Moution;
    real32  Sensitive;
    real32  NormalizedWidth;
    real32  NormalizedHeight;
};

struct GameInput {
    bool32  IsAnalog;
    Mouse   MouseInput;

    Key     QButton;
    Key     EButton;
    Key     WButton;
    Key     SButton;
    Key     DButton;
    Key     AButton;
    Key     IButton;
    Key     KButton;
    Key     LButton;
    Key     JButton;

    Key     ArrowUp;
    Key     ArrowDown;
    Key     ArrowRight;
    Key     ArrowLeft;
};

inline i32 SafeTruncateI64(i64 Val)
{
    Assert(Val <= 0xFFFFFFFF);
    i32 Result = (i32)Val;
    return Result;
}

#endif