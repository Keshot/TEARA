#include "AssetsLoader.h"
#include "Debug.h"
#include "TLib/3rdparty/stb/stb_image.h"
#include "TLib/3rdparty/fastobj/fast_obj.h"

#include <string.h>

#ifndef TEARA_MAXIMUM_NEIGHBOURS
    #define TEARA_MAXIMUM_NEIGHBOURS (20)
#endif

struct MeshCacheData {
    u32 TextureIndex[TEARA_MAXIMUM_NEIGHBOURS];
    u32 NormalIndex[TEARA_MAXIMUM_NEIGHBOURS];
    u32 RealIndex[TEARA_MAXIMUM_NEIGHBOURS];
    u32 Amount;
};

struct MeshCache {
    MeshCacheData*  Cache;
    u32             CacheSize;
};

MeshCache MeshLoadCache;

static inline bool32 FindInCacheArray(u32 *Array, u32 ArraySize, u32 FindingIndex, u32 *RealIndex)
{
    for (u32 Index = 0; Index < ArraySize; ++Index) {
        if (Array[Index] == FindingIndex) {
            *RealIndex = Index;
            return 1;
        }
    }
    return 0;
}

static bool32 FindInCache(MeshCache *ThreadCache, u32 PosIndex, u32 TextureIndex, u32 NormalIndex, u32 *ResultIndex)
{
    Assert(ThreadCache->CacheSize > PosIndex);
    
    u32*            CurrentArray;
    bool32          IndexFound  = 0;
    u32             FoundIndex  = 0;
    MeshCacheData*  CacheData   = &ThreadCache->Cache[PosIndex - 1]; // NOTE (ismail): -1 because position index starts from 1

    if (CacheData->Amount == 0) {
        return 0;
    }

    if (TextureIndex > 0) {
        IndexFound = FindInCacheArray(CacheData->TextureIndex, CacheData->Amount, TextureIndex, &FoundIndex);
        if (!IndexFound) {
            return 0;
        }
    }

    if (NormalIndex > 0) {
        if (!IndexFound) {
            IndexFound = FindInCacheArray(CacheData->NormalIndex, CacheData->Amount, NormalIndex, &FoundIndex);

            if (!IndexFound) {
                return 0;
            }
        }
        else if (CacheData->NormalIndex[FoundIndex] != NormalIndex) {
            return 0;
        }
    }

    *ResultIndex = CacheData->RealIndex[FoundIndex];

    return 1;
}

static inline void AddInCache(MeshCache *ThreadCache, u32 PosIndex, u32 TextureIndex, u32 NormalIndex, u32 RealIndex)
{
    Assert(ThreadCache->CacheSize > (PosIndex - 1));
    MeshCacheData *CacheData = &ThreadCache->Cache[PosIndex - 1];

    Assert(CacheData->Amount < TEARA_MAXIMUM_NEIGHBOURS);
    
    CacheData->TextureIndex[CacheData->Amount]  = TextureIndex;
    CacheData->NormalIndex[CacheData->Amount]   = NormalIndex;
    CacheData->RealIndex[CacheData->Amount]     = RealIndex;
    
    ++CacheData->Amount;
}

static inline void RenewCache(MeshCache *ThreadCache)
{
    memset((void*)ThreadCache->Cache, 0, ThreadCache->CacheSize);
}

void AssetsLoaderInit(EnginePlatform *PlatformContext, AssetsLoaderVars *LoaderVars)
{
    stbi_set_flip_vertically_on_load_thread(1);

    MeshLoadCache.Cache     = (MeshCacheData*)PlatformContext->AllocMem(sizeof(MeshCacheData) * LoaderVars->AssetsLoaderCacheSize);
    MeshLoadCache.CacheSize = LoaderVars->AssetsLoaderCacheSize;

    RenewCache(&MeshLoadCache);
}

