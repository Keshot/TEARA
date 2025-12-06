#ifndef _TEARA_ASSETS_GLTF_LOADER_H_
#define _TEARA_ASSETS_GLTF_LOADER_H_

#include "Core/Types.h"
#include "Math/Quat.h"

#include <stdlib.h>

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

struct GltfAnimationArray {
    GltfAnimation* Animations;
    i32 AnimationsAmount;
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
    GltfJoint*  Joints;
    i32         JointsAmount;
};

struct GltfFile {
    enum {
        Success,
        Failed
    };

    GltfFile();

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

    GltfAnimationArray Animations;
};

inline GltfFile::GltfFile() 
    : Meshes(0)
    , MeshesAmount(0)
    , Skins(0)
    , SkinsAmount(0)
{
}

#endif