#ifndef _TEARA_ASSETS_GLTF_LOADER_H_
#define _TEARA_ASSETS_GLTF_LOADER_H_

#include "Core/Types.h"
#include "Math/Quat.h"

#include <stdlib.h>

struct GltfMemoryArena {
    GltfMemoryArena();
    ~GltfMemoryArena();

    GltfMemoryArena(const GltfMemoryArena&) = delete;
    GltfMemoryArena(GltfMemoryArena&&) = delete;

    GltfMemoryArena& operator=(const GltfMemoryArena&) = delete;
    GltfMemoryArena& operator=(GltfMemoryArena&&) = delete;

    bool32 Init(u64 size);

    void Clear();
    void* Alloc(u64 Size);

    byte*   Base;
    u64     Size;
    byte*   NextAvail;
    u64     AvailSize;
};

inline GltfMemoryArena::GltfMemoryArena()
    : Base(0)
    , Size(0)
    , NextAvail(0)
    , AvailSize(0)
{
}

inline GltfMemoryArena::~GltfMemoryArena()
{
    if (Base) {
        free(Base);
    }
}

struct GltfJointIndex {
    i32 x, y, z, w;
};

struct GltfMaterial {
    bool32  HaveTexture;
    bool32  HaveSpecularExponent;
    char*   TextureFilePath;
    char*   SpecularExpFilePath;
    vec3    AmbientColor;
    vec3    DiffuseColor;
    vec3    SpecularColor;
};

struct GltfPrimitive {
    GltfMaterial    Material;
    vec3*           Positions;
    vec3*           Normals;
    vec2*           TextureCoord;
    vec4*           BoneWeights;
    GltfJointIndex* BoneIds;
    u32*            Indices;
    u32             PositionsCount;
    u32             NormalsCount;
    u32             TexturesCount;
    u32             BoneWeightsCount;
    u32             BoneIdsCount;
    u32             IndicesCount;
};

struct GltfMesh {
    GltfPrimitive* Primitives;
    i32 PrimitivesAmount;
};

struct GltfAnimationTransform {
    enum InterpolationType {
        IStep,
        ILinear,
        IMax,
    };

    union TransformationStorage {
        vec3    Translation;
        quat    Rotation;
        vec3    Scale;
    };

    InterpolationType       IType;
    real32*                 Keyframes;
    TransformationStorage*  Transforms;
    i32                     Amount;
    bool32                  Validated;
};

struct GltfAnimationFrame {
    enum {
        ATranslation,
        ARotation,
        AScale,
        AMax,
    };

    GltfAnimationTransform Transformations[AMax];
};

struct GltfAnimation {
    GltfAnimationFrame* PerBonesFrame;
    i32                 FramesAmount;
    real32              Duration;
};

struct GltfJoint {
    char*   BoneName;
    u32     NameLen;
    i32     Parent;
    i32*    Children;
    i32     ChildrenAmount;
    mat4    InverseBindMatrix;
};

struct GltfSkin {
    i32*        Roots;
    i32         RootsAmount;
    i32         JointsAmount;
    GltfJoint*  Joints;
};

struct GltfFile {
    enum {
        Success,
        Failed
    };

    GltfFile();
    GltfFile(u64 ArenaLen, i32 RootMax);

    GltfFile(const GltfFile&) = delete;
    GltfFile(GltfFile&&) = delete;

    GltfFile& operator=(const GltfFile&) = delete;
    GltfFile& operator=(GltfFile&&) = delete;

    ~GltfFile();

    bool32 Read(const char* Name);

    GltfMesh* Meshes;
    i32 MeshesAmount;

    GltfSkin* Skins;
    i32 SkinsAmount;

    GltfAnimation* Animations;
    i32 AnimationsAmount;

    GltfMemoryArena Arena;

private:
    u64 MemoryArenaSize;
    i32 RootJointsMax;
};

inline GltfFile::~GltfFile() = default;

inline GltfFile::GltfFile() 
    : Meshes(0)
    , MeshesAmount(0)
    , Skins(0)
    , SkinsAmount(0)
    , MemoryArenaSize(10 * 1000 * 1000) // 10mb
    , RootJointsMax(1)
{
}

inline GltfFile::GltfFile(u64 ArenaLen, i32 RootMax)
    : Meshes(0)
    , MeshesAmount(0)
    , Skins(0)
    , SkinsAmount(0)
    , MemoryArenaSize(ArenaLen)
    , RootJointsMax(RootMax)
{

}

#endif