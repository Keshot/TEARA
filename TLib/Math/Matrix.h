#ifndef _TEARA_LIB_MATH_MATRIX_H_
#define _TEARA_LIB_MATH_MATRIX_H_

#include "TLib/Math/Math.h"
#include "TLib/Math/Vector.h"

// TODO(ismail): convert all to SIMD
// TODO(ismail): add tests

struct Mat2x2 {
    real32 Matrix[2][2];

    real32 *operator[](i32 Index);
    Mat2x2 operator-() const;
    Mat2x2 operator-(const Mat2x2 &Other) const;
    Mat2x2 operator+(const Mat2x2 &Other) const;
    Mat2x2 operator*(real32 Scalar) const;
    Mat2x2 operator*(const Mat2x2 &Other) const;
    Vec2 operator*(const Vec2 &Vec) const;
    Mat2x2 &operator*=(real32 Scalar);
    Mat2x2 &operator-=(const Mat2x2 &Other);
    Mat2x2 &operator+=(const Mat2x2 &Other);
    Mat2x2 &operator*=(const Mat2x2 &Other);
};

inline real32 *Mat2x2::operator[](i32 Index)
{
    // TODO (ismail): add here an assert?

    real32 *Result = (real32*)(&Matrix[Index]);

    return Result;
}

inline Mat2x2 Mat2x2::operator-() const
{
    Mat2x2 Result = {
        -Matrix[0][0], -Matrix[0][1],
        -Matrix[1][0], -Matrix[1][1]
    };

    return Result;
}

inline Mat2x2 Mat2x2::operator-(const Mat2x2 &Other) const
{
    Mat2x2 Result = {
        Matrix[0][0] - Other.Matrix[0][0], Matrix[0][1] - Other.Matrix[0][1],
        Matrix[1][0] - Other.Matrix[1][0], Matrix[1][1] - Other.Matrix[1][1]
    };

    return Result;
}

inline Mat2x2 Mat2x2::operator+(const Mat2x2 &Other) const
{
    Mat2x2 Result = {
        Matrix[0][0] + Other.Matrix[0][0], Matrix[0][1] + Other.Matrix[0][1],
        Matrix[1][0] + Other.Matrix[1][0], Matrix[1][1] + Other.Matrix[1][1]
    };

    return Result;
}

inline Mat2x2 Mat2x2::operator*(real32 Scalar) const
{
    Mat2x2 Result = {
        Matrix[0][0] * Scalar, Matrix[0][1] * Scalar,
        Matrix[1][0] * Scalar, Matrix[1][1] * Scalar,
    };

    return Result;
}

inline Mat2x2 Mat2x2::operator*(const Mat2x2 &Other) const
{
    Mat2x2 Result = {
        Matrix[0][0] * Other.Matrix[0][0] + Matrix[0][1] * Other.Matrix[1][0], Matrix[0][0] * Other.Matrix[0][1] + Matrix[0][1] * Other.Matrix[1][1],
        Matrix[1][0] * Other.Matrix[0][0] + Matrix[1][1] * Other.Matrix[1][0], Matrix[1][0] * Other.Matrix[0][1] + Matrix[1][1] * Other.Matrix[1][1]
    };

    return Result;
}

inline Vec2 Mat2x2::operator*(const Vec2 &Vec) const
{
    Vec2 Result = {
        Matrix[0][0] * Vec.x + Matrix[0][1] * Vec.y,
        Matrix[1][0] * Vec.x + Matrix[1][1] * Vec.y,
    };

    return Result;
}

inline Mat2x2 &Mat2x2::operator*=(real32 Scalar)
{
    Matrix[0][0] *= Scalar; Matrix[0][1] *= Scalar;
    Matrix[1][0] *= Scalar; Matrix[1][1] *= Scalar;

    return *this;
}

inline Mat2x2 &Mat2x2::operator-=(const Mat2x2 &Other)
{
    Matrix[0][0] -= Other.Matrix[0][0]; Matrix[0][1] -= Other.Matrix[0][1];
    Matrix[1][0] -= Other.Matrix[1][0]; Matrix[1][1] -= Other.Matrix[1][1]; 

    return *this;
}

