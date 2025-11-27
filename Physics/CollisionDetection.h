#ifndef TEARA_PHYSICS_COLLISION_H_
#define TEARA_PHYSICS_COLLISION_H_

#include "Core/Types.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"

#define OBB_EPSILON 0.0001f

enum BoundingVolumeType {
    SphereVolume,
    AABBVolume,
    OBBVolume,
};

struct AABB {
    vec3 Center;
    vec3 Extens; // Rx, Ry, Rz radius of halfwidth
};

struct Sphere {
    vec3    Center;
    real32  Radius;
};

struct OBB {
    vec3 Center;
    vec3 Extens;
    vec3 Axis[3];
};

union BoundingVolumes {
    AABB            AxisBox;
    Sphere          Sphere;
    OBB             OrientedBox;
};

struct BoundingVolume {
    BoundingVolumeType  VolumeType;
    BoundingVolumes     VolumeData;
};

void AABBRecalculate(mat4 *Rotation, vec3 *Translation, AABB *Original, AABB *Result)
{
    // NOTE (ismail): for now aabb center is always x = 0, y = 0, z = 0, fix it in future
    Result->Center.x = Translation->x;
    Result->Center.y = Translation->y;
    Result->Center.z = Translation->z;

    for (i32 i = 0; i < 3; ++i) {
        Result->Extens[i] = 0.0f;

        for (i32 j = 0; j < 3; ++j) {
            Result->Extens[i] += Fabs((*Rotation)[i][j]) * Original->Extens[j];
        }
    }
}

bool32 AABBToAABBTestOverlap(AABB *A, AABB *B)
{
    // TODO (ismail): convert to SIMD
    // TODO (ismail): may be calculate A.Center - B.Center one time and use it?
    
    if (Fabs(A->Center.x - B->Center.x) > (A->Extens.x + B->Extens.x)) {
        return 0;
    }
    if (Fabs(A->Center.y - B->Center.y) > (A->Extens.y + B->Extens.y)) {
        return 0;
    }
    if (Fabs(A->Center.z - B->Center.z) > (A->Extens.z + B->Extens.z)) {
        return 0;
    }

    return 1;
}

bool32 SphereToAABBTestOverlap(AABB *A, Sphere *B)
{
    vec3 DistanceVector = A->Center - B->Center;
    real32 DistanceSquare = vec3::DotProduct(DistanceVector, DistanceVector);

    real32 RadiusSumX = A->Extens.x + B->Radius;
    real32 RadiusSumY = A->Extens.y + B->Radius;
    real32 RadiusSumZ = A->Extens.z + B->Radius;

    if (DistanceSquare > SQUARE(RadiusSumX)) {
        return 0;
    }
    if (DistanceSquare > SQUARE(RadiusSumY)) {
        return 0;
    }
    if (DistanceSquare > SQUARE(RadiusSumZ)) {
        return 0;
    }

    return 1;
}

bool32 SphereToSphereTestOverlap(Sphere *A, Sphere *B)
{
    vec3 DistanceVector = A->Center - B->Center;
    real32 DistanceSquare = vec3::DotProduct(DistanceVector, DistanceVector);

    real32 RadiusSum = A->Radius + B->Radius;
    real32 RadiusSquare = SQUARE(RadiusSum);

    return DistanceSquare < RadiusSquare;
}

