#ifndef _TEARA_MATH_MATRIX_TRANSFORMATION_H_
#define _TEARA_MATH_MATRIX_TRANSFORMATION_H_

#include "CoreMath.h"
#include "Matrix.h"
#include "Vector.h"

inline Mat4x4 MakeOrtographProjection(real32 Left, real32 Right, real32 Bot, real32 Top, real32 Near, real32 Far)
{
    real32 HorizontalDistance = Right - Left;
    real32 VerticalDistance = Top - Bot;
    real32 DepthDistance = Far - Near;

    Mat4x4 Result = {
        2.0f / HorizontalDistance, 0.0f,                    0.0f,                   -((Left + Right) / HorizontalDistance),
        0.0f,                      2.0f / VerticalDistance, 0.0f,                   -((Bot + Top) / VerticalDistance),
        0.0f,                                         0.0f, 2.0f / DepthDistance,   -((Near + Far) / DepthDistance),
        0.0f,                                         0.0f, 0.0f,                   1.0f
    };

    return Result;
}

inline Mat4x4 MakePerspectiveProjection(real32 FovInDegree, real32 AspectRatio, real32 NearZ, real32 FarZ)
{
    real32 d = 1.0f / Tangens(DEGREE_TO_RAD(FovInDegree / 2));
    real32 x = d / AspectRatio;

    real32 ClipDistance = NearZ - FarZ;

    real32 a = (-FarZ - NearZ) / ClipDistance;
    real32 b = (2.0f * FarZ * NearZ) / ClipDistance;

    Mat4x4 Result = {
           x, 0.0f, 0.0f, 0.0f,
        0.0f,    d, 0.0f, 0.0f,
        0.0f, 0.0f,    a,    b,
        0.0f, 0.0f, 1.0f, 0.0f
    };

    return Result;
}

inline Mat4x4 MakeScale(Vec3 *Scale)
{
    Mat4x4 Result = {
        Scale->x,     0.0f,     0.0f, 0.0f,
            0.0f, Scale->y,     0.0f, 0.0f,
            0.0f,     0.0f, Scale->z, 0.0f,
            0.0f,     0.0f,     0.0f, 1.0f
    };

    return Result;
}

inline Mat4x4 MakeTranslation(Vec3 *Vec)
{
    Mat4x4 Result = {
        1.0f, 0.0f, 0.0f, Vec->x,
        0.0f, 1.0f, 0.0f, Vec->y,
        0.0f, 0.0f, 1.0f, Vec->z,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    return Result;
}

inline Mat4x4 MakeRotationAroundX(real32 Rad)
{
    Mat4x4 Result = {
        1.0f,         0.0f,        0.0f,    0.0f,
        0.0f,  Cosine(Rad),  -Sine(Rad),    0.0f,
        0.0f,    Sine(Rad), Cosine(Rad),    0.0f,
        0.0f,         0.0f,        0.0f,    1.0f
    };

    return Result;
}

inline Mat4x4 MakeRotationAroundY(real32 Rad)
{
    Mat4x4 Result = {
        Cosine(Rad), 0.0f,  -Sine(Rad),    0.0f,
               0.0f, 1.0f,        0.0f,    0.0f,
          Sine(Rad), 0.0f, Cosine(Rad),    0.0f,
               0.0f, 0.0f,        0.0f,    1.0f
    };

    return Result;
}

inline Mat3x3 MakeRotationAroundY3x3(real32 Rad)
{
    Mat3x3 Result = {
        Cosine(Rad), 0.0f,  -Sine(Rad),
               0.0f, 1.0f,        0.0f,
          Sine(Rad), 0.0f, Cosine(Rad)
    };

    return Result;
}

inline Mat4x4 MakeRotationAroundZ(real32 Rad)
{
    Mat4x4 Result = {
        Cosine(Rad),   -Sine(Rad),    0.0f,    0.0f,
          Sine(Rad),  Cosine(Rad),    0.0f,    0.0f,
               0.0f,         0.0f,    1.0f,    0.0f,
               0.0f,         0.0f,    0.0f,    1.0f
    };

    return Result;
}

#endif