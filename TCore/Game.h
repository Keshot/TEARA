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

struct Camera {
    Vec3        Position;
    Rotation    Rotator;
};

#define BATTLE_AREA_GRID_VERT_AMOUNT 3
#define ONE_SQUARE_INDEX_AMOUNT 6
#define TERRAIN_INDEX_AMOUNT SQUARE(BATTLE_AREA_GRID_VERT_AMOUNT - 1) * ONE_SQUARE_INDEX_AMOUNT

enum OpenGLBuffersLocation {
    POSITION_LOCATION,
    TEXTURES_LOCATION,
    INDEX_ARRAY_LOCATION,
    VERTEX_ARRAY_LOCATION,
    LOCATION_MAX
};

struct GraphicComponent {
    u32 BuffersHandler[LOCATION_MAX];
    u32 ShaderProgram;
};

struct WorldTransform {
    Vec3        Position;
    Rotation    Rotation;
};

enum ShaderProgramsType {
    MeshShader,

    ShaderProgramsTypeMax
};

struct ShaderProgramsCache {
    i32 ShadersPrograms[ShaderProgramsTypeMax];
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
    u32                     BuffersHandler[LOCATION_MAX];
    u32                     ShaderProgram;
    u32                     WorldTransformMatrixLocation;
    u32                     DiffuseTextureLocation;
};

struct SceneObject {
    WorldTransform  Transform;
    MeshComponent   ObjMesh;
};

struct GameContext {
    bool32 PolygonModeActive;

    real32 Trans;
    real32 Delta;
    real32 Rot;
    real32 RotDelta;

    ObjFile ReadedFile;
    u32 FinalShaderProgram;
    u32 VAO;
    u32 MatLocation;
    u32 TextureHandle;

    bool32 QWasTriggered;

    Camera PlayerCamera;

    Vec3 Vertices[SQUARE(BATTLE_AREA_GRID_VERT_AMOUNT)];
    Vec2 Textures[SQUARE(BATTLE_AREA_GRID_VERT_AMOUNT)];
    u32 Indices[TERRAIN_INDEX_AMOUNT];
    GraphicComponent BattleGrid;
    u32 BattleGridMatLocation;
    u32 TerrainTextureHandle;

    SceneObject TestSceneObject;
};

#endif