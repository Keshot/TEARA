#include "AssetsLoader.h"
#include "Debug.h"
#include "TLib/3rdparty/stb/stb_image.h"
#include "TLib/3rdparty/fastobj/fast_obj.h"

struct MeshCache {
    u32 TextureIndex;
    u32 ActualIndex;
};

void AssetsLoaderInit()
{
    stbi_set_flip_vertically_on_load_thread(1);
}

// NOTE (ismail): for now in File variable we must store actual buffers outside of function
Statuses LoadObjFile(const char *Path, ObjFile *File)
{
    MeshCache Cache[30000] = {};
    // NOTE (ismail): for now we use fast_obj in oreder to read .obj but in future i prefere to use own .obj parser
    // fast_obj set in every buffer (position, textcoord, normals) a dummy entry so we skip first elemen of every array
    u32     PosCount        = 0;
    u32     TexCoordCount   = 0;
    u32     IndicesCount    = 0;

    Vec3 *Pos       = File->Positions;
    Vec2 *TexCoord  = File->TextureCoord;
    u32  *Indices   = File->Indices;

    fastObjMesh *LoadedMesh = fast_obj_read(Path);

    if (!LoadedMesh) {
        // TODO (ismail): diagnostic and write to log some info
        Assert(LoadedMesh);
        return Statuses::FileLoadFailed;
    }

    real32 *LoadPos             = LoadedMesh->positions;
    real32 *LoadTexCoord        = LoadedMesh->texcoords;
    fastObjIndex *LoadIndices   = LoadedMesh->indices;

    for (u32 Index = 0; Index < LoadedMesh->index_count; ++Index) {
        MeshCache CacheElem = Cache[LoadIndices[Index].p];

        if (CacheElem.ActualIndex != 0 && CacheElem.TextureIndex == LoadIndices[Index].t) {
            Indices[Index] = CacheElem.ActualIndex;
        }
        else {
            Pos[PosCount++]             = *((Vec3*)(LoadPos + LoadIndices[Index].p));
            TexCoord[TexCoordCount++]   = *((Vec2*)(LoadTexCoord + LoadIndices[Index].t));
            Indices[IndicesCount++]     = Index;

            Cache[LoadIndices[Index].p].ActualIndex     = Index;
            Cache[LoadIndices[Index].p].TextureIndex    = LoadIndices[Index].t;
        }
    }

    fast_obj_destroy(LoadedMesh);

    File->IndicesCount      = IndicesCount;
    File->PositionsCount    = PosCount;
    File->TexturesCount     = TexCoordCount;

    return Statuses::Success;
}

Statuses LoadTextureFile(const char *Path, TextureFile *ReadedFile)
{
    unsigned char* ImageData = stbi_load(Path, &ReadedFile->Width, &ReadedFile->Height, &ReadedFile->BitsPerPixel, 0);

    if (!ImageData) {
        // TODO (ismail): diagnostic and write to log some info
        Assert(image_data);
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