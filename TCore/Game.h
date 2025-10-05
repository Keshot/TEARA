#ifndef _TEARA_GAME_PLATFORM_H_
#define _TEARA_GAME_PLATFORM_H_

// NOTE(ismail): remove this shit
#include <string>
#include <map>

#include "TLib/Utils/Types.h"
#include "TLib/Utils/AssetsLoader.h"
#include "TLib/Math/Vector.h"

struct Rotation {
    real32 Heading;
    real32 Pitch;
    real32 Bank;
};

#define BATTLE_AREA_GRID_VERT_AMOUNT    3
#define ONE_SQUARE_INDEX_AMOUNT         6
#define TERRAIN_INDEX_AMOUNT            SQUARE(BATTLE_AREA_GRID_VERT_AMOUNT - 1) * ONE_SQUARE_INDEX_AMOUNT
#define SCENE_OBJECTS_MAX               3
#define DYNAMIC_SCENE_OBJECTS_MAX       1
#define MAX_POINTS_LIGHTS               2
#define MAX_SPOT_LIGHTS                 1
#define MAX_BONES                       200
#define MAX_KEYFRAMES                   400
#define MAX_CHARACTER_ANIMATIONS        20
#define MAX_JOINT_CHILDREN_AMOUNT       10
#define MAX_MESH_PRIMITIVES             5
#define MAX_MESHES                      1

enum OpenGLBuffersLocation {
    // STATIC MESH
    GLPositionLocation,
    GLTextureLocation,
    GLNormalsLocation,
    // SKELETAL MESH
    GLBoneIndicesLocation,
    GLBoneWeightsLocation,
    // MISC
    GLIndexArrayLocation,
    GLVertexArrayLocation,

    GLLocationMax,
};

const Vec3 GRAVITY = { 0.0f, -10.0f, 0.0f };

enum _Local_Constants_ {
    _x_ = 0,
    _y_ = 1,
    _z_ = 2,
    _w_ = 3,
};

struct Quat {
    union {
        struct {
            real32 w, x, y, z;
        };
        struct {
            real32 w;
            Vec3 n;
        };
        real32 ValueHolder[4];   
    };

    Quat()
        : w(0.0f)
        , x(0.0f)
        , y(0.0f)
        , z(0.0f) 
    {
    }

    Quat(real32 w, real32 x, real32 y, real32 z)
        : w(w)
        , x(x)
        , y(y)
        , z(z)
    {
    }

    Quat(real32 angle, const Vec3& n) {
        real32 alpha = angle / 2.0f;

        w = cosf(alpha); 
        this->n = n;
        this->n *= sinf(alpha);
    }

    // NOTE(ismail): I expect that w "coordinate" will be fourth in Scalars
    Quat& operator=(const real32* Scalars) 
    {
        w = Scalars[_w_];
        x = Scalars[_x_];
        y = Scalars[_y_];
        z = Scalars[_z_];

        return *this;
    }

    Quat operator-() const {
        Quat Result(-w, -x, -y, -z);
        return Result;
    }

    real32 Length() const {
        return sqrtf(SQUARE(w) + SQUARE(x) + SQUARE(y) + SQUARE(z));
    }

    Quat operator*(const Quat &b) const {
        Quat Result(
            w * b.w - x * b.x - y * b.y - z * b.z,
            w * b.x + x * b.w + y * b.z - z * b.y,
            w * b.y + y * b.w + z * b.x - x * b.z,
            w * b.z + z * b.w + x * b.y - y * b.x
        );
        return Result;
    }

    Vec3 operator*(const Vec3 &b) const 
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

        Vec3 Result = {
            b.x * (wwyy + xxzz)         + b.z * (yw2 + zx2)     + b.y * (yx2 - zw2),
            b.y * (yy + ww - zz - xx)   + b.x * (yx2 + zw2)     + b.z * (zy2 - xw2),
            b.z * (wwyy - xxzz)         + b.y * (zy2 + xw2)     + b.x * (zx2 - yw2)
        };

