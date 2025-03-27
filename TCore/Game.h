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

struct GameContext {
    real32 Trans;
    real32 Delta;
    real32 Rot;
    real32 RotDelta;

    ObjFile ReadedFile;
    u32 FinalShaderProgramm;
    u32 VAO;
    u32 MatLocation;

    Camera PlayerCamera;
};

#endif