inline Mat2x2 &Mat2x2::operator+=(const Mat2x2 &Other)
{
    Matrix[0][0] += Other.Matrix[0][0]; Matrix[0][1] += Other.Matrix[0][1];
    Matrix[1][0] += Other.Matrix[1][0]; Matrix[1][1] += Other.Matrix[1][1]; 

    return *this;
}

inline Mat2x2 &Mat2x2::operator*=(const Mat2x2 &Other)
{
    real32 x, y;

    x = Matrix[0][0];
    y = Matrix[0][1];

    Matrix[0][0] = x * Other.Matrix[0][0] + y * Other.Matrix[1][0]; Matrix[0][1] = x * Other.Matrix[0][1] + y * Other.Matrix[1][1];

    x = Matrix[1][0];
    y = Matrix[1][1];

    Matrix[1][0] = x * Other.Matrix[0][0] + y * Other.Matrix[1][0]; Matrix[1][0] = x * Other.Matrix[0][1] + y * Other.Matrix[1][1];


    return *this;
}

struct Mat3x3 {
    real32 Matrix[3][3];

    real32 *operator[](i32 Index);
    Mat3x3 operator-() const;
    Mat3x3 operator-(const Mat3x3 &Other) const;
    Mat3x3 operator+(const Mat3x3 &Other) const;
    Mat3x3 operator*(real32 Scalar) const;
    Mat3x3 operator*(const Mat3x3 &Other) const;
    Vec3 operator*(const Vec3 &Vec) const;
    Mat3x3 &operator*=(real32 Scalar);
    Mat3x3 &operator-=(const Mat3x3 &Other);
    Mat3x3 &operator+=(const Mat3x3 &Other);
    Mat3x3 &operator*=(const Mat3x3 &Other);

    void Transpose();
};

inline real32 *Mat3x3::operator[](i32 Index)
{
    // TODO (ismail): add here an assert?
    real32 *Result = (real32*)(&Matrix[Index]);

    return Result;
}

inline Mat3x3 Mat3x3::operator-() const
{
    Mat3x3 Result = {
        -Matrix[0][0], -Matrix[0][1], -Matrix[0][2],
        -Matrix[1][0], -Matrix[1][1], -Matrix[1][2],
        -Matrix[2][0], -Matrix[2][1], -Matrix[2][2],
    };

    return Result;
}

inline Mat3x3 Mat3x3::operator-(const Mat3x3 &Other) const
{
    Mat3x3 Result = {
        Matrix[0][0] - Other.Matrix[0][0], Matrix[0][1] - Other.Matrix[0][1], Matrix[0][2] - Other.Matrix[0][2],
        Matrix[1][0] - Other.Matrix[1][0], Matrix[1][1] - Other.Matrix[1][1], Matrix[1][2] - Other.Matrix[1][2],
        Matrix[2][0] - Other.Matrix[2][0], Matrix[2][1] - Other.Matrix[2][1], Matrix[2][2] - Other.Matrix[2][2]
    };

    return Result;
}

inline Mat3x3 Mat3x3::operator+(const Mat3x3 &Other) const
{
    Mat3x3 Result = {
        Matrix[0][0] + Other.Matrix[0][0], Matrix[0][1] + Other.Matrix[0][1], Matrix[0][2] + Other.Matrix[0][2],
        Matrix[1][0] + Other.Matrix[1][0], Matrix[1][1] + Other.Matrix[1][1], Matrix[1][2] + Other.Matrix[1][2],
        Matrix[2][0] + Other.Matrix[2][0], Matrix[2][1] + Other.Matrix[2][1], Matrix[2][2] + Other.Matrix[2][2]
    };

    return Result;
}

