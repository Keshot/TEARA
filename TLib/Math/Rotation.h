#ifndef _TEARA_MATH_ROTATION_H_
#define _TEARA_MATH_ROTATION_H_

#include "Matrix.h"

struct Rotation {
    Rotation();
    Rotation(real32 head, real32 pitch, real32 bank);

    void ObjectToUpright(mat3& Result);
    void UprightToObject(mat3& Result);
    void ObjectToUpright(mat4& Result);
    void UprightToObject(mat4& Result);
    void ToVec(vec3& Target, vec3& Up, vec3& Right);

    real32 h;
    real32 p;
    real32 b;
};

inline Rotation::Rotation()
    : h(0.0f)
    , p(0.0f)
    , b(0.0f)
{
}

inline Rotation::Rotation(real32 head, real32 pitch, real32 bank)
    : h(head)
    , p(pitch)
    , b(bank)
{
}

// TODO(Ismail): correct for Gimbal Lock
inline void Rotation::ObjectToUpright(mat4& Result)
{
    real32 Cosh, Sinh;
    real32 Cosp, Sinp;
    real32 Cosb, Sinb;

    real32 Heading = DEGREE_TO_RAD(h);
    real32 Pitch = DEGREE_TO_RAD(p);
    real32 Bank = DEGREE_TO_RAD(b);

    Sinh = Sine(Heading);
    Cosh = Cosine(Heading);

    Sinp = Sine(Pitch);
    Cosp = Cosine(Pitch);

    Sinb = Sine(Bank);
    Cosb = Cosine(Bank);

    Result = {
        Cosh * Cosb + Sinh * Sinp * Sinb, -Cosh * Sinb + Sinh * Sinp * Cosb, Sinh * Cosp, 0.0f,
                             Cosp * Sinb,                       Cosp * Cosb,       -Sinp, 0.0f,
       -Sinh * Cosb + Cosh * Sinp * Sinb,  Sinh * Sinb + Cosh * Sinp * Cosb, Cosh * Cosp, 0.0f,
                                    0.0f,                              0.0f,        0.0f, 1.0f
    };
}

inline void Rotation::UprightToObject(mat4& Result)
{
    real32 Cosh, Sinh;
    real32 Cosp, Sinp;
    real32 Cosb, Sinb;

    real32 Heading = DEGREE_TO_RAD(h);
    real32 Pitch = DEGREE_TO_RAD(p);
    real32 Bank = DEGREE_TO_RAD(b);

    Sinh = Sine(Heading);
    Cosh = Cosine(Heading);

    Sinp = Sine(Pitch);
    Cosp = Cosine(Pitch);

    Sinb = Sine(Bank);
    Cosb = Cosine(Bank);

    Result = {
         Cosh * Cosb + Sinh * Sinp * Sinb,  Cosp * Sinb,  -Sinh * Cosb + Cosh * Sinp * Sinb, 0.0f,
        -Cosh * Sinb + Sinh * Sinp * Cosb,  Cosp * Cosb,   Sinh * Sinb + Cosh * Sinp * Cosb, 0.0f,
                              Sinh * Cosp,        -Sinp,                        Cosh * Cosp, 0.0f,
                                     0.0f,         0.0f,                               0.0f, 1.0f,
    };
}

inline void Rotation::ObjectToUpright(mat3& Result)
{
    real32 Cosh, Sinh;
    real32 Cosp, Sinp;
    real32 Cosb, Sinb;

    real32 Heading = DEGREE_TO_RAD(h);
    real32 Pitch = DEGREE_TO_RAD(p);
    real32 Bank = DEGREE_TO_RAD(b);

    Sinh = Sine(Heading);
    Cosh = Cosine(Heading);

    Sinp = Sine(Pitch);
    Cosp = Cosine(Pitch);

    Sinb = Sine(Bank);
    Cosb = Cosine(Bank);

    Result = {
        Cosh * Cosb + Sinh * Sinp * Sinb, -Cosh * Sinb + Sinh * Sinp * Cosb, Sinh * Cosp,
                             Cosp * Sinb,                       Cosp * Cosb,       -Sinp,
       -Sinh * Cosb + Cosh * Sinp * Sinb,  Sinh * Sinb + Cosh * Sinp * Cosb, Cosh * Cosp,
    };
}

inline void Rotation::UprightToObject(mat3& Result)
{
    real32 Cosh, Sinh;
    real32 Cosp, Sinp;
    real32 Cosb, Sinb;

    real32 Heading = DEGREE_TO_RAD(h);
    real32 Pitch = DEGREE_TO_RAD(p);
    real32 Bank = DEGREE_TO_RAD(b);

    Sinh = Sine(Heading);
    Cosh = Cosine(Heading);

    Sinp = Sine(Pitch);
    Cosp = Cosine(Pitch);

    Sinb = Sine(Bank);
    Cosb = Cosine(Bank);

    Result = {
         Cosh * Cosb + Sinh * Sinp * Sinb,  Cosp * Sinb,  -Sinh * Cosb + Cosh * Sinp * Sinb,
        -Cosh * Sinb + Sinh * Sinp * Cosb,  Cosp * Cosb,   Sinh * Sinb + Cosh * Sinp * Cosb,
                              Sinh * Cosp,        -Sinp,                        Cosh * Cosp,
    };
}

inline void Rotation::ToVec(vec3& Target, vec3& Up, vec3& Right)
{
    real32 Cosh, Sinh;
    real32 Cosp, Sinp;
    real32 Cosb, Sinb;

    real32 Heading = DEGREE_TO_RAD(h);
    real32 Pitch = DEGREE_TO_RAD(p);
    real32 Bank = DEGREE_TO_RAD(b);

    Sinh = Sine(Heading);
    Cosh = Cosine(Heading);

    Sinp = Sine(Pitch);
    Cosp = Cosine(Pitch);

    Sinb = Sine(Bank);
    Cosb = Cosine(Bank);

    Target = {
        Sinh * Cosp,
              -Sinp,
        Cosh * Cosp,
    };

    Up = {
       -Cosh * Sinb + Sinh * Sinp * Cosb,
                             Cosp * Cosb,
        Sinh * Sinb + Cosh * Sinp * Cosb
    };

    Right = {
        Cosh * Cosb + Sinh * Sinp * Sinb,
                             Cosp * Sinb,
       -Sinh * Cosb + Cosh * Sinp * Sinb
    };
}

#endif