bool32 OBBToOBBTestOverlap(OBB *A, OBB *B)
{
    // TODO (ismail): use this for all magic numbers into this function
    enum _local { x = 0, y = 1, z = 2 };

    // TODO (ismail): T * (Ax X Bx) = (T * Az) * (Ay * Bx) - (T * Ay) * (Az * Bx) optimization
    // TODO (ismail): refactoring
    // TODO (ismail): optimization when we aligned any other three axis

    mat3 R, AbsR;
    real32 ProjT, MaxA, MaxB;
    vec3 EdgesCrossProduct;
    vec3 T = B->Center - A->Center;

    // NOTE (ismail): compute Ax * Bx, Ax * By, Ax * Bz 
    //                        Ay * Bx, Ay * By, Ay * Bz => AT * B
    //                        Az * Bx, Az * By, Az * Bz
    for (i32 i = 0; i < 3; ++i) {
        for (i32 j = 0; j < 3; ++j) {
            R[i][j] = vec3::DotProduct(A->Axis[i], B->Axis[j]);
            AbsR[i][j] = Fabs(R[i][j]) + OBB_EPSILON;
        }
    }

    // NOTE (ismail): check A three axis first (Ax, Ay, Az)
    //  L = Ax
    // |T * Ax| > Eax + |Ebx * Bx * Ax| + |Eby * By * Ax| + |Ebz * Bz * Ax|
    //  L = Ay
    // |T * Ay| > Eay + |Ebx * Bx * Ay| + |Eby * By * Ay| + |Ebz * Bz * Ay|
    //  L = Az
    // |T * Az| > Eaz + |Ebx * Bx * Az| + |Eby * By * Az| + |Ebz * Bz * Az|
    for (i32 i = 0; i < 3; ++i) {
        ProjT = Fabs(vec3::DotProduct(T, A->Axis[i]));
        MaxA = A->Extens[i];
        MaxB = B->Extens[0] * AbsR[i][0] + B->Extens[1] * AbsR[i][1] + B->Extens[2] * AbsR[i][2];

        if (ProjT > MaxA + MaxB) {
            return 0;
        }
    }

    // NOTE (ismail): check B three axis (Bx, By, Bz)
    //  L = Bx
    // |T * Bx| > Ebx + |Eax * Ax * Bx| + |Eay * Ay * Bx| + |Eaz * Az * Bx|
    //  L = By
    // |T * By| > Eby + |Eax * Ax * By| + |Eay * Ay * By| + |Eaz * Az * By|
    //  L = Bz
    // |T * Bz| > Ebz + |Eax * Ax * Bz| + |Eay * Ay * Bz| + |Eaz * Az * Bz|
    for (i32 i = 0; i < 3; ++i) {
        ProjT = Fabs(vec3::DotProduct(T, B->Axis[i]));
        MaxA = A->Extens[0] * AbsR[0][i] + A->Extens[1] * AbsR[1][i] + A->Extens[2] * AbsR[2][i];
        MaxB = B->Extens[i];

        if (ProjT > MaxA + MaxB) {
            return 0;
        }
    }

    // NOTE (ismail): check nine edges

    //  L = Ax X Bx
    // |T * (Ax X Bx)| > |-Eay * Bx * Az| + |Eaz * Bx * Ay| + |Eby * Ax * Bz| + |-Ebz * Ax * By|
    ProjT = vec3::DotProduct(T, A->Axis[0].Cross(B->Axis[0]));
    MaxA = A->Extens[1] * AbsR[2][0] + A->Extens[2] * AbsR[1][0];
    MaxB = B->Extens[1] * AbsR[0][2] + B->Extens[2] * AbsR[0][1];
    
    if (ProjT > MaxA + MaxB) {
        return 0;
    }

    //  L = Ax X By
    // |T * (Ax X By)| > |-Eay * By * Az| + |Eaz * By * Ay| + |-Ebx * Ax * Bz| + |Ebz * Ax * Bx|
    ProjT = vec3::DotProduct(T, A->Axis[x].Cross(B->Axis[y]));
    MaxA = A->Extens[y] * AbsR[z][y] + A->Extens[z] * AbsR[y][y];
    MaxB = B->Extens[x] * AbsR[x][z] + B->Extens[z] * AbsR[x][x];
    
    if (ProjT > MaxA + MaxB) {
        return 0;
    }

    //  L = Ax X Bz
    // |T * (Ax X Bz)| > |-Eay * Bz * Az| + |Eaz * Bz * Ay| + |Ebx * Ax * By| + |-Eby * Ax * Bx|
    ProjT = vec3::DotProduct(T, A->Axis[x].Cross(B->Axis[z]));
    MaxA = A->Extens[y] * AbsR[z][z] + A->Extens[z] * AbsR[y][z];
    MaxB = B->Extens[x] * AbsR[x][y] + B->Extens[y] * AbsR[x][x];
    
    if (ProjT > MaxA + MaxB) {
        return 0;
    }

    //  L = Ay X Bx
    // |T * (Ay X Bx)| > |Eax * Bx * Az| + |-Eaz * Bx * Ax| + |Eby * Ay * Bz| + |-Ebz * Ay * By|
    ProjT = vec3::DotProduct(T, A->Axis[y].Cross(B->Axis[x]));
    MaxA = A->Extens[x] * AbsR[z][x] + A->Extens[z] * AbsR[x][x];
    MaxB = B->Extens[y] * AbsR[y][z] + B->Extens[z] * AbsR[y][y];
    
    if (ProjT > MaxA + MaxB) {
        return 0;
    }

    // L = Ay X By
    // |T * (Ay X By)| > |Eax * By * Az| + |-Eaz * By * Ax| + |-Ebx * Ay * Bz| + |Ebz * Ay * Bx|
    ProjT = vec3::DotProduct(T, A->Axis[y].Cross(B->Axis[y]));
    MaxA = A->Extens[x] * AbsR[z][y] + A->Extens[z] * AbsR[x][y];
    MaxB = B->Extens[x] * AbsR[y][z] + B->Extens[z] * AbsR[y][y];
    
    if (ProjT > MaxA + MaxB) {
        return 0;
    }

    // L = Ay X Bz
    // |T * (Ay X Bz)| > |Eax * Bz * Az| + |-Eaz * Bz * Ax| + |Ebx * Ay * By| + |-Eby * Ay * Bx|
    ProjT = vec3::DotProduct(T, A->Axis[y].Cross(B->Axis[z]));
    MaxA = A->Extens[x] * AbsR[z][z] + A->Extens[z] * AbsR[x][z];
    MaxB = B->Extens[x] * AbsR[y][y] + B->Extens[y] * AbsR[y][x];
    
    if (ProjT > MaxA + MaxB) {
        return 0;
    }

    //  L = Az X Bx
    // |T * (Az X Bx)| > |-Eax * Bx * Ay| + |Eay * Bx * Ax| + |Eby * Az * Bz| + |-Ebz * Az * By|
    ProjT = vec3::DotProduct(T, A->Axis[z].Cross(B->Axis[x]));
    MaxA = A->Extens[x] * AbsR[y][x] + A->Extens[y] * AbsR[x][x];
    MaxB = B->Extens[y] * AbsR[z][z] + B->Extens[z] * AbsR[z][y];
    
    if (ProjT > MaxA + MaxB) {
        return 0;
    }

    //  L = Az X By
    // |T * (Az X By)| > |-Eax * By * Ay| + |Eay * By * Ax| + |-Ebx * Az * Bz| + |Ebz * Az * Bx|
    ProjT = vec3::DotProduct(T, A->Axis[z].Cross(B->Axis[y]));
    MaxA = A->Extens[x] * AbsR[y][y] + A->Extens[y] * AbsR[x][y];
    MaxB = B->Extens[x] * AbsR[z][z] + B->Extens[z] * AbsR[z][x];
    
    if (ProjT > MaxA + MaxB) {
        return 0;
    }

    //  L = Az X Bz
    // |T * (Az X Bz)| > |-Eax * Bz * Ay| + |Eay * Bz * Ax| + |Ebx * Az * By| + |-Eby * Az * Bx|
    ProjT = vec3::DotProduct(T, A->Axis[z].Cross(B->Axis[z]));
    MaxA = A->Extens[x] * AbsR[y][z] + A->Extens[y] * AbsR[x][z];
    MaxB = B->Extens[x] * AbsR[z][y] + B->Extens[y] * AbsR[z][x];
    
    if (ProjT > MaxA + MaxB) {
        return 0;
    }

    return 1;
}

#endif