inline Mat3x3 Mat3x3::operator*(real32 Scalar) const
{
    Mat3x3 Result = {
        Matrix[0][0] * Scalar, Matrix[0][1] * Scalar, Matrix[0][2] * Scalar,
        Matrix[1][0] * Scalar, Matrix[1][1] * Scalar, Matrix[1][2] * Scalar,
        Matrix[2][0] * Scalar, Matrix[2][1] * Scalar, Matrix[2][2] * Scalar
    };

    return Result;
}

inline Mat3x3 Mat3x3::operator*(const Mat3x3 &Other) const
{
    // TODO(ismail): check can i optimise this?

    Mat3x3 Result = {
        Matrix[0][0] * Other.Matrix[0][0] + Matrix[0][1] * Other.Matrix[1][0] + Matrix[0][2] * Other.Matrix[2][0], // i = 1 j = 1
        Matrix[0][0] * Other.Matrix[0][1] + Matrix[0][1] * Other.Matrix[1][1] + Matrix[0][2] * Other.Matrix[2][1], // i = 1 j = 2
        Matrix[0][0] * Other.Matrix[0][2] + Matrix[0][1] * Other.Matrix[1][2] + Matrix[0][2] * Other.Matrix[2][2], // i = 1 j = 3

        Matrix[1][0] * Other.Matrix[0][0] + Matrix[1][1] * Other.Matrix[1][0] + Matrix[1][2] * Other.Matrix[2][0], // i = 2 j = 1
        Matrix[1][0] * Other.Matrix[0][1] + Matrix[1][1] * Other.Matrix[1][1] + Matrix[1][2] * Other.Matrix[2][1], // i = 2 j = 2
        Matrix[1][0] * Other.Matrix[0][2] + Matrix[1][1] * Other.Matrix[1][2] + Matrix[1][2] * Other.Matrix[2][2], // i = 2 j = 3

        Matrix[2][0] * Other.Matrix[0][0] + Matrix[2][1] * Other.Matrix[1][0] + Matrix[2][2] * Other.Matrix[2][0], // i = 3 j = 1
        Matrix[2][0] * Other.Matrix[0][1] + Matrix[2][1] * Other.Matrix[1][1] + Matrix[2][2] * Other.Matrix[2][1], // i = 3 j = 2
        Matrix[2][0] * Other.Matrix[0][2] + Matrix[2][1] * Other.Matrix[1][2] + Matrix[2][2] * Other.Matrix[2][2], // i = 3 j = 3
    };

    return Result;
}

inline Vec3 Mat3x3::operator*(const Vec3 &Vec) const
{
    Vec3 Result = {
        Matrix[0][0] * Vec.x + Matrix[0][1] * Vec.y + Matrix[0][2] * Vec.z,
        Matrix[1][0] * Vec.x + Matrix[1][1] * Vec.y + Matrix[1][2] * Vec.z,
        Matrix[2][0] * Vec.x + Matrix[2][1] * Vec.y + Matrix[2][2] * Vec.z,
    };

    return Result;
}

inline Mat3x3 &Mat3x3::operator*=(real32 Scalar)
{
    Matrix[0][0] *= Scalar; Matrix[0][1] *= Scalar; Matrix[0][2] *= Scalar;
    Matrix[1][0] *= Scalar; Matrix[1][1] *= Scalar; Matrix[1][2] *= Scalar;
    Matrix[2][0] *= Scalar; Matrix[2][1] *= Scalar; Matrix[2][2] *= Scalar;

    return *this;
}

inline Mat3x3 &Mat3x3::operator-=(const Mat3x3 &Other)
{
    Matrix[0][0] -= Other.Matrix[0][0]; Matrix[0][1] -= Other.Matrix[0][1]; Matrix[0][2] -= Other.Matrix[0][2];
    Matrix[1][0] -= Other.Matrix[1][0]; Matrix[1][1] -= Other.Matrix[1][1]; Matrix[1][2] -= Other.Matrix[1][2];
    Matrix[2][0] -= Other.Matrix[2][0]; Matrix[2][1] -= Other.Matrix[2][1]; Matrix[2][2] -= Other.Matrix[2][2];

    return *this;
}

