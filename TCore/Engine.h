#ifndef _TEARA_CORE_ENGINE_H_
#define _TEARA_CORE_ENGINE_H_

#include "TLib/Utils/Types.h"
#include "TLib/Utils/Debug.h"
#include "TLib/Math/Vector.h"

struct File {
    u64      Size;
    byte    *Data;
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
    Assert(Val <= 0xFFFFFFFF)
    i32 Result = (i32)Val;
    return Result;
}

File LoadFile(const char *FileName);

inline void FreeFileMemory(File *FileData);

#endif