        return Result;
    }

    void ToMat3(Mat3x3 *Result) const
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

        *Result = {
            1.0f - yy2 - zz2,         xy2 - wz2,         xz2 + wy2,
                   xy2 + wz2,  1.0f - xx2 - zz2,         yz2 - wx2,
                   xz2 - wy2,         yz2 + wx2,  1.0f - xx2 - yy2,
        };
    }

    void ToMat4(Mat4x4 *Result) const
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

        *Result = {
            1.0f - yy2 - zz2,         xy2 - wz2,         xz2 + wy2,     0.0f,
                   xy2 + wz2,  1.0f - xx2 - zz2,         yz2 - wx2,     0.0f,
                   xz2 - wy2,         yz2 + wx2,  1.0f - xx2 - yy2,     0.0f,
                        0.0f,              0.0f,              0.0f,     1.0f
        };
    }

    void ToUprightToObjectMat4(Mat4x4 *Result) const
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

        *Result = {
            1.0f - yy2 - zz2,          xy2 + wz2,          xz2 - wy2,  0.0f,
                   xy2 - wz2,   1.0f - xx2 - zz2,          yz2 + wx2,  0.0f,
                   xz2 + wy2,          yz2 - wx2,   1.0f - xx2 - yy2,  0.0f,
                        0.0f,               0.0f,               0.0f,  1.0f,
        };
    }

    inline real32 Dot(const Quat &b) const {
        real32 Result = w * b.w + x * b.x + y * b.y + z * b.z;
        return Result;
    }

    static Quat Slerp(const Quat &Src, const Quat &Dst, real32 Delta) {
        if (Delta > 0.9999f) {
            return Dst;
        }
        else if (Delta < 0.0001f) {
            return Src;
        }

        real32 k0, k1;
        Quat SrcTemp = Src;
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
            real32 SinOmega = sqrtf(1.0f - SQUARE(CosOmega));

            real32 Omega = atan2f(SinOmega, CosOmega);
            real32 OneOverSinOmega = 1.0f / SinOmega;

            k0 = sinf((1.0f - Delta) * Omega) * OneOverSinOmega;
            k1 = sinf(Delta * Omega) * OneOverSinOmega;
        }

        Quat Result(
            SrcTemp.w * k0 + Dst.w * k1,
            SrcTemp.x * k0 + Dst.x * k1,
            SrcTemp.y * k0 + Dst.y * k1,
            SrcTemp.z * k0 + Dst.z * k1
        );

        return Result;
    }
};

// From a to b with factor of t
Vec3 Lerp(const Vec3 &a, const Vec3 &b, real32 t)
{
    Vec3 To = b - a;
    return To * t;
}

struct JointsInfo {
    std::string BoneName;
    JointsInfo* Parent;
    JointsInfo* Children[MAX_JOINT_CHILDREN_AMOUNT];
    i32         ChildrenAmount;
    Vec3        DefaultTranslation;
    Vec3        DefaultScale;
    Quat        DefaultRotation;
    Mat4x4      InverseBindMatrix;
};

struct BoneIDs {
    i32 OriginalBoneID;
    i32 BoneID;
};

struct Skinning {
    std::map<std::string, BoneIDs>  Bones;
    JointsInfo*                     Joints;
    u32                             JointsAmount;
};

enum AnimationType {
    ATranslation,
    ARotation,
    AScale,
    AMax,
};

enum InterpolationType {
    IStep,
    ILinear,
    IMax,
};

union TransformationStorage {
    Vec3    Translation;
    Quat    Rotation;
    Vec3    Scale;
};

struct AnimationTransformation {
    bool32                  Valid;
    InterpolationType       IType;
    real32                  Keyframes[MAX_KEYFRAMES];
    TransformationStorage   Transforms[MAX_KEYFRAMES];
    i32                     Amount;
};

struct AnimationFrame {
    i32                     Target;
    i32                     OriginalBoneID;
    AnimationTransformation Transformations[AMax];
};

struct Animation {
    AnimationFrame  PerBonesFrame[MAX_BONES];
    i32             FramesAmount;
    real32          MaxDuration;
};