inline Mat3x3 &Mat3x3::operator+=(const Mat3x3 &Other)
{
    Matrix[0][0] += Other.Matrix[0][0]; Matrix[0][1] += Other.Matrix[0][1]; Matrix[0][2] += Other.Matrix[0][2];
    Matrix[1][0] += Other.Matrix[1][0]; Matrix[1][1] += Other.Matrix[1][1]; Matrix[1][2] += Other.Matrix[1][2];
    Matrix[2][0] += Other.Matrix[2][0]; Matrix[2][1] += Other.Matrix[2][1]; Matrix[2][2] += Other.Matrix[2][2];

    return *this;
}

inline Mat3x3 &Mat3x3::operator*=(const Mat3x3 &Other)
{
    real32 x, y, z;

    x = Matrix[0][0];
    y = Matrix[0][1];
    z = Matrix[0][2];

    Matrix[0][0] = x * Other.Matrix[0][0] + y * Other.Matrix[1][0] + z * Other.Matrix[2][0]; // i = 1 j = 1
    Matrix[0][1] = x * Other.Matrix[0][1] + y * Other.Matrix[1][1] + z * Other.Matrix[2][1], // i = 1 j = 2
    Matrix[0][2] = x * Other.Matrix[0][2] + y * Other.Matrix[1][2] + z * Other.Matrix[2][2], // i = 1 j = 3

    x = Matrix[1][0];
    y = Matrix[1][1];
    z = Matrix[1][2];

    Matrix[1][0] = x * Other.Matrix[0][0] + y * Other.Matrix[1][0] + z * Other.Matrix[2][0]; // i = 2 j = 1
    Matrix[1][1] = x * Other.Matrix[0][1] + y * Other.Matrix[1][1] + z * Other.Matrix[2][1], // i = 2 j = 2
    Matrix[1][2] = x * Other.Matrix[0][2] + y * Other.Matrix[1][2] + z * Other.Matrix[2][2], // i = 2 j = 3

    x = Matrix[2][0];
    y = Matrix[2][1];
    z = Matrix[2][2];

    Matrix[2][0] = x * Other.Matrix[0][0] + y * Other.Matrix[1][0] + z * Other.Matrix[2][0]; // i = 3 j = 1
    Matrix[2][1] = x * Other.Matrix[0][1] + y * Other.Matrix[1][1] + z * Other.Matrix[2][1]; // i = 3 j = 2
    Matrix[2][2] = x * Other.Matrix[0][2] + y * Other.Matrix[1][2] + z * Other.Matrix[2][2]; // i = 3 j = 3 

    return *this;
}

inline void Mat3x3::Transpose()
{
    // | 1  1  1 |    | 1  2  3 |
    // | 2  2  2 | to | 1  2  3 |
    // | 3  3  3 |    | 1  2  3 |

    real32 tmp;

    tmp = Matrix[0][1];
    Matrix[0][1] = Matrix[1][0];
    Matrix[1][0] = tmp;

    // | 1  2  1 |
    // | 1  2  2 |
    // | 3  3  3 |

    tmp = Matrix[0][2];
    Matrix[0][2] = Matrix[2][0];
    Matrix[2][0] = tmp;

    // | 1  2  3 |
    // | 1  2  2 |
    // | 1  3  3 |

    tmp = Matrix[1][2];
    Matrix[1][2] = Matrix[2][1];
    Matrix[2][1] = tmp;

    // | 1  2  3 |
    // | 1  2  3 |
    // | 1  2  3 |
}

struct Mat4x4 {
    real32 Matrix[4][4];

    const real32* operator[](i32 Index) const;
    real32 *operator[](i32 Index);
    Mat4x4 operator-() const;
    Mat4x4 operator-(const Mat4x4 &Other) const;
    Mat4x4 operator+(const Mat4x4 &Other) const;
    Mat4x4 operator*(real32 Scalar) const;
    Mat4x4 operator*(const Mat4x4 &Other) const;
    Vec4 operator*(const Vec4 &Vec) const;
    Mat4x4 &operator*=(real32 Scalar);
    Mat4x4 &operator-=(const Mat4x4 &Other);
    Mat4x4 &operator+=(const Mat4x4 &Other);
    Mat4x4 &operator*=(const Mat4x4 &Other);
};

