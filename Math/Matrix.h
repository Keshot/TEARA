#ifndef _TEARA_MATH_MATRIX_H_
#define _TEARA_MATH_MATRIX_H_

#include "Vector.h"

// TODO(ismail): convert all to SIMD
// TODO(ismail): add tests

struct mat2 {
    real32 *operator[](i32 Index);
    mat2 operator-() const;
    mat2 operator-(const mat2 &Other) const;
    mat2 operator+(const mat2 &Other) const;
    mat2 operator*(real32 Scalar) const;
    mat2 operator*(const mat2 &Other) const;
    vec2 operator*(const vec2 &Vec) const;
    mat2 &operator*=(real32 Scalar);
    mat2 &operator-=(const mat2 &Other);
    mat2 &operator+=(const mat2 &Other);
    mat2 &operator*=(const mat2 &Other);

    real32 mat[2][2];
};

typedef mat2 mat2x2;

inline real32 *mat2::operator[](i32 Index)
{
    Assert(Index < (sizeof(mat) / sizeof(*mat)));

    real32 *Result = (real32*)(&mat[Index]);

    return Result;
}

inline mat2 mat2::operator-() const
{
    mat2 Result = {
        -mat[0][0], -mat[0][1],
        -mat[1][0], -mat[1][1]
    };

    return Result;
}

inline mat2 mat2::operator-(const mat2 &Other) const
{
    mat2 Result = {
        mat[0][0] - Other.mat[0][0], mat[0][1] - Other.mat[0][1],
        mat[1][0] - Other.mat[1][0], mat[1][1] - Other.mat[1][1]
    };

    return Result;
}

inline mat2 mat2::operator+(const mat2 &Other) const
{
    mat2 Result = {
        mat[0][0] + Other.mat[0][0], mat[0][1] + Other.mat[0][1],
        mat[1][0] + Other.mat[1][0], mat[1][1] + Other.mat[1][1]
    };

    return Result;
}

inline mat2 mat2::operator*(real32 Scalar) const
{
    mat2 Result = {
        mat[0][0] * Scalar, mat[0][1] * Scalar,
        mat[1][0] * Scalar, mat[1][1] * Scalar,
    };

    return Result;
}

inline mat2 mat2::operator*(const mat2 &Other) const
{
    mat2 Result = {
        mat[0][0] * Other.mat[0][0] + mat[0][1] * Other.mat[1][0], mat[0][0] * Other.mat[0][1] + mat[0][1] * Other.mat[1][1],
        mat[1][0] * Other.mat[0][0] + mat[1][1] * Other.mat[1][0], mat[1][0] * Other.mat[0][1] + mat[1][1] * Other.mat[1][1]
    };

    return Result;
}

inline vec2 mat2::operator*(const vec2 &Vec) const
{
    vec2 Result = {
        mat[0][0] * Vec.x + mat[0][1] * Vec.y,
        mat[1][0] * Vec.x + mat[1][1] * Vec.y,
    };

    return Result;
}

inline mat2 &mat2::operator*=(real32 Scalar)
{
    mat[0][0] *= Scalar; mat[0][1] *= Scalar;
    mat[1][0] *= Scalar; mat[1][1] *= Scalar;

    return *this;
}

inline mat2 &mat2::operator-=(const mat2 &Other)
{
    mat[0][0] -= Other.mat[0][0]; mat[0][1] -= Other.mat[0][1];
    mat[1][0] -= Other.mat[1][0]; mat[1][1] -= Other.mat[1][1]; 

    return *this;
}

inline mat2 &mat2::operator+=(const mat2 &Other)
{
    mat[0][0] += Other.mat[0][0]; mat[0][1] += Other.mat[0][1];
    mat[1][0] += Other.mat[1][0]; mat[1][1] += Other.mat[1][1]; 

    return *this;
}

