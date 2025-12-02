#ifndef _TEARA_MATH_VECTOR_H_
#define _TEARA_MATH_VECTOR_H_

#include "Math.h"

// TODO(ismail): convert all to SIMD
// TODO(ismail): add tests

#define VEC_EPS (1e-12f)

struct vec2 {
    real32& operator[](i32 indx);
    vec2 operator-() const;
    vec2 operator*(real32 Scalar) const;
    vec2& operator*=(real32 Scalar);
    vec2 operator+(const vec2 &Other) const;
    vec2& operator+=(const vec2 &Other);
    vec2 operator-(const vec2 &Other) const;
    vec2& operator-=(const vec2 &Other);
    real32 Dot(const vec2 &Other) const;
    real32 Length() const;
    void Normalize();

    static real32 Distance(const vec2 &a, const vec2 &b);
    static vec2 Lerp(const vec2 &a, const vec2 &b, real32 t);

    union {
        struct {
            real32 x, y;
        };
        real32 vec[2];   
    };
};

inline real32& vec2::operator[](i32 indx) 
{
    Assert(indx < (sizeof(vec) / sizeof(*vec)));

    real32& Result = vec[indx];

    return Result;
}

inline vec2 vec2::operator-() const 
{
    vec2 Result = { -x, -y };

    return Result;
}

inline vec2 vec2::operator*(real32 Scalar) const
{
    vec2 Result = { x * Scalar, y * Scalar };
    return Result;
}

inline vec2& vec2::operator*=(real32 Scalar) 
{
    x *= Scalar;
    y *= Scalar;

    return *this;
}

inline vec2 vec2::operator+(const vec2 &Other) const 
{
    vec2 Result = { x + Other.x, y + Other.y };

    return Result;
}

inline vec2& vec2::operator+=(const vec2 &Other) 
{
    x += Other.x;
    y += Other.y;

    return *this;
}

inline vec2 vec2::operator-(const vec2 &Other) const 
{
    vec2 Result = { x - Other.x, y - Other.y };

    return Result;
}

inline vec2& vec2::operator-=(const vec2 &Other) 
{
    x -= Other.x;
    y -= Other.y;

    return *this;
}

inline real32 vec2::Dot(const vec2 &Other) const
{
    real32 Result = x * Other.x + y * Other.y;

    return Result;
}

inline real32 vec2::Length() const
{
    real32 LenSqr = SQUARE(x) + SQUARE(y);

    return Sqrt(LenSqr);
}

inline void vec2::Normalize() 
{
    real32 Len = Length();

    if (Len < VEC_EPS) {
        x = y = 0.0f;

        return;
    }

    real32 InverseLength = 1.0f / ( Len );

    x *= InverseLength;
    y *= InverseLength;
}

inline real32 vec2::Distance(const vec2 &a, const vec2 &b) 
{
    real32 Result = (a - b).Length();

    return Result;
}

inline vec2 vec2::Lerp(const vec2 &a, const vec2 &b, real32 t)
{
    vec2 To = b - a;
    return To * t;
}

struct vec3 {
    real32& operator[](i32 indx);
    const real32& operator[](i32 indx) const;
    vec3 operator-() const;
    vec3 operator*(real32 Scalar) const;
    vec3& operator*=(real32 Scalar);
    vec3 operator+(const vec3 &Other) const;
    vec3& operator+=(const vec3 &Other);
    vec3 operator-(const vec3 &Other) const;
    vec3& operator-=(const vec3 &Other);
    real32 Dot(const vec3 &Other) const;
    vec3 Cross(const vec3 &Other) const;
    real32 Length() const;
    void Normalize();
    real32* Data();

    static real32 Distance(const vec3 &a, const vec3 &b);
    static vec3 Normalize(const vec3 &Other);
    static real32 DotProduct(const vec3 &A, const vec3 &B);
    static vec3 Lerp(const vec3 &a, const vec3 &b, real32 t);

    union {
        struct {
            real32 x, y, z;
        };
        real32 vec[3];
    };
};

inline real32& vec3::operator[](i32 indx)
{
    Assert(indx < (sizeof(vec) / sizeof(*vec)));

    real32& Result = vec[indx];

    return Result;
}

inline const real32& vec3::operator[](i32 indx) const
{
    Assert(indx < (sizeof(vec) / sizeof(*vec)));

    const real32& Result = vec[indx];

    return Result;
}

inline vec3 vec3::operator-() const
{
    vec3 Result = { -x, -y, -z };

    return Result;
}

inline vec3 vec3::operator*(real32 Scalar) const
{
    vec3 Result = { x * Scalar, y * Scalar, z * Scalar };

    return Result;
}

inline vec3& vec3::operator*=(real32 Scalar)
{
    x *= Scalar;
    y *= Scalar;
    z *= Scalar;

    return *this;
}

inline vec3 vec3::operator+(const vec3 &Other) const
{
    vec3 Result = { x + Other.x, y + Other.y, z + Other.z };

    return Result;
}

inline vec3& vec3::operator+=(const vec3 &Other)
{
    x += Other.x;
    y += Other.y;
    z += Other.z;

    return *this;
}

inline vec3 vec3::operator-(const vec3 &Other) const
{
    vec3 Result = { x - Other.x, y - Other.y, z - Other.z };

    return Result;
}

inline vec3& vec3::operator-=(const vec3 &Other)
{
    x -= Other.x;
    y -= Other.y;
    z -= Other.z;

    return *this;
}