real32 *Mat4x4::operator[](i32 Index)
{
    // TODO (ismail): add here an assert?
    real32 *Result = (real32*)(&Matrix[Index]);

    return Result;
}

const real32* Mat4x4::operator[](i32 Index) const
{    
    // TODO (ismail): add here an assert?
    real32 *Result = (real32*)(&Matrix[Index]);

    return Result;
}

Mat4x4 Mat4x4::operator-() const
{
    Mat4x4 Result = {
        -Matrix[0][0], -Matrix[0][1], -Matrix[0][2], -Matrix[0][3],
        -Matrix[1][0], -Matrix[1][1], -Matrix[1][2], -Matrix[1][3],
        -Matrix[2][0], -Matrix[2][1], -Matrix[2][2], -Matrix[2][3],
        -Matrix[3][0], -Matrix[3][1], -Matrix[3][2], -Matrix[3][3]
    };

    return Result;
}

Mat4x4 Mat4x4::operator-(const Mat4x4 &Other) const
{
    Mat4x4 Result = {
        Matrix[0][0] - Other.Matrix[0][0], Matrix[0][1] - Other.Matrix[0][1], Matrix[0][2] - Other.Matrix[0][2], Matrix[0][3] - Other.Matrix[0][3],
        Matrix[1][0] - Other.Matrix[1][0], Matrix[1][1] - Other.Matrix[1][1], Matrix[1][2] - Other.Matrix[1][2], Matrix[1][3] - Other.Matrix[1][3],
        Matrix[2][0] - Other.Matrix[2][0], Matrix[2][1] - Other.Matrix[2][1], Matrix[2][2] - Other.Matrix[2][2], Matrix[2][3] - Other.Matrix[2][3],
        Matrix[3][0] - Other.Matrix[3][0], Matrix[3][1] - Other.Matrix[3][1], Matrix[3][2] - Other.Matrix[3][2], Matrix[3][3] - Other.Matrix[3][3]
    };

    return Result;
}

Mat4x4 Mat4x4::operator+(const Mat4x4 &Other) const
{
    Mat4x4 Result = {
        Matrix[0][0] + Other.Matrix[0][0], Matrix[0][1] + Other.Matrix[0][1], Matrix[0][2] + Other.Matrix[0][2], Matrix[0][3] + Other.Matrix[0][3],
        Matrix[1][0] + Other.Matrix[1][0], Matrix[1][1] + Other.Matrix[1][1], Matrix[1][2] + Other.Matrix[1][2], Matrix[1][3] + Other.Matrix[1][3],
        Matrix[2][0] + Other.Matrix[2][0], Matrix[2][1] + Other.Matrix[2][1], Matrix[2][2] + Other.Matrix[2][2], Matrix[2][3] + Other.Matrix[2][3],
        Matrix[3][0] + Other.Matrix[3][0], Matrix[3][1] + Other.Matrix[3][1], Matrix[3][2] + Other.Matrix[3][2], Matrix[3][3] + Other.Matrix[3][3]
    };

    return Result;
}

Mat4x4 Mat4x4::operator*(real32 Scalar) const
{
    Mat4x4 Result = {
        Matrix[0][0] * Scalar, Matrix[0][1] * Scalar, Matrix[0][2] * Scalar, Matrix[0][3] * Scalar,
        Matrix[1][0] * Scalar, Matrix[1][1] * Scalar, Matrix[1][2] * Scalar, Matrix[1][3] * Scalar,
        Matrix[2][0] * Scalar, Matrix[2][1] * Scalar, Matrix[2][2] * Scalar, Matrix[2][3] * Scalar,
        Matrix[3][0] * Scalar, Matrix[3][1] * Scalar, Matrix[3][2] * Scalar, Matrix[3][3] * Scalar
    };

    return Result;
}