inline mat2 &mat2::operator*=(const mat2 &Other)
{
    real32 x, y;

    x = mat[0][0];
    y = mat[0][1];

    mat[0][0] = x * Other.mat[0][0] + y * Other.mat[1][0]; mat[0][1] = x * Other.mat[0][1] + y * Other.mat[1][1];

    x = mat[1][0];
    y = mat[1][1];

    mat[1][0] = x * Other.mat[0][0] + y * Other.mat[1][0]; mat[1][0] = x * Other.mat[0][1] + y * Other.mat[1][1];


    return *this;
}

struct mat3 {
    real32 *operator[](i32 Index);
    mat3 operator-() const;
    mat3 operator-(const mat3 &Other) const;
    mat3 operator+(const mat3 &Other) const;
    mat3 operator*(real32 Scalar) const;
    mat3 operator*(const mat3 &Other) const;
    vec3 operator*(const vec3 &Vec) const;
    mat3 &operator*=(real32 Scalar);
    mat3 &operator-=(const mat3 &Other);
    mat3 &operator+=(const mat3 &Other);
    mat3 &operator*=(const mat3 &Other);

    void Transpose();

    real32 mat[3][3];
};

typedef mat3 mat3x3;

inline real32 *mat3::operator[](i32 Index)
{
    Assert(Index < (sizeof(mat) / sizeof(*mat)));

    real32 *Result = (real32*)(&mat[Index]);

    return Result;
}

inline mat3 mat3::operator-() const
{
    mat3 Result = {
        -mat[0][0], -mat[0][1], -mat[0][2],
        -mat[1][0], -mat[1][1], -mat[1][2],
        -mat[2][0], -mat[2][1], -mat[2][2],
    };

    return Result;
}

inline mat3 mat3::operator-(const mat3 &Other) const
{
    mat3 Result = {
        mat[0][0] - Other.mat[0][0], mat[0][1] - Other.mat[0][1], mat[0][2] - Other.mat[0][2],
        mat[1][0] - Other.mat[1][0], mat[1][1] - Other.mat[1][1], mat[1][2] - Other.mat[1][2],
        mat[2][0] - Other.mat[2][0], mat[2][1] - Other.mat[2][1], mat[2][2] - Other.mat[2][2]
    };

    return Result;
}

inline mat3 mat3::operator+(const mat3 &Other) const
{
    mat3 Result = {
        mat[0][0] + Other.mat[0][0], mat[0][1] + Other.mat[0][1], mat[0][2] + Other.mat[0][2],
        mat[1][0] + Other.mat[1][0], mat[1][1] + Other.mat[1][1], mat[1][2] + Other.mat[1][2],
        mat[2][0] + Other.mat[2][0], mat[2][1] + Other.mat[2][1], mat[2][2] + Other.mat[2][2]
    };

    return Result;
}

inline mat3 mat3::operator*(real32 Scalar) const
{
    mat3 Result = {
        mat[0][0] * Scalar, mat[0][1] * Scalar, mat[0][2] * Scalar,
        mat[1][0] * Scalar, mat[1][1] * Scalar, mat[1][2] * Scalar,
        mat[2][0] * Scalar, mat[2][1] * Scalar, mat[2][2] * Scalar
    };

    return Result;
}

