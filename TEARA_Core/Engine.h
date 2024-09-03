#ifndef _TEARA_CORE_ENGINE_H_
#define _TEARA_CORE_ENGINE_H_

#include "TEARA_Lib/Utils/Types.h"
#include "TEARA_Lib/Utils/Debug.h"

struct File {
    u64      Size;
    byte    *Data;
};

enum KeyState {
    Pressed     = 0,
    Released    = 1
};

struct Key {
    i32         TransactionCount;
    KeyState    State;
};

struct GameInput {
    bool32  IsAnalog;
    Vec2    MouseMoution;

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
    Assert(Val <= 0xFFFFFFFF)
    i32 Result = (i32)Val;
    return Result;
}

File LoadFile(const char *FileName);

inline void FreeFileMemory(File *FileData);

#endif