Mat4x4 Mat4x4::operator*(const Mat4x4 &Other) const
{
    // TODO(ismail): check can i optimise this?

    Mat4x4 Result = {
        Matrix[0][0] * Other.Matrix[0][0] + Matrix[0][1] * Other.Matrix[1][0] + Matrix[0][2] * Other.Matrix[2][0] + Matrix[0][3] * Other.Matrix[3][0], // i = 1 j = 1
        Matrix[0][0] * Other.Matrix[0][1] + Matrix[0][1] * Other.Matrix[1][1] + Matrix[0][2] * Other.Matrix[2][1] + Matrix[0][3] * Other.Matrix[3][1], // i = 1 j = 2
        Matrix[0][0] * Other.Matrix[0][2] + Matrix[0][1] * Other.Matrix[1][2] + Matrix[0][2] * Other.Matrix[2][2] + Matrix[0][3] * Other.Matrix[3][2], // i = 1 j = 3
        Matrix[0][0] * Other.Matrix[0][3] + Matrix[0][1] * Other.Matrix[1][3] + Matrix[0][2] * Other.Matrix[2][3] + Matrix[0][3] * Other.Matrix[3][3], // i = 1 j = 4

        Matrix[1][0] * Other.Matrix[0][0] + Matrix[1][1] * Other.Matrix[1][0] + Matrix[1][2] * Other.Matrix[2][0] + Matrix[1][3] * Other.Matrix[3][0], // i = 2 j = 1
        Matrix[1][0] * Other.Matrix[0][1] + Matrix[1][1] * Other.Matrix[1][1] + Matrix[1][2] * Other.Matrix[2][1] + Matrix[1][3] * Other.Matrix[3][1], // i = 2 j = 2
        Matrix[1][0] * Other.Matrix[0][2] + Matrix[1][1] * Other.Matrix[1][2] + Matrix[1][2] * Other.Matrix[2][2] + Matrix[1][3] * Other.Matrix[3][2], // i = 2 j = 3
        Matrix[1][0] * Other.Matrix[0][3] + Matrix[1][1] * Other.Matrix[1][3] + Matrix[1][2] * Other.Matrix[2][3] + Matrix[1][3] * Other.Matrix[3][3], // i = 2 j = 4

        Matrix[2][0] * Other.Matrix[0][0] + Matrix[2][1] * Other.Matrix[1][0] + Matrix[2][2] * Other.Matrix[2][0] + Matrix[2][3] * Other.Matrix[3][0], // i = 3 j = 1
        Matrix[2][0] * Other.Matrix[0][1] + Matrix[2][1] * Other.Matrix[1][1] + Matrix[2][2] * Other.Matrix[2][1] + Matrix[2][3] * Other.Matrix[3][1], // i = 3 j = 2
        Matrix[2][0] * Other.Matrix[0][2] + Matrix[2][1] * Other.Matrix[1][2] + Matrix[2][2] * Other.Matrix[2][2] + Matrix[2][3] * Other.Matrix[3][2], // i = 3 j = 3
        Matrix[2][0] * Other.Matrix[0][3] + Matrix[2][1] * Other.Matrix[1][3] + Matrix[2][2] * Other.Matrix[2][3] + Matrix[2][3] * Other.Matrix[3][3], // i = 3 j = 4

        Matrix[3][0] * Other.Matrix[0][0] + Matrix[3][1] * Other.Matrix[1][0] + Matrix[3][2] * Other.Matrix[2][0] + Matrix[3][3] * Other.Matrix[3][0], // i = 4 j = 1
        Matrix[3][0] * Other.Matrix[0][1] + Matrix[3][1] * Other.Matrix[1][1] + Matrix[3][2] * Other.Matrix[2][1] + Matrix[3][3] * Other.Matrix[3][1], // i = 4 j = 2
        Matrix[3][0] * Other.Matrix[0][2] + Matrix[3][1] * Other.Matrix[1][2] + Matrix[3][2] * Other.Matrix[2][2] + Matrix[3][3] * Other.Matrix[3][2], // i = 4 j = 3
        Matrix[3][0] * Other.Matrix[0][3] + Matrix[3][1] * Other.Matrix[1][3] + Matrix[3][2] * Other.Matrix[2][3] + Matrix[3][3] * Other.Matrix[3][3], // i = 4 j = 4
    };

    return Result;
}