inline mat3 mat3::operator*(const mat3 &Other) const
{
    // TODO(ismail): check can i optimise this?

    mat3 Result = {
        mat[0][0] * Other.mat[0][0] + mat[0][1] * Other.mat[1][0] + mat[0][2] * Other.mat[2][0], // i = 1 j = 1
        mat[0][0] * Other.mat[0][1] + mat[0][1] * Other.mat[1][1] + mat[0][2] * Other.mat[2][1], // i = 1 j = 2
        mat[0][0] * Other.mat[0][2] + mat[0][1] * Other.mat[1][2] + mat[0][2] * Other.mat[2][2], // i = 1 j = 3

        mat[1][0] * Other.mat[0][0] + mat[1][1] * Other.mat[1][0] + mat[1][2] * Other.mat[2][0], // i = 2 j = 1
        mat[1][0] * Other.mat[0][1] + mat[1][1] * Other.mat[1][1] + mat[1][2] * Other.mat[2][1], // i = 2 j = 2
        mat[1][0] * Other.mat[0][2] + mat[1][1] * Other.mat[1][2] + mat[1][2] * Other.mat[2][2], // i = 2 j = 3

        mat[2][0] * Other.mat[0][0] + mat[2][1] * Other.mat[1][0] + mat[2][2] * Other.mat[2][0], // i = 3 j = 1
        mat[2][0] * Other.mat[0][1] + mat[2][1] * Other.mat[1][1] + mat[2][2] * Other.mat[2][1], // i = 3 j = 2
        mat[2][0] * Other.mat[0][2] + mat[2][1] * Other.mat[1][2] + mat[2][2] * Other.mat[2][2], // i = 3 j = 3
    };

    return Result;
}

inline vec3 mat3::operator*(const vec3 &Vec) const
{
    vec3 Result = {
        mat[0][0] * Vec.x + mat[0][1] * Vec.y + mat[0][2] * Vec.z,
        mat[1][0] * Vec.x + mat[1][1] * Vec.y + mat[1][2] * Vec.z,
        mat[2][0] * Vec.x + mat[2][1] * Vec.y + mat[2][2] * Vec.z,
    };

    return Result;
}

inline mat3 &mat3::operator*=(real32 Scalar)
{
    mat[0][0] *= Scalar; mat[0][1] *= Scalar; mat[0][2] *= Scalar;
    mat[1][0] *= Scalar; mat[1][1] *= Scalar; mat[1][2] *= Scalar;
    mat[2][0] *= Scalar; mat[2][1] *= Scalar; mat[2][2] *= Scalar;

    return *this;
}

inline mat3 &mat3::operator-=(const mat3 &Other)
{
    mat[0][0] -= Other.mat[0][0]; mat[0][1] -= Other.mat[0][1]; mat[0][2] -= Other.mat[0][2];
    mat[1][0] -= Other.mat[1][0]; mat[1][1] -= Other.mat[1][1]; mat[1][2] -= Other.mat[1][2];
    mat[2][0] -= Other.mat[2][0]; mat[2][1] -= Other.mat[2][1]; mat[2][2] -= Other.mat[2][2];

    return *this;
}

inline mat3 &mat3::operator+=(const mat3 &Other)
{
    mat[0][0] += Other.mat[0][0]; mat[0][1] += Other.mat[0][1]; mat[0][2] += Other.mat[0][2];
    mat[1][0] += Other.mat[1][0]; mat[1][1] += Other.mat[1][1]; mat[1][2] += Other.mat[1][2];
    mat[2][0] += Other.mat[2][0]; mat[2][1] += Other.mat[2][1]; mat[2][2] += Other.mat[2][2];

    return *this;
}

inline mat3 &mat3::operator*=(const mat3 &Other)
{
    real32 x, y, z;

    x = mat[0][0];
    y = mat[0][1];
    z = mat[0][2];

    mat[0][0] = x * Other.mat[0][0] + y * Other.mat[1][0] + z * Other.mat[2][0]; // i = 1 j = 1
    mat[0][1] = x * Other.mat[0][1] + y * Other.mat[1][1] + z * Other.mat[2][1], // i = 1 j = 2
    mat[0][2] = x * Other.mat[0][2] + y * Other.mat[1][2] + z * Other.mat[2][2], // i = 1 j = 3

    x = mat[1][0];
    y = mat[1][1];
    z = mat[1][2];

    mat[1][0] = x * Other.mat[0][0] + y * Other.mat[1][0] + z * Other.mat[2][0]; // i = 2 j = 1
    mat[1][1] = x * Other.mat[0][1] + y * Other.mat[1][1] + z * Other.mat[2][1], // i = 2 j = 2
    mat[1][2] = x * Other.mat[0][2] + y * Other.mat[1][2] + z * Other.mat[2][2], // i = 2 j = 3

    x = mat[2][0];
    y = mat[2][1];
    z = mat[2][2];

    mat[2][0] = x * Other.mat[0][0] + y * Other.mat[1][0] + z * Other.mat[2][0]; // i = 3 j = 1
    mat[2][1] = x * Other.mat[0][1] + y * Other.mat[1][1] + z * Other.mat[2][1]; // i = 3 j = 2
    mat[2][2] = x * Other.mat[0][2] + y * Other.mat[1][2] + z * Other.mat[2][2]; // i = 3 j = 3 

    return *this;
}

