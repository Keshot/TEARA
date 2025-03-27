#ifndef _TEARA_LIB_MATH_MATRIX_TRANSFORMATION_H_
#define _TEARA_LIB_MATH_MATRIX_TRANSFORMATION_H_

#include "TLib/Math/Math.h"
#include "TLib/Math/Matrix.h"
#include "TLib/Math/Vector.h"

struct Rotation {
    real32 Heading;
    real32 Pitch;
    real32 Bank;
};

// TODO (ismail): refactor this change separate cos sin on SineCosine
// TODO (ismail): add canonical Euler function
// TODO (ismail): rotation about an arbitrary axis

inline Mat4x4 MakeRotation4x4(Rotation *Rotation)
{
    real32 cosh, sinh;
    real32 cosp, sinp;
    real32 cosb, sinb;

    real32 Heading = WrapPi(Rotation->Heading);
    real32 Pitch;
    real32 Bank;

    if (Rotation->Pitch <= -90.0f) {
        Pitch = -90.0f;
        Bank = 0.0f;
    }
    else if (Rotation->Pitch >= 90.0f) {
        Pitch = 90.0f;
        Bank = 0.0f;
    }
    else {
        Pitch = Rotation->Pitch;
        Bank = Rotation->Bank;
    }

    SineCosine(Heading, &sinh, &cosh);
    SineCosine(Pitch, &sinp, &cosp);
    SineCosine(Bank, &sinb, &cosb);

    Mat4x4 Result = {
        cosh * cosb + sinh * sinp * sinb, -cosh * sinb + sinh * sinp * cosb, sinh * cosp, 0.0f,
                             cosp * sinb,                       cosp * cosb,       -sinp, 0.0f,
       -sinh * cosb + cosh * sinp * sinb,  sinh * sinb + cosh * sinp * cosb, cosh * cosp, 0.0f,
                                    0.0f,                              0.0f,        0.0f, 1.0f
    };

    return Result;
}

inline Mat3x3 MakeRotation3x3(Rotation *Rotation)
{
    real32 cosh, sinh;
    real32 cosp, sinp;
    real32 cosb, sinb;

    real32 Heading = WrapPi(Rotation->Heading);
    real32 Pitch;
    real32 Bank;

    if (Rotation->Pitch <= -90.0f) {
        Pitch = -90.0f;
        Bank = 0.0f;
    }
    else if (Rotation->Pitch >= 90.0f) {
        Pitch = 90.0f;
        Bank = 0;
    }
    else {
        Pitch = Rotation->Pitch;
        Bank = Rotation->Bank;
    }

    SineCosine(Heading, &sinh, &cosh);
    SineCosine(Pitch, &sinp, &cosp);
    SineCosine(Bank, &sinb, &cosb);

    Mat3x3 Result = {
        cosh * cosb + sinh * sinp * sinb, -cosh * sinb + sinh * sinp * cosb, sinh * cosp,
                             cosp * sinb,                       cosp * cosb,       -sinp,
       -sinh * cosb + cosh * sinp * sinb,  sinh * sinb + cosh * sinp * cosb, cosh * cosp
    };

    return Result;
}

inline Mat4x4 MakeRotation4x4Inverse(Rotation *Rotation)
{
    real32 cosh, sinh;
    real32 cosp, sinp;
    real32 cosb, sinb;

    real32 Heading = WrapPi(Rotation->Heading);
    real32 Pitch;
    real32 Bank;

    if (Rotation->Pitch <= -90.0f) {
        Pitch = -90.0f;
        Bank = 0.0f;
    }
    else if (Rotation->Pitch >= 90.0f) {
        Pitch = 90.0f;
        Bank = 0;
    }
    else {
        Pitch = Rotation->Pitch;
        Bank = Rotation->Bank;
    }

    SineCosine(Heading, &sinh, &cosh);
    SineCosine(Pitch, &sinp, &cosp);
    SineCosine(Bank, &sinb, &cosb);

    Mat4x4 Result = {
         cosh * cosb + sinh * sinp * sinb, cosp * sinb, -sinh * cosb + cosh * sinp * sinb, 0.0f,
        -cosh * sinb + sinh * sinp * cosb, cosp * cosb,  sinh * sinb + cosh * sinp * cosb, 0.0f,
                              sinh * cosp,       -sinp,                       cosh * cosp, 0.0f,
                                     0.0f,        0.0f,                              0.0f, 1.0f
    };

    return Result;
}

inline void RotationToVectors(Rotation *Rotation, Vec3 *Target, Vec3 *Right, Vec3 *Up)
{
    real32 cosh, sinh;
    real32 cosp, sinp;
    real32 cosb, sinb;

    real32 Heading = WrapPi(Rotation->Heading);
    real32 Pitch;
    real32 Bank;

    if (Rotation->Pitch <= -90.0f) {
        Pitch = -90.0f;
        Bank = 0.0f;
    }
    else if (Rotation->Pitch >= 90.0f) {
        Pitch = 90.0f;
        Bank = 0;
    }
    else {
        Pitch = Rotation->Pitch;
        Bank = Rotation->Bank;
    }

    SineCosine(Heading, &sinh, &cosh);
    SineCosine(Pitch, &sinp, &cosp);
    SineCosine(Bank, &sinb, &cosb);

    Target->x = sinh * cosp;
    Target->y = -sinp;
    Target->z = cosh * cosp;

    Right->x = cosh * cosb + sinh * sinp * sinb;
    Right->y = cosp * sinb;
    Right->z = -sinh * cosb + cosh * sinp * sinb;

    Up->x = -cosh * sinb + sinh * sinp * cosb;
    Up->y = cosp * cosb;
    Up->z = sinh * sinb + cosh * sinp * cosb;
}

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
        Cosine(Rad), 0.0f,   Sine(Rad),    0.0f,
               0.0f, 1.0f,        0.0f,    0.0f,
         -Sine(Rad), 0.0f, Cosine(Rad),    0.0f,
               0.0f, 0.0f,        0.0f,    1.0f
    };

    return Result;
}

inline Mat3x3 MakeRotationAroundY3x3(real32 Rad)
{
    Mat3x3 Result = {
        Cosine(Rad), 0.0f,   Sine(Rad),
               0.0f, 1.0f,        0.0f,
         -Sine(Rad), 0.0f, Cosine(Rad)
    };

    return Result;
}

inline Mat3x3 MakeRotationAroundX3x3(real32 Rad)
{
    Mat3x3 Result = {
        1.0f,        0.0f,        0.0f,
        0.0f, Cosine(Rad),  -Sine(Rad),
        0.0f,   Sine(Rad), Cosine(Rad)
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

inline Mat3x3 MakeRotationAroundZ3x3(real32 Rad)
{
    Mat3x3 Result = {
        Cosine(Rad),   -Sine(Rad),    0.0f,
          Sine(Rad),  Cosine(Rad),    0.0f,
               0.0f,         0.0f,    1.0f
    };

    return Result;
}

#endif