#ifndef _TEARA_LIB_UTILS_TYPES_H_
#define _TEARA_LIB_UTILS_TYPES_H_

#include <inttypes.h>

typedef float       real32;
typedef double      real64;

typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef i32         bool32;
typedef i64         bool64;

typedef u8          byte;

enum Statuses {
    Success                         =  0,
    Failed                          = -1,
    RegisterClassFailed             = -2,
    WindowCreateFailed              = -3,
    GetDCCallFailed                 = -4,
    LibraryDllLoadFailed            = -5,
    FunctionLoadFailed              = -6,
    GLContextCreateFailed           = -7,
    FileLoadFailed                  = -8,
};

#endif