Vec4 Mat4x4::operator*(const Vec4 &Vec) const
{
    Vec4 Result = {
        Matrix[0][0] * Vec.x + Matrix[0][1] * Vec.y + Matrix[0][2] * Vec.z + Matrix[0][3] * Vec.w,
        Matrix[1][0] * Vec.x + Matrix[1][1] * Vec.y + Matrix[1][2] * Vec.z + Matrix[1][3] * Vec.w,
        Matrix[2][0] * Vec.x + Matrix[2][1] * Vec.y + Matrix[2][2] * Vec.z + Matrix[2][3] * Vec.w,
        Matrix[3][0] * Vec.x + Matrix[3][1] * Vec.y + Matrix[3][2] * Vec.z + Matrix[3][3] * Vec.w
    };

    return Result;
}

Mat4x4 &Mat4x4::operator*=(real32 Scalar)
{
    Matrix[0][0] *= Scalar; Matrix[0][1] *= Scalar; Matrix[0][2] *= Scalar; Matrix[0][3] *= Scalar;
    Matrix[1][0] *= Scalar; Matrix[1][1] *= Scalar; Matrix[1][2] *= Scalar; Matrix[1][3] *= Scalar;
    Matrix[2][0] *= Scalar; Matrix[2][1] *= Scalar; Matrix[2][2] *= Scalar; Matrix[2][3] *= Scalar;
    Matrix[3][0] *= Scalar; Matrix[3][1] *= Scalar; Matrix[3][2] *= Scalar; Matrix[3][3] *= Scalar;

    return *this;
}

Mat4x4 &Mat4x4::operator-=(const Mat4x4 &Other)
{
    Matrix[0][0] -= Other.Matrix[0][0]; Matrix[0][1] -= Other.Matrix[0][1]; Matrix[0][2] -= Other.Matrix[0][2]; Matrix[0][3] -= Other.Matrix[0][3];
    Matrix[1][0] -= Other.Matrix[1][0]; Matrix[1][1] -= Other.Matrix[1][1]; Matrix[1][2] -= Other.Matrix[1][2]; Matrix[1][3] -= Other.Matrix[1][3];
    Matrix[2][0] -= Other.Matrix[2][0]; Matrix[2][1] -= Other.Matrix[2][1]; Matrix[2][2] -= Other.Matrix[2][2]; Matrix[2][3] -= Other.Matrix[2][3];
    Matrix[3][0] -= Other.Matrix[3][0]; Matrix[3][1] -= Other.Matrix[3][1]; Matrix[3][2] -= Other.Matrix[3][2]; Matrix[3][3] -= Other.Matrix[3][3];

    return *this;
}

Mat4x4 &Mat4x4::operator+=(const Mat4x4 &Other)
{
    Matrix[0][0] += Other.Matrix[0][0]; Matrix[0][1] += Other.Matrix[0][1]; Matrix[0][2] += Other.Matrix[0][2]; Matrix[0][3] += Other.Matrix[0][3];
    Matrix[1][0] += Other.Matrix[1][0]; Matrix[1][1] += Other.Matrix[1][1]; Matrix[1][2] += Other.Matrix[1][2]; Matrix[1][3] += Other.Matrix[1][3];
    Matrix[2][0] += Other.Matrix[2][0]; Matrix[2][1] += Other.Matrix[2][1]; Matrix[2][2] += Other.Matrix[2][2]; Matrix[2][3] += Other.Matrix[2][3];
    Matrix[3][0] += Other.Matrix[3][0]; Matrix[3][1] += Other.Matrix[3][1]; Matrix[3][2] += Other.Matrix[3][2]; Matrix[3][3] += Other.Matrix[3][3];

    return *this;
}