inline void mat3::Transpose()
{
    // | 1  1  1 |    | 1  2  3 |
    // | 2  2  2 | to | 1  2  3 |
    // | 3  3  3 |    | 1  2  3 |

    real32 tmp;

    tmp = mat[0][1];
    mat[0][1] = mat[1][0];
    mat[1][0] = tmp;

    // | 1  2  1 |
    // | 1  2  2 |
    // | 3  3  3 |

    tmp = mat[0][2];
    mat[0][2] = mat[2][0];
    mat[2][0] = tmp;

    // | 1  2  3 |
    // | 1  2  2 |
    // | 1  3  3 |

    tmp = mat[1][2];
    mat[1][2] = mat[2][1];
    mat[2][1] = tmp;

    // | 1  2  3 |
    // | 1  2  3 |
    // | 1  2  3 |
}

struct mat4 {
    real32 mat[4][4];

    const real32* operator[](i32 Index) const;
    real32 *operator[](i32 Index);
    mat4 operator-() const;
    mat4 operator-(const mat4 &Other) const;
    mat4 operator+(const mat4 &Other) const;
    mat4 operator*(real32 Scalar) const;
    mat4 operator*(const mat4 &Other) const;
    vec4 operator*(const vec4 &Vec) const;
    mat4 &operator*=(real32 Scalar);
    mat4 &operator-=(const mat4 &Other);
    mat4 &operator+=(const mat4 &Other);
    mat4 &operator*=(const mat4 &Other);
};

typedef mat4 mat4x4;

real32 *mat4::operator[](i32 Index)
{
    Assert(Index < (sizeof(mat) / sizeof(*mat)));

    real32 *Result = (real32*)(&mat[Index]);

    return Result;
}

const real32* mat4::operator[](i32 Index) const
{    
    Assert(Index < (sizeof(mat) / sizeof(*mat)));

    real32 *Result = (real32*)(&mat[Index]);

    return Result;
}

mat4 mat4::operator-() const
{
    mat4 Result = {
        -mat[0][0], -mat[0][1], -mat[0][2], -mat[0][3],
        -mat[1][0], -mat[1][1], -mat[1][2], -mat[1][3],
        -mat[2][0], -mat[2][1], -mat[2][2], -mat[2][3],
        -mat[3][0], -mat[3][1], -mat[3][2], -mat[3][3]
    };

    return Result;
}

mat4 mat4::operator-(const mat4 &Other) const
{
    mat4 Result = {
        mat[0][0] - Other.mat[0][0], mat[0][1] - Other.mat[0][1], mat[0][2] - Other.mat[0][2], mat[0][3] - Other.mat[0][3],
        mat[1][0] - Other.mat[1][0], mat[1][1] - Other.mat[1][1], mat[1][2] - Other.mat[1][2], mat[1][3] - Other.mat[1][3],
        mat[2][0] - Other.mat[2][0], mat[2][1] - Other.mat[2][1], mat[2][2] - Other.mat[2][2], mat[2][3] - Other.mat[2][3],
        mat[3][0] - Other.mat[3][0], mat[3][1] - Other.mat[3][1], mat[3][2] - Other.mat[3][2], mat[3][3] - Other.mat[3][3]
    };

    return Result;
}

