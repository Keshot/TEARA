#ifndef _TEARA_LIB_MATH_UTILS_H_
#define _TEARA_LIB_MATH_UTILS_H_

// TODO (ismail): my own SIMD math function or instead my own use SDL
#include <corecrt_math.h>
#include <stdlib.h>

#include "TLib/Utils/Types.h"
#include "TLib/Utils/Debug.h"

#define                     PI         (3.14159265358979323846f)
#define                 TWO_PI         (2.0f * PI)
#define          HALF_ROTATION         (180.0f)
#define            ONE_OVER_PI         (1.0f / PI)
#define        ONE_OVER_TWO_PI         (1.0f / TWO_PI)
#define ONE_OVER_HALF_ROTATION         (1.0f / HALF_ROTATION)
#define          DEGREE_IN_RAD         (HALF_ROTATION * ONE_OVER_PI)
#define          RAD_IN_DEGREE         (PI * ONE_OVER_HALF_ROTATION)
#define          DEGREE_TO_RAD(Deg)    ((Deg) * RAD_IN_DEGREE)
#define          RAD_TO_DEGREE(Rad)    (Rad * DEGREE_IN_RAD)
#define               INFINITY		   (1e30f)
#define         SMALLEST_FLOAT         (1.1754944e-038f)
#define                 SQUARE(Val)    ((Val) * (Val))

#define Tan(Rad)    tanf((Rad))
#define Atan2(x, y) atan2f((x), (y))
#define Sqrt(x)     sqrtf((x))
#define Sin(Rad)    sinf((Rad))
#define Cos(Rad)    cosf((Rad))
#define Fabs(x)     fabsf((x))
#define Atof(Str)   atof((Str))
#define Atoi(Str)   atoi((Str))
#define Floor(x)    floorf((x))

enum Coordinates {
    _x_ = 0,
    _y_ = 1,
    _z_ = 2,
    _w_ = 3,
};

inline real32 Clampf(real32 Scalar, real32 Min, real32 Max)
{
    real32 Result = Scalar > Min ? (Scalar <= Max ? Scalar : Max) : Min;
    return Result;
}

inline real32 WrapPi(real32 Theta)
{
    if (Fabs(Theta) > PI) {
        real32 Revolutions = Floor((Theta + PI) * ONE_OVER_TWO_PI);
        Theta = Theta - (Revolutions * TWO_PI);
    }

    return Theta;
}

inline i32 SafeTruncateI64(i64 Val)
{
    Assert(Val <= 0xFFFFFFFF);
    i32 Result = (i32)Val;
    return Result;
}

#endif