struct AnimationsArray {
    Animation   Anims[MAX_CHARACTER_ANIMATIONS];
    i32         AnimsAmount;
};

struct WorldTransform {
    Vec3        Position;
    Rotation    Rotation;
    Vec3        Scale;
};

struct Camera {
    WorldTransform Transform;
};

enum ShaderProgramsType {
    MeshShader,
    SkeletalMeshShader,
    ParticlesShader,
    DebugDraw,
    ShaderProgramsTypeMax,
};

struct ShaderProgramVariablesStorage {

    struct ObjectTransform {
        i32 ObjectToCameraSpaceTransformationLocation;
        i32 ObjectGeneralTransformationLocation;
    } Transform;

    struct ObjectMaterial {

        struct ShaderTextureInfo {
            i32 Location;
            i32 Unit;
            i32 UnitNum;
        };

        i32 MaterialAmbientColorLocation;
        i32 MaterialDiffuseColorLocation;
        i32 MaterialSpecularColorLocation;

        ShaderTextureInfo   DiffuseTexture;
        ShaderTextureInfo   SpecularExpMap;
    } MaterialInfo;

    struct LightWork {

        struct LightSpecLocations {
            i32 ColorLocation;
            i32 IntensityLocation;
            i32 AmbientIntensityLocation;
            i32 SpecularIntensityLocation;
        };

        struct LightAttenuationLocations {
            i32 PositionLocation;
            i32 DisctanceMaxLocation;
            i32 DisctanceMinLocation;
            i32 AttenuationFactorLocation;
        };

        struct PointLightLocations {
            LightSpecLocations          SpecLocation;
            LightAttenuationLocations   AttenuationLocation;
        };

        struct SpotLightLocations {
            LightSpecLocations          SpecLocation;
            LightAttenuationLocations   AttenuationLocation;
            i32                         DirectionLocation;
            i32                         CosCutoffAngleLocation;
            i32                         CutoffAttenuationFactorLocation;
        };

        PointLightLocations PointLightsLocations[MAX_POINTS_LIGHTS];

        SpotLightLocations  SpotLightsLocations[MAX_SPOT_LIGHTS];

        LightSpecLocations  DirectionalLightSpecLocations;
        i32                 DirectionalLightDirectionLocation;

        i32                 ViewerPositionLocation;
        i32                 PointLightsAmountLocation;
        i32                 SpotLightsAmountLocation;
    } Light;

    struct AnimationInfo {
        i32 AnimationMatricesLocation[MAX_BONES];
        i32 BoneIDLocation;
    } Animation;

};

struct ShaderProgram {
    u32                             Program;
    ShaderProgramVariablesStorage   ProgramVarsStorage;
};

struct LightSpec {
    Vec3    Color;
    real32  Intensity;
    real32  AmbientIntensity;
    real32  SpecularIntensity;
};

struct LightAttenuation {
    Vec3        Position;
    real32      DisctanceMax;
    real32      DisctanceMin;
    real32      AttenuationFactor;
};

struct DirectionalLight {
    LightSpec   Specification;
    Vec3        Direction;
};

struct PointLight {
    LightSpec           Specification;
    LightAttenuation    Attenuation;
};

struct SpotLight {
    LightSpec           Specification;
    LightAttenuation    Attenuation;
    Vec3                Direction;
    real32              CosCutoffAngle;
    real32              CutoffAttenuationFactor;
};

struct MeshMaterial {
    bool32  HaveTexture;
    u32     TextureHandle;
    bool32  HaveSpecularExponent;
    u32     SpecularExponentMapTextureHandle;
    Vec3    AmbientColor;
    Vec3    DiffuseColor;
    Vec3    SpecularColor;
};

struct MeshComponentObjects {
    MeshMaterial    Material;
    u32             NumIndices;
    u32             IndexOffset;
    u32             VertexOffset;
};

struct SkeletalComponent {
    AnimationsArray*    Animations;
    Skinning            Skin;
};