mat4 mat4::operator+(const mat4 &Other) const
{
    mat4 Result = {
        mat[0][0] + Other.mat[0][0], mat[0][1] + Other.mat[0][1], mat[0][2] + Other.mat[0][2], mat[0][3] + Other.mat[0][3],
        mat[1][0] + Other.mat[1][0], mat[1][1] + Other.mat[1][1], mat[1][2] + Other.mat[1][2], mat[1][3] + Other.mat[1][3],
        mat[2][0] + Other.mat[2][0], mat[2][1] + Other.mat[2][1], mat[2][2] + Other.mat[2][2], mat[2][3] + Other.mat[2][3],
        mat[3][0] + Other.mat[3][0], mat[3][1] + Other.mat[3][1], mat[3][2] + Other.mat[3][2], mat[3][3] + Other.mat[3][3]
    };

    return Result;
}

mat4 mat4::operator*(real32 Scalar) const
{
    mat4 Result = {
        mat[0][0] * Scalar, mat[0][1] * Scalar, mat[0][2] * Scalar, mat[0][3] * Scalar,
        mat[1][0] * Scalar, mat[1][1] * Scalar, mat[1][2] * Scalar, mat[1][3] * Scalar,
        mat[2][0] * Scalar, mat[2][1] * Scalar, mat[2][2] * Scalar, mat[2][3] * Scalar,
        mat[3][0] * Scalar, mat[3][1] * Scalar, mat[3][2] * Scalar, mat[3][3] * Scalar
    };

    return Result;
}

mat4 mat4::operator*(const mat4 &Other) const
{
    // TODO(ismail): check can i optimise this?

    mat4 Result = {
        mat[0][0] * Other.mat[0][0] + mat[0][1] * Other.mat[1][0] + mat[0][2] * Other.mat[2][0] + mat[0][3] * Other.mat[3][0], // i = 1 j = 1
        mat[0][0] * Other.mat[0][1] + mat[0][1] * Other.mat[1][1] + mat[0][2] * Other.mat[2][1] + mat[0][3] * Other.mat[3][1], // i = 1 j = 2
        mat[0][0] * Other.mat[0][2] + mat[0][1] * Other.mat[1][2] + mat[0][2] * Other.mat[2][2] + mat[0][3] * Other.mat[3][2], // i = 1 j = 3
        mat[0][0] * Other.mat[0][3] + mat[0][1] * Other.mat[1][3] + mat[0][2] * Other.mat[2][3] + mat[0][3] * Other.mat[3][3], // i = 1 j = 4

        mat[1][0] * Other.mat[0][0] + mat[1][1] * Other.mat[1][0] + mat[1][2] * Other.mat[2][0] + mat[1][3] * Other.mat[3][0], // i = 2 j = 1
        mat[1][0] * Other.mat[0][1] + mat[1][1] * Other.mat[1][1] + mat[1][2] * Other.mat[2][1] + mat[1][3] * Other.mat[3][1], // i = 2 j = 2
        mat[1][0] * Other.mat[0][2] + mat[1][1] * Other.mat[1][2] + mat[1][2] * Other.mat[2][2] + mat[1][3] * Other.mat[3][2], // i = 2 j = 3
        mat[1][0] * Other.mat[0][3] + mat[1][1] * Other.mat[1][3] + mat[1][2] * Other.mat[2][3] + mat[1][3] * Other.mat[3][3], // i = 2 j = 4

        mat[2][0] * Other.mat[0][0] + mat[2][1] * Other.mat[1][0] + mat[2][2] * Other.mat[2][0] + mat[2][3] * Other.mat[3][0], // i = 3 j = 1
        mat[2][0] * Other.mat[0][1] + mat[2][1] * Other.mat[1][1] + mat[2][2] * Other.mat[2][1] + mat[2][3] * Other.mat[3][1], // i = 3 j = 2
        mat[2][0] * Other.mat[0][2] + mat[2][1] * Other.mat[1][2] + mat[2][2] * Other.mat[2][2] + mat[2][3] * Other.mat[3][2], // i = 3 j = 3
        mat[2][0] * Other.mat[0][3] + mat[2][1] * Other.mat[1][3] + mat[2][2] * Other.mat[2][3] + mat[2][3] * Other.mat[3][3], // i = 3 j = 4

        mat[3][0] * Other.mat[0][0] + mat[3][1] * Other.mat[1][0] + mat[3][2] * Other.mat[2][0] + mat[3][3] * Other.mat[3][0], // i = 4 j = 1
        mat[3][0] * Other.mat[0][1] + mat[3][1] * Other.mat[1][1] + mat[3][2] * Other.mat[2][1] + mat[3][3] * Other.mat[3][1], // i = 4 j = 2
        mat[3][0] * Other.mat[0][2] + mat[3][1] * Other.mat[1][2] + mat[3][2] * Other.mat[2][2] + mat[3][3] * Other.mat[3][2], // i = 4 j = 3
        mat[3][0] * Other.mat[0][3] + mat[3][1] * Other.mat[1][3] + mat[3][2] * Other.mat[2][3] + mat[3][3] * Other.mat[3][3], // i = 4 j = 4
    };

    return Result;
}

