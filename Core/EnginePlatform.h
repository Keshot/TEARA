#ifndef _TEARA_ENGINE_PLATFORM_H_
#define _TEARA_ENGINE_PLATFORM_H_

#include "Types.h"
#include "Math/Vector.h"

struct File {
    u64      Size;
    byte    *Data;
};

#define TEARA_PLATFORM_ALLOCATE_MEMORY(Name) void* (Name)(u64 Size)
typedef TEARA_PLATFORM_ALLOCATE_MEMORY(*TEARA_PlatformAllocateMemory);

#define TEARA_PLATFORM_RELEASE_MEMORY(Name) void (Name)(void *Ptr)
typedef TEARA_PLATFORM_RELEASE_MEMORY(*TEARA_PlatformReleaseMemory);

#define TEARA_PLATFORM_READ_FILE(Name) File (Name)(const char *FileName)
typedef TEARA_PLATFORM_READ_FILE(*TEARA_PlatformReadFile);

#define TEARA_PLATFORM_FREE_FILE_DATA(Name) void (Name)(File *FileData)
typedef TEARA_PLATFORM_FREE_FILE_DATA(*TEARA_PlatformFreeFileData);

enum KeyState {
    Released    = 0,
    Pressed     = 1,
};

struct Key {
    i32         TransactionCount;
    KeyState    State;
    KeyState    PrevState;
};

struct Mouse {
    vec2    Moution;
    real32  Sensitive;
    real32  NormalizedWidth;
    real32  NormalizedHeight;
};

struct Input {
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
    Key     MButton;

    Key     ArrowUp;
    Key     ArrowDown;
    Key     ArrowRight;
    Key     ArrowLeft;
};

struct ScreenOptions {
    i32     Width;
    i32     Height;
    i32     CenterWidth;
    i32     CenterHeight;
    i32     ActualWidth;
    i32     ActualHeight;
    i32     ActualCenterWidth;
    i32     ActualCenterHeight;
    real32  AspectRatio;
};

enum MouseCursorState { Lock, Unlock };

struct Platform {
    ScreenOptions                   ScreenOpt;
    Input                           Input;
    bool32                          Running;
    bool32                          CursorSwitched;
    MouseCursorState                CursorState;

    TEARA_PlatformAllocateMemory    AllocMem;
    TEARA_PlatformReleaseMemory     ReleaseMem;
    TEARA_PlatformReadFile          ReadFile;
    TEARA_PlatformFreeFileData      FreeFileData;
};

#endif