struct MeshComponent {
    const char*             ObjectPath;
    MeshComponentObjects*   MeshesInfo;
    u32                     MeshesAmount;
    u32                     BuffersHandler[GLLocationMax];
};

struct MeshPrimitives {
    MeshMaterial    Material;
    u32             BuffersHandler[GLLocationMax];
    u32             InidicesAmount;
};

struct SkeletalMeshComponent {
    const char*         ObjectPath;
    MeshPrimitives      Primitives[MAX_MESH_PRIMITIVES];
    i32                 PrimitivesAmount;
    SkeletalComponent   Skelet;
};

struct DynamicSceneObject;

struct ObjectNesting {
    std::string         AttachedToBone;
    DynamicSceneObject* Parent;
};

struct SceneObject {
    WorldTransform  Transform;
    MeshComponent   ObjMesh;
    ObjectNesting   Nesting;
};

struct DynamicSceneObject {
    WorldTransform          Transform;
    SkeletalMeshComponent   ObjMesh;
    ObjectNesting           Nesting;
};

struct TerrainLoadFile {
    Vec3    Vertices[SQUARE(BATTLE_AREA_GRID_VERT_AMOUNT)];
    Vec2    Textures[SQUARE(BATTLE_AREA_GRID_VERT_AMOUNT)];
    Vec3    Normals[SQUARE(BATTLE_AREA_GRID_VERT_AMOUNT)];
    u32     Indices[TERRAIN_INDEX_AMOUNT];
    u32     VerticesAmount;
    u32     TexturesAmount;
    u32     NormalsAmount;
    u32     IndicesAmount;
};

struct Terrain {
    u32             BuffersHandler[GLLocationMax];
    u32             TextureHandle;
    u32             IndicesAmount;
    WorldTransform  Transform;
};

struct SkinningMatricesStorage {
    Mat4x4  Matrices[MAX_BONES];
    i32     Amount;
};

#define PARTICLES_MAX 1

struct ParticleSystem {
    u32 BuffersHandler[GLLocationMax];
    i32 IndicesAmount;
};

struct Particle {
    WorldTransform  Transform;
    Vec3            Velocity;
    Vec3            Acceleration;
    real32          Damping;
    real32          InverseMass;

    void Integrate(real32 DeltaT)
    {
        // x(t) = x0 + v*t
        // v(t) = v0*t + a*t
        
        Transform.Position += Velocity * DeltaT;
        
        // NOTE(Ismail): it may cause some perfomance issue, if we will be simulate many particles
        // so check it for optimization if it will need
        real32 DampingEffect = powf(Damping, DeltaT);

        Vec3 AccelerationTmp = Acceleration * DeltaT;

        Velocity += AccelerationTmp;

        Velocity *= DampingEffect;
    }
};

struct FrameData {
    Mat4x4                  CameraTransformation;
    SkinningMatricesStorage SkinMatrix;
};

struct GameContext {
    real32  DeltaTimeSec;

    bool32  PolygonModeActive;
    bool32  QWasTriggered;
    bool32  MWasTriggered;
    bool32  EditorModeOn;

    bool32  ArrowUpWasTriggered;
    bool32  AnimationBlending;

    real32  TranslationDelta;
    real32  RotationDelta;
    real32  AnimationBlendingFactor;

    real32  FrAnimationDuration;
    real32  ScAnimationDuration;
    i32     CurrentStep;

    i32     BoneID;

    Camera              PlayerCamera;
    SceneObject         TestSceneObjects[SCENE_OBJECTS_MAX];
    DynamicSceneObject  TestDynamocSceneObjects[DYNAMIC_SCENE_OBJECTS_MAX];
    Terrain             Terrain;
    Particle            SceneParticles[PARTICLES_MAX];
    ParticleSystem      ParticleSystem;

    DirectionalLight    LightSource;
    PointLight          PointLights[MAX_POINTS_LIGHTS];
    SpotLight           SpotLights[MAX_SPOT_LIGHTS];

    bool32 EWasPressed;
};

#endif