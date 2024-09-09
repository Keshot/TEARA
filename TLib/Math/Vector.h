#ifndef _TEARA_MATH_VECTOR_H_
#define _TEARA_MATH_VECTOR_H_

#include "CoreMath.h"

#define VECTOR_EPSILON 0.001f

// TODO(ismail): convert all to SIMD
// TODO(ismail): remove all const things?
// TODO(ismail): reorder code first definition then implementation?
// TODO(ismail): use here my int types

// Vector2D

struct Vec2 {
    union {
        struct {
            real32 x, y;
        };
        real32 ValueHolder[2];   
    };

    inline real32& operator[](i32 indx) {
        // TODO(ismail): may be some assert for check overflow?

        real32& Result = ValueHolder[indx];

        return Result;
    }

    inline Vec2 operator-() {
        Vec2 Result = { -x, -y };

        return Result;
    }

    inline Vec2 operator*(real32 Scalar) {
        Vec2 Result = { x * Scalar, y * Scalar };

        return Result;
    }

    inline Vec2& operator*=(real32 Scalar) {
        x *= Scalar;
        y *= Scalar;

        return *this;
    }

    inline Vec2 operator+(const Vec2 &Other) {
        Vec2 Result = { x + Other.x, y + Other.y };

        return Result;
    }

    inline Vec2& operator+=(const Vec2 &Other) {
        x += Other.x;
        y += Other.y;

        return *this;
    }

    inline Vec2 operator-(const Vec2 &Other) const {
        Vec2 Result = { x - Other.x, y - Other.y };

        return Result;
    }

    inline Vec2& operator-=(const Vec2 &Other) {
        x -= Other.x;
        y -= Other.y;

        return *this;
    }

    inline real32 Dot(const Vec2 &Other) {
        real32 Result = x * Other.x + y * Other.y;

        return Result;
    }

    inline real32 Length() {
        return Sqrt(x * x + y * y);
    }

    inline void Normalize() {
        real32 InverseLength = 1 / ( Sqrt( x * x + y * y ) );

        x *= InverseLength;
        y *= InverseLength;
    }
};

inline real32 Distance(const Vec2 &a, const Vec2 &b) {
    real32 Result = (a - b).Length();

    return Result;
}

// Vector3D

struct Vec3 {
    union {
        struct {
            real32 x, y, z;
        };
        real32 ValueHolder[3];   
    };

    inline real32& operator[](int indx){
        // TODO(ismail): may be some assert for check overflow?

        real32& Result = ValueHolder[indx];

        return Result;
    }

    inline Vec3 operator-() {
        Vec3 Result = { -x, -y, -z };

        return Result;
    }

    inline Vec3 operator*(real32 Scalar) {
        Vec3 Result = { x * Scalar, y * Scalar, z * Scalar };

        return Result;
    }

    inline Vec3& operator*=(real32 Scalar) {
        x *= Scalar;
        y *= Scalar;
        z *= Scalar;

        return *this;
    }

    inline Vec3 operator+(Vec3 &Other) {
        Vec3 Result = { x + Other.x, y + Other.y, z + Other.z };

        return Result;
    }

    inline Vec3& operator+=(Vec3 &Other) {
        x += Other.x;
        y += Other.y;
        z += Other.z;

        return *this;
    }

    inline Vec3 operator-(const Vec3 &Other) const {
        Vec3 Result = { x - Other.x, y - Other.y, z - Other.z };

        return Result;
    }

    inline Vec3& operator-=(const Vec3 &Other) {
        x -= Other.x;
        y -= Other.y;
        z -= Other.z;

        return *this;
    }

    inline real32 Dot(const Vec3 &Other) {
        real32 Result = x * Other.x + y * Other.y + z * Other.z;

        return Result;
    }

    inline Vec3 Cross(const Vec3 &Other) {
        // | THISx1 |   | OTHERx2 |   | y1 * z2 - z1 * y2 |
        // | THISy1 | x | OTHERy2 | = | z1 * x2 - x1 * z2 |
        // | THISz1 |   | OTHERz2 |   | x1 * y2 - y1 * x1 |
        Vec3 Result = { 
                y * Other.z - z * Other.y, 
                z * Other.x - x * Other.z,
                x * Other.y - y * Other.x
            };

        return Result;
    }

    inline real32 Length() {
        return Sqrt(x * x + y * y + z * z);
    }

    inline void Normalize() {
        // TODO (ismail): normalize this with epsilon
        if (x == 0.0f && y == 0.0f && z == 0.0f) {
            return;
        }

        real32 InverseLength = 1 / ( Sqrt( x * x + y * y + z * z ) );

        x *= InverseLength;
        y *= InverseLength;
        z *= InverseLength;
    }
};

inline real32 Distance(const Vec3 &a, const Vec3 &b) {
    real32 Result = (a - b).Length();

    return Result;
}

inline Vec3 Normalize(const Vec3 &Other) {
    Vec3 Result = Other;

    Result.Normalize();

    return Result;
}

inline real32 DotProduct(const Vec3 &A, const Vec3 &B) {
    real32 Result = A.x * B.x + A.y * B.y + A.z * B.z;
    return Result;
}

// Vector4D

struct Vec4 {
    union {
        struct {
            real32 x, y, z, w;
        };
        real32 ValueHolder[4];   
    };

    inline real32& operator[](int indx){
        // TODO(ismail): may be some assert for check overflow?

        real32& Result = ValueHolder[indx];

        return Result;
    }

    inline Vec4 operator-() {
        Vec4 Result = { -x, -y, -z, -w };

        return Result;
    }

    inline Vec4 operator*(real32 Scalar) {
        // NOTE(ismail): should we multiply w by the Scalar?
        Vec4 Result = { x * Scalar, y * Scalar, z * Scalar, w * Scalar };

        return Result;
    }

    inline Vec4& operator*=(real32 Scalar) {
        x *= Scalar;
        y *= Scalar;
        z *= Scalar;
        w *= Scalar; // ?

        return *this;
    }

    inline Vec4 operator+(Vec4 &Other) {
        Vec4 Result = { x + Other.x, y + Other.y, z + Other.z, w + Other.w }; // ?

        return Result;
    }

    inline Vec4& operator+=(Vec4 &Other) {
        x += Other.x;
        y += Other.y;
        z += Other.z;
        w += Other.w; // ?

        return *this;
    }

    inline Vec4 operator-(const Vec4 &Other) const {
        Vec4 Result = { x - Other.x, y - Other.y, z - Other.z, w - Other.w };

        return Result;
    }

    inline Vec4& operator-=(const Vec4 &Other) {
        x -= Other.x;
        y -= Other.y;
        z -= Other.z;
        w -= Other.w;

        return *this;
    }

    inline real32 Dot(const Vec4 &Other) {
        real32 Result = x * Other.x + y * Other.y + z * Other.z + w * Other.w;

        return Result;
    }

    inline real32 Length() {
        return Sqrt(x * x + y * y + z * z + w * w);
    }

    inline void Normalize() {
        real32 InverseLength = 1 / ( Sqrt( x * x + y * y + z * z + w * w ) );

        x *= InverseLength;
        y *= InverseLength;
        z *= InverseLength;
        w *= InverseLength;
    }

    inline real32 Distance(const Vec4 &a, const Vec4 &b) {
        real32 Result = (a - b).Length();

        return Result;
    }
};

#endif