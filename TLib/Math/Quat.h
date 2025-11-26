#ifndef _TEARA_MATH_QUAT_H_
#define _TEARA_MATH_QUAT_H_

#include "Matrix.h"

struct quat {
    quat();
    quat(real32 w, real32 x, real32 y, real32 z);
    quat(real32* Rot);
    quat(real32 angle, const vec3& n);

    // NOTE(ismail): I expect that w "coordinate" will be fourth in Scalars
    quat& operator=(const real32* Scalars);
    quat operator-() const;
    real32 Length() const;
    quat operator*(const quat &b) const;
    vec3 operator*(const vec3 &b) const;
    void Mat3(mat3& Result) const;
    void Mat4(mat4& Result) const;
    void UprightToObject(mat4& Result) const;
    real32 Dot(const quat& b) const;

    static quat Slerp(const quat &Src, const quat &Dst, real32 Delta);

    union {
        struct {
            real32 w, x, y, z;
        };
        struct {
            real32 w;
            vec3 n;
        };
        real32 q[4];   
    };
};

inline quat::quat()
    : w(0.0f)
    , x(0.0f)
    , y(0.0f)
    , z(0.0f) 
{
}

inline quat::quat(real32 w, real32 x, real32 y, real32 z)
    : w(w)
    , x(x)
    , y(y)
    , z(z)
{
}

quat::quat(real32* Rot)
    : w(Rot[_w_])
    , x(Rot[_x_])
    , y(Rot[_y_])
    , z(Rot[_z_])
{
}

inline quat::quat(real32 angle, const vec3& n) 
{
    real32 alpha = angle / 2.0f;

    w = Cos(alpha); 
    this->n = n;
    this->n *= Sin(alpha);
}

// NOTE(ismail): I expect that w "coordinate" will be fourth in Scalars
inline quat& quat::operator=(const real32* Scalars) 
{
    w = Scalars[_w_];
    x = Scalars[_x_];
    y = Scalars[_y_];
    z = Scalars[_z_];

    return *this;
}

inline quat quat::operator-() const 
{
    quat Result(-w, -x, -y, -z);
    
    return Result;
}

inline real32 quat::Length() const 
{
    real32 LenSquare = SQUARE(w) + SQUARE(x) + SQUARE(y) + SQUARE(z);
    
    return Sqrt(LenSquare);
}

inline quat quat::operator*(const quat &b) const 
{
    quat Result(
        w * b.w - x * b.x - y * b.y - z * b.z,
        w * b.x + x * b.w + y * b.z - z * b.y,
        w * b.y + y * b.w + z * b.x - x * b.z,
        w * b.z + z * b.w + x * b.y - y * b.x
    );
    
    return Result;
}

inline vec3 quat::operator*(const vec3 &b) const 
{
    // x = b.x(w^2 + x^2 - z^2 - y^2) + b.z(2*y*w + 2*z*x) + b.y(2*y*x - 2*z*w);
    // y = b.y(y^2 + w^2 - z^2 - x^2) + b.x(2*x*y + 2*w*z) + b.z(2*z*y - 2*x*w);
    // z = b.z(z^2 + w^2 - x^2 - y^2) + b.y(2*y*z + 2*w*x) + b.x(2*x*z - 2*w*y);

    real32 ww = SQUARE(w);
    real32 xx = SQUARE(x);
    real32 yy = SQUARE(y);
    real32 zz = SQUARE(z);

    real32 wwyy = ww - yy;
    real32 xxzz = xx - zz;

    real32 yw2 = y * w * 2.0f;
    real32 zx2 = z * x * 2.0f;
    real32 yx2 = y * x * 2.0f;
    real32 zw2 = z * w * 2.0f;
    real32 zy2 = z * y * 2.0f;
    real32 xw2 = x * w * 2.0f;

    vec3 Result = {
        b.x * (wwyy + xxzz)         + b.z * (yw2 + zx2)     + b.y * (yx2 - zw2),
        b.y * (yy + ww - zz - xx)   + b.x * (yx2 + zw2)     + b.z * (zy2 - xw2),
        b.z * (wwyy - xxzz)         + b.y * (zy2 + xw2)     + b.x * (zx2 - yw2)
    };

    return Result;
}

