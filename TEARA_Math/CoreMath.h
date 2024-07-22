#ifndef _TEARA_MATH_UTILS_H_
#define _TEARA_MATH_UTILS_H_

#include "Types.h"

// TODO (ismail): my own SIMD math function or instead my own use SDL
#include <math.h>

#define                     PI         3.14159265358979323846f
#define                 TWO_PI         2.0f * PI
#define          HALF_ROTATION         180.0f
#define            ONE_OVER_PI         (1.0f / PI)
#define ONE_OVER_HALF_ROTATION         (1.0f / HALF_ROTATION)
#define          DEGREE_IN_RAD         (HALF_ROTATION * ONE_OVER_PI)
#define          RAD_IN_DEGREE         (PI * ONE_OVER_HALF_ROTATION)
#define          DEGREE_TO_RAD(Deg)    (Deg * RAD_IN_DEGREE)
#define          RAD_TO_DEGREE(Rad)    (Rad * DEGREE_IN_RAD)

inline real32 Tangens(real32 Rad)
{
    real32 Result = tanf(Rad);
    return Result;
}

inline real32 Sqrt(real32 Scalar) 
{
    real32 Result = sqrtf(Scalar);
    return Result;
}

inline real32 Sine(real32 Rad)
{
    real32 Result = sinf(Rad);
    return Result;
}

inline real32 Cosine(real32 Rad)
{
    real32 Result = cosf(Rad);
    return Result;
}

#endif