#ifndef _TEARA_CORE_ENGINE_H_
#define _TEARA_CORE_ENGINE_H_

#include "TEARA_Lib/Utils/Types.h"
#include "TEARA_Lib/Utils/Debug.h"

struct File {
    u64      Size;
    byte    *Data;
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