inline void quat::Mat3(mat3& Result) const
{
    real32 x2 = x * 2.0f;
    real32 y2 = y * 2.0f;
    real32 z2 = z * 2.0f;

    real32 xx2 = x2 * x;
    real32 yy2 = y2 * y;
    real32 zz2 = z2 * z;

    real32 wx2  = x2 * w;
    real32 xy2  = x2 * y;

    real32 wz2  = z2 * w;
    real32 xz2  = z2 * x;

    real32 wy2  = y2 * w;
    real32 yz2  = y2 * z;

    Result = {
        1.0f - yy2 - zz2,         xy2 - wz2,         xz2 + wy2,
               xy2 + wz2,  1.0f - xx2 - zz2,         yz2 - wx2,
               xz2 - wy2,         yz2 + wx2,  1.0f - xx2 - yy2,
    };
}

inline void quat::Mat4(mat4& Result) const
{
    real32 x2 = x * 2.0f;
    real32 y2 = y * 2.0f;
    real32 z2 = z * 2.0f;

    real32 xx2 = x2 * x;
    real32 yy2 = y2 * y;
    real32 zz2 = z2 * z;

    real32 wx2  = x2 * w;
    real32 xy2  = x2 * y;

    real32 wz2  = z2 * w;
    real32 xz2  = z2 * x;

    real32 wy2  = y2 * w;
    real32 yz2  = y2 * z;

    Result = {
        1.0f - yy2 - zz2,         xy2 - wz2,         xz2 + wy2,     0.0f,
               xy2 + wz2,  1.0f - xx2 - zz2,         yz2 - wx2,     0.0f,
               xz2 - wy2,         yz2 + wx2,  1.0f - xx2 - yy2,     0.0f,
                    0.0f,              0.0f,              0.0f,     1.0f
    };
}

inline void quat::UprightToObject(mat4& Result) const
{
    real32 x2 = x * 2.0f;
    real32 y2 = y * 2.0f;
    real32 z2 = z * 2.0f;

    real32 xx2 = x2 * x;
    real32 yy2 = y2 * y;
    real32 zz2 = z2 * z;

    real32 wx2  = x2 * w;
    real32 xy2  = x2 * y;

    real32 wz2  = z2 * w;
    real32 xz2  = z2 * x;

    real32 wy2  = y2 * w;
    real32 yz2  = y2 * z;

    Result = {
        1.0f - yy2 - zz2,          xy2 + wz2,          xz2 - wy2,  0.0f,
               xy2 - wz2,   1.0f - xx2 - zz2,          yz2 + wx2,  0.0f,
               xz2 + wy2,          yz2 - wx2,   1.0f - xx2 - yy2,  0.0f,
                    0.0f,               0.0f,               0.0f,  1.0f,
    };
}

inline real32 quat::Dot(const quat &b) const 
{
    real32 Result = w * b.w + x * b.x + y * b.y + z * b.z;

    return Result;
}

inline quat quat::Slerp(const quat &Src, const quat &Dst, real32 Delta) 
{
    if (Delta > 0.9999f) {
        return Dst;
    }
    else if (Delta < 0.0001f) {
        return Src;
    }

    real32 k0, k1;
    quat SrcTemp = Src;
    real32 CosOmega = SrcTemp.Dot(Dst);

    if (CosOmega < 0.0f) {
        SrcTemp.w   = -Src.w;
        SrcTemp.x   = -Src.x;
        SrcTemp.y   = -Src.y;
        SrcTemp.z   = -Src.z;
        CosOmega    = -CosOmega;
    }

    if (CosOmega > 0.9999f) {
        k0 = 1.0f - Delta;
        k1 = Delta;
    }
    else {
        real32 SinOmega = Sqrt(1.0f - SQUARE(CosOmega));

        real32 Omega = Atan2(SinOmega, CosOmega);
        real32 OneOverSinOmega = 1.0f / SinOmega;

        k0 = Sin((1.0f - Delta) * Omega) * OneOverSinOmega;
        k1 = Sin(Delta * Omega) * OneOverSinOmega;
    }

    quat Result(
        SrcTemp.w * k0 + Dst.w * k1,
        SrcTemp.x * k0 + Dst.x * k1,
        SrcTemp.y * k0 + Dst.y * k1,
        SrcTemp.z * k0 + Dst.z * k1
    );

    return Result;
}

#endif