// NOTE (ismail): for now in File variable we must store actual buffers outside of function
Statuses LoadObjFile(const char *Path, ObjFile *File)
{
    // NOTE (ismail): need to hug Alina 
    // NOTE (ismail): for now we use fast_obj in oreder to read .obj but in future i prefere to use own .obj parser
    // fast_obj set in every buffer (position, textcoord, normals) a dummy entry so we skip first elemen of every array
    u32     PosCount        = 0;
    u32     NormCount       = 0;
    u32     TexCoordCount   = 0;
    u32     IndicesCount    = 0;

    Vec3 *Pos       = File->Positions;
    Vec3 *Normals   = File->Normals;
    Vec2 *TexCoord  = File->TextureCoord;
    u32  *Indices   = File->Indices;

    fastObjMesh *LoadedMesh = fast_obj_read(Path);

    if (!LoadedMesh) {
        // TODO (ismail): diagnostic and write to log some info
        Assert(LoadedMesh);
        return Statuses::FileLoadFailed;
    }

    real32 *LoadPos             = LoadedMesh->positions;
    real32 *LoadNormals         = LoadedMesh->normals;
    real32 *LoadTexCoord        = LoadedMesh->texcoords;

    fastObjIndex *LoadIndices   = LoadedMesh->indices;

    for (u32 Index = 0; Index < LoadedMesh->index_count; ++Index) {
        u32 PosIndex        = LoadIndices[Index].p;
        u32 TextIndex       = LoadIndices[Index].t;
        u32 NormalsIndex    = LoadIndices[Index].n;
        u32 FoundIndex;

        bool32 Found = FindInCache(&MeshLoadCache, PosIndex, TextIndex, NormalsIndex, &FoundIndex);

        if (Found) {
            Indices[IndicesCount++] = FoundIndex;
        }
        else {
            Indices[IndicesCount++]     = PosCount;
            Pos[PosCount++]             = *((Vec3*)LoadPos + PosIndex);

            if (NormalsIndex > 0) { // NOTE (ismail): quick fix but later i need make up something
                Normals[NormCount++]   = *((Vec3*)LoadNormals + NormCount);   
            }

            if (TextIndex > 0) { // NOTE (ismail): quick fix but later i need make up something
                TexCoord[TexCoordCount++]   = *((Vec2*)LoadTexCoord + TextIndex);   
            }

            AddInCache(&MeshLoadCache, PosIndex, TextIndex, NormalsIndex, Index);
        }
    }

    fast_obj_destroy(LoadedMesh);

    File->IndicesCount      = IndicesCount;
    File->PositionsCount    = PosCount;
    File->NormalsCount      = NormCount;
    File->TexturesCount     = TexCoordCount;

    RenewCache(&MeshLoadCache);

    return Statuses::Success;
}

Statuses LoadTextureFile(const char *Path, TextureFile *ReadedFile)
{
    unsigned char* ImageData = stbi_load(Path, &ReadedFile->Width, &ReadedFile->Height, &ReadedFile->BitsPerPixel, 0);

    if (!ImageData) {
        // TODO (ismail): diagnostic and write to log some info
        Assert(ImageData);
        return Statuses::FileLoadFailed;
    }

    ReadedFile->Internal = ImageData;

    return Statuses::Success;
}

void FreeTextureFile(TextureFile *ReadedFile)
{
    stbi_image_free(ReadedFile->Internal);
}

// TODO (ismail): function must return value in order to detect wrong file
// NOTE (ismail): it is my simple obj parser but for more advanced files i use 3rd party library
/*
static void ParseObj(const char *Data, ObjFile *LoadedFile)
{
    // TODO (ismail): maybe change SDL_function on my own?
    i64 IndexArraySize = 0;
    u32 *IndexArray = LoadedFile->Indices;
    i64 VertexArraySize = 0;
    Vertex *VertexArray = LoadedFile->Vertices;

    for (;;) {
        char Sym = *Data;

        if (Sym == 0) {
            break;
        }

        switch (Sym) {
            case 's':
            case 'o':
            case '#': {
                const char *CommentStart = (Data + 2);
                for (; *CommentStart != '\n'; ++CommentStart); // TODO (ismail): case when sym == 0
                Data = CommentStart + 1; // NOTE (ismail): in order to skip current '\n'
            } break;
            case 'v': {
                Vec3 VertexStorage = {};
                i8 VertexIndex = 0;
                u8 Finish = 0;

                for (const char *VertexStart = (Data + 2); !Finish; ++VertexStart) {
                    char InnerSym = *VertexStart;
                    
                    switch (InnerSym) {
                        case ' ':
                        case 'v': {
                            continue;
                        } break;
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                        case '-': {
                            // TODO (ismail): VertexIndex > 3?
                            VertexStorage[VertexIndex++] = Atof(VertexStart);
                            for (const char *FetchToNextValue = VertexStart;; ++FetchToNextValue) {
                                InnerSym = *FetchToNextValue;

                                if (InnerSym == ' ') {
                                    VertexStart = FetchToNextValue;
                                    break;
                                }
                                else if (InnerSym == '\n') {
                                    VertexArray[VertexArraySize++].Position = VertexStorage;
                                    VertexIndex = 0;
                                    VertexStart = FetchToNextValue;
                                    break;
                                }
                                else if (InnerSym == '\0') {
                                    goto end;
                                }
                            }
                        } break;
                        case '\0': {
                            VertexArray[VertexArraySize++].Position = VertexStorage;
                            goto end;
                        } break;
                        default: {
                            Data = VertexStart;
                            Finish = 1;
                        } break;
                    }
                }
            } break;
            case 'f': {
                u8 Finish = 0;
                
                for (const char *FaceStart = (Data + 2); !Finish;) {
                    char InnerSym = *FaceStart;

                    if (isdigit(InnerSym)) {
                        IndexArray[IndexArraySize++] = Atoi((const char*)FaceStart) - 1;
                        ++FaceStart;
                        for (; isdigit(*FaceStart); ++FaceStart);

                        continue;
                    }

                    switch (InnerSym) {
                        case '\n': 
                        case ' ': 
                        case 'f': {
                            ++FaceStart;
                        } break;
                        case '\0': {
                            goto end;
                        } break;
                        default : {
                            Data = FaceStart;
                            Finish = 1;
                        }
                    }
                }
            } break;
            default: {
                ++Data;
            }
        }
    }

end:
    LoadedFile->VertexArraySize = VertexArraySize;
    LoadedFile->IndexArraySize = IndexArraySize;
}
*/