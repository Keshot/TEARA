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
    u32 Location;
    i32 Unit;
    i32 UnitNum;
};

struct ShaderProgram {
    u32     Program;

    union {
        struct {
            u32                 ObjectToWorldTransformationLocation;
            ShaderTextureInfo   DiffuseTexture;
        } Common;

        struct {
        } MeshShader;

        struct {
        } TerrainShader;

    } ProgramVarsStorage;
};

struct MeshComponentObjects {
    u32 TextureHandle;
    u32 NumIndices;
    u32 IndexOffset;
    u32 VertexOffset;
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
    u32     Indices[TERRAIN_INDEX_AMOUNT];
    u32     VerticesAmount;
    u32     TexturesAmount;
    u32     IndicesAmount;
};

struct Terrain {
    u32     BuffersHandler[GLLocationMax];
    u32     TextureHandle;
    u32     IndicesAmount;
    Vec3    TerrainPosition;
};

struct GameContext {
    bool32          PolygonModeActive;
    bool32          QWasTriggered;

    real32          TranslationDelta;
    real32          RotationDelta;

    Camera          PlayerCamera;
    SceneObject     TestSceneObjects[SCENE_OBJECTS_MAX];
    Terrain         Terrain;
};

#endif