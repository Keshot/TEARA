#ifndef _TEARA_LIB_UTILS_ASSETS_LOADER_H_
#define _TEARA_LIB_UTILS_ASSETS_LOADER_H_

#include "TLib/Utils/Types.h"
#include "TLib/Math/Vector.h"

// NOTE (ismail): may be use separate x, y, z for positions and normals, and x, y for textures coordinat
// we need to chek will it increase performance
struct ObjFile {
    Vec3*   Positions;
    Vec2*   TextureCoord;
    u32*    Indices;
    u32     PositionsCount;
    u32     TexturesCount;
    u32     IndicesCount;
};

struct TextureFile {
    i32     Width;
    i32     Height;
    i32     BitsPerPixel;
    void*   Internal;
};

void AssetsLoaderInit();

// NOTE (ismail): for now in File variable we must store actual buffers outside of function
Statuses LoadObjFile(const char *Path, ObjFile *ReadedFile);

Statuses LoadTextureFile(const char *Path, TextureFile *ReadedFile);
void FreeTextureFile(TextureFile *ReadedFile);

#endif