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

#define BATTLE_AREA_GRID_VERT_AMOUNT 3
#define ONE_SQUARE_INDEX_AMOUNT 6
#define TERRAIN_INDEX_AMOUNT SQUARE(BATTLE_AREA_GRID_VERT_AMOUNT - 1) * ONE_SQUARE_INDEX_AMOUNT
#define SCENE_OBJECTS_MAX 2

enum OpenGLBuffersLocation {
    GLPositionLocation,
    GLTextureLocation,
    GLNormalsLocation,

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
    TerrainShader,
    ShaderProgramsTypeMax,
};

struct ShaderTextureInfo {
    i32 Location;
    i32 Unit;
    i32 UnitNum;
};

struct ShaderProgram {
    u32     Program;

    union ProgramVariablesStorage {
        struct {
            i32                 ObjectToWorldTransformationLocation;
            i32                 MaterialAmbientColorLocation;
            i32                 MaterialDiffuseColorLocation;
            i32                 MaterialSpecularColorLocation;
            i32                 DirectionalLightColorLocation;
            i32                 DirectionalLightDirectionLocation;
            i32                 DirectionalLightIntensityLocation;
            i32                 DirectionalLightAmbientIntensityLocation;
            i32                 DirectionalLightSpecularIntensityLocation;
            i32                 ViewerPositionLocation;
            ShaderTextureInfo   DiffuseTexture;
            ShaderTextureInfo   SpecularExpMap;
        } Common;

        struct {
        } MeshShader;

        struct {
        } TerrainShader;

    } ProgramVarsStorage;
};

struct DirectionalLight {
    Vec3    Color;
    Vec3    Direction;
    real32  Intensity;
    real32  AmbientIntensity;
    real32  SpecularIntensity;
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

    real32              TranslationDelta;
    real32              RotationDelta;

    Camera              PlayerCamera;
    SceneObject         TestSceneObjects[SCENE_OBJECTS_MAX];
    Terrain             Terrain;

    DirectionalLight    LightSource;
};

#endif