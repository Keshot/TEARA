#ifndef _TEARA_LIB_UTILS_ASSETS_LOADER_H_
#define _TEARA_LIB_UTILS_ASSETS_LOADER_H_

#include "TCore/EnginePlatform.h"
#include "TLib/Utils/Types.h"
#include "TLib/Math/Vector.h"

#define TEXTURE_FILE_NAME_MAX   64

struct AssetsLoaderVars {
    // NOTE (ismail): cache size per thread
    u32 AssetsLoaderCacheSize;
};

struct Mesh {
    char    TextureFilePath[TEXTURE_FILE_NAME_MAX];
    bool32  HaveTexture;
    u32     IndexOffset;
    u32     VertexOffset;
    u32     IndicesAmount;
};

// ASSETS TYPES
// NOTE (ismail): may be use separate x, y, z for positions and normals, and x, y for textures coordinat
// we need to chek will it increase performance
struct ObjFile {
    Mesh*           Meshes;
    Vec3*           Positions;
    Vec3*           Normals;
    Vec2*           TextureCoord;
    u32*            Indices;
    u32             MeshesCount;
    u32             PositionsCount;
    u32             NormalsCount;
    u32             TexturesCount;
    u32             IndicesCount;
};

// ASSETS TYPES END

void AssetsLoaderInit(Platform *PlatformContext, AssetsLoaderVars *LoaderVars);

struct ObjFileLoaderFlags {
    unsigned int GenerateSmoothNormals  : 1;
    unsigned int SelfGenerateNormals    : 1;
};

// NOTE (ismail): for now in File variable we must store actual buffers outside of function
Statuses LoadObjFile(const char *Path, ObjFile *ReadedFile, ObjFileLoaderFlags Flags);

#endif