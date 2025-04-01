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

enum OpenGLBuffersLocation{
    POSITION_LOCATION,
    INDEX_ARRAY_LOCATION,
    VERTEX_ARRAY_LOCATION,
    MAX
};

struct GraphicComponent {
    u32 BuffersHandler[MAX];
    u32 ShaderProgram;
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

    Camera PlayerCamera;

    Vec3 Vertices[SQUARE(BATTLE_AREA_GRID_VERT_AMOUNT)];
    u32 Indices[TERRAIN_INDEX_AMOUNT];
    GraphicComponent BattleGrid;
    u32 BattleGridMatLocation;
};

#endif