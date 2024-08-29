#ifndef _TEARA_MATH_UTILS_H_
#define _TEARA_MATH_UTILS_H_

// TODO (ismail): my own SIMD math function or instead my own use SDL
#include <math.h>

#include "Utils/Types.h"

#define                     PI         (3.14159265358979323846f)
#define                 TWO_PI         (2.0f * PI)
#define          HALF_ROTATION         (180.0f)
#define            ONE_OVER_PI         (1.0f / PI)
#define        ONE_OVER_TWO_PI         (1.0f / TWO_PI)
#define ONE_OVER_HALF_ROTATION         (1.0f / HALF_ROTATION)
#define          DEGREE_IN_RAD         (HALF_ROTATION * ONE_OVER_PI)
#define          RAD_IN_DEGREE         (PI * ONE_OVER_HALF_ROTATION)
#define          DEGREE_TO_RAD(Deg)    (Deg * RAD_IN_DEGREE)
#define          RAD_TO_DEGREE(Rad)    (Rad * DEGREE_IN_RAD)
#define               INFINITY		   (1e30f)
#define         SMALLEST_FLOAT         (1.1754944e-038f)
#define                 SQUARE(Val)    ((Val) * (Val))

inline real32 Tangens(real32 Rad)
{
    real32 Result = tanf(Rad);
    return Result;
}

inline real32 Sqrt(real32 Scalar) 
{
    real32 Result = Scalar > SMALLEST_FLOAT ? sqrtf(Scalar) : INFINITY;
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

inline void SineCosine(real32 Rad, real32 *SineVal, real32 *CosineVal)
{
    *SineVal = sinf(Rad);
    *CosineVal = cosf(Rad);
}

inline real32 Fabs(real32 Scalar)
{
    real32 Result = fabsf(Scalar);
    return Result;
}

inline real32 Floor(real32 Scalar)
{
    real32 Result = floorf(Scalar);
    return Result;
}

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

#endif