#ifndef _TEARA_MATH_TRANSFORM_H_
#define _TEARA_MATH_TRANSFORM_H_

#include "Quat.h"

inline void ScaleFromVec(vec3& ScaleFactor, mat4& Result)
{
    Result = {
        ScaleFactor.x,          0.0f,           0.0f,  0.0f,
                 0.0f, ScaleFactor.y,           0.0f,  0.0f,
                 0.0f,          0.0f,  ScaleFactor.z,  0.0f,
                 0.0f,          0.0f,           0.0f,  1.0f,
    };
}

inline void ScaleFromVecRelative(vec3& ScaleFactor, vec3& RelativeTo, mat4& Result)
{
    real32 x = 1.0f / RelativeTo.x;
    real32 y = 1.0f / RelativeTo.y;
    real32 z = 1.0f / RelativeTo.z;

    Result = {
         ScaleFactor.x * x,               0.0f,               0.0f, 0.0f,
                      0.0f,  ScaleFactor.y * y,               0.0f, 0.0f,
                      0.0f,               0.0f,  ScaleFactor.z * z, 0.0f,
                      0.0f,               0.0f,               0.0f, 1.0f,
    };
}

inline void InverseScaleFromArr(real32 *Scale, mat4& Result)
{
    real32 x = 1.0f / Scale[_x_];
    real32 y = 1.0f / Scale[_y_];
    real32 z = 1.0f / Scale[_z_];

    Result = {
           x, 0.0f, 0.0f, 0.0f,
        0.0f,    y, 0.0f, 0.0f,
        0.0f, 0.0f,    z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
}

inline void InverseScaleFromArr(real32 *Scale, mat3& Result)
{
    real32 x = 1.0f / Scale[_x_];
    real32 y = 1.0f / Scale[_y_];
    real32 z = 1.0f / Scale[_z_];

    Result = {
           x,   0.0f,    0.0f,
        0.0f,      y,    0.0f,
        0.0f,   0.0f,       z
    };
}

inline void TranslationFromVec(vec3& Vec, mat4& Result)
{
    Result = {
        1.0f,   0.0f,   0.0f,  Vec.x,
        0.0f,   1.0f,   0.0f,  Vec.y,
        0.0f,   0.0f,   1.0f,  Vec.z,
        0.0f,   0.0f,   0.0f,   1.0f,
    };
}

inline void InverseTranslationFromVec(vec3& Vec, mat4& Result)
{
    Result = {
        1.0f,   0.0f,   0.0f,  -Vec.x,
        0.0f,   1.0f,   0.0f,  -Vec.y,
        0.0f,   0.0f,   1.0f,  -Vec.z,
        0.0f,   0.0f,   0.0f,    1.0f,
    };
}

void InverseTranslationFromArr(real32 *Translation, mat4& Result)
{
    Result = {
        1.0f, 0.0f, 0.0f, -Translation[_x_],
        0.0f, 1.0f, 0.0f, -Translation[_y_],
        0.0f, 0.0f, 1.0f, -Translation[_z_],
        0.0f, 0.0f, 0.0f, 1.0f,
    };
}

inline void MakePerspProjection(mat4& Result, real32 FovInDegree, real32 AspectRatio, real32 NearZ, real32 FarZ)
{
    real32 d = 1 / Tan(DEGREE_TO_RAD(FovInDegree / 2.0f));
    
    real32 x = d / AspectRatio;
    real32 y = d;

    real32 ClipDistance = NearZ - FarZ;
    
    real32 a = (-FarZ - NearZ) / ClipDistance;
    real32 b = (2 * NearZ * FarZ) / ClipDistance;

    Result = {
           x, 0.0f, 0.0f, 0.0f,
        0.0f,    y, 0.0f, 0.0f,
        0.0f, 0.0f,    a,    b,
        0.0f, 0.0f, 1.0f, 0.0f,
    };
}

inline void MakeOrthoProjection(mat4& Result, real32 Right, real32 Left, real32 Top, real32 Bot, real32 Far, real32 Near)
{
    real32 XDen = Right - Left;
    real32 YDen = Top - Bot;
    real32 ZDen = Far - Near;

    Result = {
        2.0f / XDen,        0.0f,        0.0f, (-Left - Right) / XDen,
               0.0f, 2.0f / YDen,        0.0f,    (-Bot - Top) / YDen,
               0.0f,        0.0f, 2.0f / ZDen,   (-Near - Far) / ZDen,
               0.0f,        0.0f,        0.0f,                   1.0f,
    };
}

#endif