Mat4x4 &Mat4x4::operator*=(const Mat4x4 &Other)
{
    real32 x, y, z, w;

    x = Matrix[0][0];
    y = Matrix[0][1];
    z = Matrix[0][2];
    w = Matrix[0][3];

    Matrix[0][0] = x * Other.Matrix[0][0] + y * Other.Matrix[1][0] + z * Other.Matrix[2][0] + w * Other.Matrix[3][0]; // i = 1 j = 1
    Matrix[0][1] = x * Other.Matrix[0][1] + y * Other.Matrix[1][1] + z * Other.Matrix[2][1] + w * Other.Matrix[3][1]; // i = 1 j = 2
    Matrix[0][2] = x * Other.Matrix[0][2] + y * Other.Matrix[1][2] + z * Other.Matrix[2][2] + w * Other.Matrix[3][2]; // i = 1 j = 3
    Matrix[0][3] = x * Other.Matrix[0][3] + y * Other.Matrix[1][3] + z * Other.Matrix[2][3] + w * Other.Matrix[3][3]; // i = 1 j = 4

    x = Matrix[1][0];
    y = Matrix[1][1];
    z = Matrix[1][2];
    w = Matrix[1][3];

    Matrix[1][0] = x * Other.Matrix[0][0] + y * Other.Matrix[1][0] + z * Other.Matrix[2][0] + w * Other.Matrix[3][0]; // i = 2 j = 1
    Matrix[1][1] = x * Other.Matrix[0][1] + y * Other.Matrix[1][1] + z * Other.Matrix[2][1] + w * Other.Matrix[3][1]; // i = 2 j = 2
    Matrix[1][2] = x * Other.Matrix[0][2] + y * Other.Matrix[1][2] + z * Other.Matrix[2][2] + w * Other.Matrix[3][2]; // i = 2 j = 3
    Matrix[1][3] = x * Other.Matrix[0][3] + y * Other.Matrix[1][3] + z * Other.Matrix[2][3] + w * Other.Matrix[3][3]; // i = 2 j = 4

    x = Matrix[2][0];
    y = Matrix[2][1];
    z = Matrix[2][2];
    w = Matrix[2][3];

    Matrix[2][0] = x * Other.Matrix[0][0] + y * Other.Matrix[1][0] + z * Other.Matrix[2][0] + w * Other.Matrix[3][0]; // i = 3 j = 1
    Matrix[2][1] = x * Other.Matrix[0][1] + y * Other.Matrix[1][1] + z * Other.Matrix[2][1] + w * Other.Matrix[3][1]; // i = 3 j = 2
    Matrix[2][2] = x * Other.Matrix[0][2] + y * Other.Matrix[1][2] + z * Other.Matrix[2][2] + w * Other.Matrix[3][2]; // i = 3 j = 3
    Matrix[2][3] = x * Other.Matrix[0][3] + y * Other.Matrix[1][3] + z * Other.Matrix[2][3] + w * Other.Matrix[3][3]; // i = 3 j = 4

    x = Matrix[3][0];
    y = Matrix[3][1];
    z = Matrix[3][2];
    w = Matrix[3][3];

    Matrix[3][0] = x * Other.Matrix[0][0] + y * Other.Matrix[1][0] + z * Other.Matrix[2][0] + w * Other.Matrix[3][0]; // i = 4 j = 1
    Matrix[3][1] = x * Other.Matrix[0][1] + y * Other.Matrix[1][1] + z * Other.Matrix[2][1] + w * Other.Matrix[3][1]; // i = 4 j = 2
    Matrix[3][2] = x * Other.Matrix[0][2] + y * Other.Matrix[1][2] + z * Other.Matrix[2][2] + w * Other.Matrix[3][2]; // i = 4 j = 3
    Matrix[3][3] = x * Other.Matrix[0][3] + y * Other.Matrix[1][3] + z * Other.Matrix[2][3] + w * Other.Matrix[3][3]; // i = 4 j = 4

    return *this;
}

// TODO (ismail): extern this
const Mat4x4 Identity4x4 = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

const Mat3x3 Identity3x3 = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};

#endif