vec4 mat4::operator*(const vec4 &Vec) const
{
    vec4 Result = {
        mat[0][0] * Vec.x + mat[0][1] * Vec.y + mat[0][2] * Vec.z + mat[0][3] * Vec.w,
        mat[1][0] * Vec.x + mat[1][1] * Vec.y + mat[1][2] * Vec.z + mat[1][3] * Vec.w,
        mat[2][0] * Vec.x + mat[2][1] * Vec.y + mat[2][2] * Vec.z + mat[2][3] * Vec.w,
        mat[3][0] * Vec.x + mat[3][1] * Vec.y + mat[3][2] * Vec.z + mat[3][3] * Vec.w
    };

    return Result;
}

mat4 &mat4::operator*=(real32 Scalar)
{
    mat[0][0] *= Scalar; mat[0][1] *= Scalar; mat[0][2] *= Scalar; mat[0][3] *= Scalar;
    mat[1][0] *= Scalar; mat[1][1] *= Scalar; mat[1][2] *= Scalar; mat[1][3] *= Scalar;
    mat[2][0] *= Scalar; mat[2][1] *= Scalar; mat[2][2] *= Scalar; mat[2][3] *= Scalar;
    mat[3][0] *= Scalar; mat[3][1] *= Scalar; mat[3][2] *= Scalar; mat[3][3] *= Scalar;

    return *this;
}

mat4 &mat4::operator-=(const mat4 &Other)
{
    mat[0][0] -= Other.mat[0][0]; mat[0][1] -= Other.mat[0][1]; mat[0][2] -= Other.mat[0][2]; mat[0][3] -= Other.mat[0][3];
    mat[1][0] -= Other.mat[1][0]; mat[1][1] -= Other.mat[1][1]; mat[1][2] -= Other.mat[1][2]; mat[1][3] -= Other.mat[1][3];
    mat[2][0] -= Other.mat[2][0]; mat[2][1] -= Other.mat[2][1]; mat[2][2] -= Other.mat[2][2]; mat[2][3] -= Other.mat[2][3];
    mat[3][0] -= Other.mat[3][0]; mat[3][1] -= Other.mat[3][1]; mat[3][2] -= Other.mat[3][2]; mat[3][3] -= Other.mat[3][3];

    return *this;
}

mat4 &mat4::operator+=(const mat4 &Other)
{
    mat[0][0] += Other.mat[0][0]; mat[0][1] += Other.mat[0][1]; mat[0][2] += Other.mat[0][2]; mat[0][3] += Other.mat[0][3];
    mat[1][0] += Other.mat[1][0]; mat[1][1] += Other.mat[1][1]; mat[1][2] += Other.mat[1][2]; mat[1][3] += Other.mat[1][3];
    mat[2][0] += Other.mat[2][0]; mat[2][1] += Other.mat[2][1]; mat[2][2] += Other.mat[2][2]; mat[2][3] += Other.mat[2][3];
    mat[3][0] += Other.mat[3][0]; mat[3][1] += Other.mat[3][1]; mat[3][2] += Other.mat[3][2]; mat[3][3] += Other.mat[3][3];

    return *this;
}

