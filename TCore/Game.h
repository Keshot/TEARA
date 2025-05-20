#ifndef _TEARA_GAME_PLATFORM_H_
#define _TEARA_GAME_PLATFORM_H_

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
#define SCENE_OBJECTS_MAX               2
#define DYNAMIC_SCENE_OBJECTS_MAX       1
#define MAX_POINTS_LIGHTS               2
#define MAX_SPOT_LIGHTS                 1

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

struct WorldTransform {
    Vec3        Position;
    Rotation    Rotation;
};

struct Camera {
    WorldTransform Transform;
};

enum ShaderProgramsType {
    MeshShader,
    SkeletalMeshShader,
    ShaderProgramsTypeMax,
};

struct ShaderProgramVariablesStorage {

    struct ObjectTransform {
        i32                 ObjectToWorldTransformationLocation;
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

struct MeshComponent {
    const char*             ObjectPath;
    MeshComponentObjects*   MeshesInfo;
    u32                     MeshesAmount;
    u32                     BuffersHandler[GLLocationMax];
};

struct SceneObject {
    WorldTransform  Transform;
    MeshComponent   ObjMesh;
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

struct GameContext {
    bool32              PolygonModeActive;
    bool32              QWasTriggered;

    bool32              ArrowUpWasTriggered;

    real32              TranslationDelta;
    real32              RotationDelta;

    i32                 BoneID;

    Camera              PlayerCamera;
    SceneObject         TestSceneObjects[SCENE_OBJECTS_MAX];
    SceneObject         TestDynamocSceneObjects[DYNAMIC_SCENE_OBJECTS_MAX];
    Terrain             Terrain;

    DirectionalLight    LightSource;
    PointLight          PointLights[MAX_POINTS_LIGHTS];
    SpotLight           SpotLights[MAX_SPOT_LIGHTS];
};

#endif