inline real32 vec3::Dot(const vec3 &Other) const
{
    real32 Result = x * Other.x + y * Other.y + z * Other.z;

    return Result;
}

inline vec3 vec3::Cross(const vec3 &Other) const
{
    // | THISx1 |   | OTHERx2 |   | y1 * z2 - z1 * y2 |
    // | THISy1 | x | OTHERy2 | = | z1 * x2 - x1 * z2 |
    // | THISz1 |   | OTHERz2 |   | x1 * y2 - y1 * x1 |
    vec3 Result = {
        y * Other.z - z * Other.y,
        z * Other.x - x * Other.z,
        x * Other.y - y * Other.x
    };

    return Result;
}

inline real32 vec3::Length() const
{
    real32 LenSq = SQUARE(x) + SQUARE(y) + SQUARE(z);

    return Sqrt( LenSq );
}

inline void vec3::Normalize()
{
    real32 Len = Length();

    if (Len < VEC_EPS) {
        x = y = z = 0.0f;

        return;
    }

    real32 InverseLength = 1.0f / ( Len );

    x *= InverseLength;
    y *= InverseLength;
    z *= InverseLength;
}

inline real32* vec3::Data()
{
    return vec;
}

inline real32 vec3::Distance(const vec3 &a, const vec3 &b)
{
    real32 Result = (a - b).Length();

    return Result;
}

inline vec3 vec3::Normalize(const vec3 &Other)
{
    vec3 Result = Other;

    Result.Normalize();

    return Result;
}

inline real32 vec3::DotProduct(const vec3 &a, const vec3 &b)
{
    real32 Result = a.x * b.x + a.y * b.y + a.z * b.z;

    return Result;
}

inline vec3 vec3::Lerp(const vec3 &a, const vec3 &b, real32 t)
{
    vec3 To = b - a;
    return To * t;
}

struct vec4 {
    real32& operator[](i32 indx);
    const real32& operator[](i32 indx) const;
    vec4 operator-() const;
    vec4 operator*(real32 Scalar) const;
    vec4& operator*=(real32 Scalar);
    vec4 operator+(const vec4 &Other) const;
    vec4& operator+=(const vec4 &Other);
    vec4 operator-(const vec4 &Other) const;
    vec4& operator-=(const vec4 &Other);
    real32 Dot(const vec4 &Other) const;
    real32 Length() const;
    void Normalize();

    static real32 Distance(const vec4 &a, const vec4 &b);
    static vec4 Lerp(const vec4 &a, const vec4 &b, real32 t);

    union {
        struct {
            real32 x, y, z, w;
        };
        real32 vec[4];
    };
};

inline real32& vec4::operator[](i32 indx)
{
    Assert(indx < (sizeof(vec) / sizeof(*vec)));

    real32& Result = vec[indx];

    return Result;
}

inline const real32& vec4::operator[](i32 indx) const
{
    Assert(indx < (sizeof(vec) / sizeof(*vec)));

    const real32& Result = vec[indx];

    return Result;
}

inline vec4 vec4::operator-() const
{
    vec4 Result = { -x, -y, -z, -w };

    return Result;
}

inline vec4 vec4::operator*(real32 Scalar) const
{
    vec4 Result = { x * Scalar, y * Scalar, z * Scalar, w * Scalar };

    return Result;
}

inline vec4& vec4::operator*=(real32 Scalar)
{
    x *= Scalar;
    y *= Scalar;
    z *= Scalar;
    w *= Scalar;

    return *this;
}

inline vec4 vec4::operator+(const vec4 &Other) const
{
    vec4 Result = { x + Other.x, y + Other.y, z + Other.z, w + Other.w };

    return Result;
}

inline vec4& vec4::operator+=(const vec4 &Other)
{
    x += Other.x;
    y += Other.y;
    z += Other.z;
    w += Other.w;

    return *this;
}

inline vec4 vec4::operator-(const vec4 &Other) const
{
    vec4 Result = { x - Other.x, y - Other.y, z - Other.z, w - Other.w };

    return Result;
}

inline vec4& vec4::operator-=(const vec4 &Other)
{
    x -= Other.x;
    y -= Other.y;
    z -= Other.z;
    w -= Other.w;

    return *this;
}

inline real32 vec4::Dot(const vec4 &Other) const
{
    real32 Result = x * Other.x + y * Other.y + z * Other.z + w * Other.w;

    return Result;
}

inline real32 vec4::Length() const
{
    real32 LenSq = SQUARE(x) + SQUARE(y) + SQUARE(z) + SQUARE(w);
    
    return Sqrt(LenSq);
}

inline void vec4::Normalize()
{
    real32 Len = Length();

    if (Len < VEC_EPS) {
        x = y = z = w = 0.0f;

        return;
    }

    real32 InverseLength = 1.0f / Len;

    x *= InverseLength;
    y *= InverseLength;
    z *= InverseLength;
    w *= InverseLength;
}

inline real32 vec4::Distance(const vec4 &a, const vec4 &b)
{
    real32 Result = (a - b).Length();

    return Result;
}

inline vec4 vec4::Lerp(const vec4 &a, const vec4 &b, real32 t)
{
    vec4 To = b - a;
    return To * t;
}

enum { 
    vec2_size = (sizeof(vec2) / sizeof(*vec2::vec)),  
    vec3_size = (sizeof(vec3) / sizeof(*vec3::vec)),  
    vec4_size = (sizeof(vec4) / sizeof(*vec4::vec)) 
};

#endif