mat4 &mat4::operator*=(const mat4 &Other)
{
    real32 x, y, z, w;

    x = mat[0][0];
    y = mat[0][1];
    z = mat[0][2];
    w = mat[0][3];

    mat[0][0] = x * Other.mat[0][0] + y * Other.mat[1][0] + z * Other.mat[2][0] + w * Other.mat[3][0]; // i = 1 j = 1
    mat[0][1] = x * Other.mat[0][1] + y * Other.mat[1][1] + z * Other.mat[2][1] + w * Other.mat[3][1]; // i = 1 j = 2
    mat[0][2] = x * Other.mat[0][2] + y * Other.mat[1][2] + z * Other.mat[2][2] + w * Other.mat[3][2]; // i = 1 j = 3
    mat[0][3] = x * Other.mat[0][3] + y * Other.mat[1][3] + z * Other.mat[2][3] + w * Other.mat[3][3]; // i = 1 j = 4

    x = mat[1][0];
    y = mat[1][1];
    z = mat[1][2];
    w = mat[1][3];

    mat[1][0] = x * Other.mat[0][0] + y * Other.mat[1][0] + z * Other.mat[2][0] + w * Other.mat[3][0]; // i = 2 j = 1
    mat[1][1] = x * Other.mat[0][1] + y * Other.mat[1][1] + z * Other.mat[2][1] + w * Other.mat[3][1]; // i = 2 j = 2
    mat[1][2] = x * Other.mat[0][2] + y * Other.mat[1][2] + z * Other.mat[2][2] + w * Other.mat[3][2]; // i = 2 j = 3
    mat[1][3] = x * Other.mat[0][3] + y * Other.mat[1][3] + z * Other.mat[2][3] + w * Other.mat[3][3]; // i = 2 j = 4

    x = mat[2][0];
    y = mat[2][1];
    z = mat[2][2];
    w = mat[2][3];

    mat[2][0] = x * Other.mat[0][0] + y * Other.mat[1][0] + z * Other.mat[2][0] + w * Other.mat[3][0]; // i = 3 j = 1
    mat[2][1] = x * Other.mat[0][1] + y * Other.mat[1][1] + z * Other.mat[2][1] + w * Other.mat[3][1]; // i = 3 j = 2
    mat[2][2] = x * Other.mat[0][2] + y * Other.mat[1][2] + z * Other.mat[2][2] + w * Other.mat[3][2]; // i = 3 j = 3
    mat[2][3] = x * Other.mat[0][3] + y * Other.mat[1][3] + z * Other.mat[2][3] + w * Other.mat[3][3]; // i = 3 j = 4

    x = mat[3][0];
    y = mat[3][1];
    z = mat[3][2];
    w = mat[3][3];

    mat[3][0] = x * Other.mat[0][0] + y * Other.mat[1][0] + z * Other.mat[2][0] + w * Other.mat[3][0]; // i = 4 j = 1
    mat[3][1] = x * Other.mat[0][1] + y * Other.mat[1][1] + z * Other.mat[2][1] + w * Other.mat[3][1]; // i = 4 j = 2
    mat[3][2] = x * Other.mat[0][2] + y * Other.mat[1][2] + z * Other.mat[2][2] + w * Other.mat[3][2]; // i = 4 j = 3
    mat[3][3] = x * Other.mat[0][3] + y * Other.mat[1][3] + z * Other.mat[2][3] + w * Other.mat[3][3]; // i = 4 j = 4

    return *this;
}

const mat4 Identity4 = 
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

const mat3 Identity3 = 
{
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};

#endif