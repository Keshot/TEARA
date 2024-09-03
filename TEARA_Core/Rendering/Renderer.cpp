#include "TEARA_Core/OpenGL/WinOpenGL.h"
#include "TEARA_Lib/Math/Vector.h"
#include "TEARA_Core/Engine.h"

#define MAX_SHADERS (0x0A)

enum RendererInitErrors {
    Success = 0,
};

struct Vertex {
    Vec3 Position;
};

// TODO (ismail:) separate Vertex and ObjFile
struct ObjFile {
    Vertex  Vertices[10000];
    i64     VertexArraySize;
    u32     Indices[30000];
    i64     IndexArraySize;
};

struct OpenGLBuffers {
    u32     VAO;
    u32     VBO;
    u32     IBO;
};

struct ObjectRenderingContext {
    OpenGLBuffers   Buffers;
    ObjFile         ObjectFile;
    i32             ShaderProgramIndex; 
};

enum ShaderUniformType {
    Matrix4f,
    Vector3f,
    Max // NOTE (ismail): last element!
};

struct Shader {
    ShaderUniformType   UniformType;
    i32                 Location;
};

struct ShaderProgram {
    u32     ProgramHandle;
    i32     ShadersAmount;
    Shader  Shaders[MAX_SHADERS];
};

// TODO (ismail): function must return value in order to detect wrong file
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
                            VertexStorage[VertexIndex++] = SDL_atof(VertexStart);
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

                    if (SDL_isdigit(InnerSym)) {
                        IndexArray[IndexArraySize++] = SDL_atoi((const char*)FaceStart) - 1;
                        ++FaceStart;
                        for (; SDL_isdigit(*FaceStart); ++FaceStart);

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

// TODO (ismail): move it to another file and optimize
static i32 LoadObjFile(const char *Path, ObjFile *LoadedFile)
{
    File FileDataTmp = LoadFile(Path);
    
    if (!FileDataTmp.Data || FileDataTmp.Size > 30000) {
        // TODO (ismail): diagnostic
        abort();
    }

    ParseObj((const char*)FileDataTmp.Data, LoadedFile);
    // TODO (ismail): diagnostic

    FreeFileMemory(&FileDataTmp);

    return 0;
}

/*
GLuint LoadTexture(const char *FileName)
{
    GLuint TextureObject;
    int u, v, bpp;
    u8 *ImageData = stbi_load(FileName, &u, &v, &bpp, 0);

    if (!ImageData) {
        // TODO (ismail): diagnostic?
        return 0;
    }

    glGenTextures(1, &TextureObject);
    glBindTexture(GL_TEXTURE_2D, TextureObject);
    // TODO (ismail): read about mip mapping
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, u, v, 0, GL_RGB, GL_UNSIGNED_BYTE, ImageData);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(ImageData);

    return TextureObject;
}
*/

static RendererInitErrors RendererInit()
{
    GLclampf Red = 0.f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f;
    GLclampf ColorValueMask = 1.0f / 256.0f;

    // NOTE (ismail): set color values for future call of glClear
    glClearColor(Red * ColorValueMask, Green * ColorValueMask, Blue * ColorValueMask, Alpha);

    // TODO (ismail): learn this staff glEnable(GL_CULL_FACE)
    // TODO (ismail): learn this staff glFrontFace(GL_CW)
    // TODO (ismail): learn this staff glCullFace
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);

    // NOTE (ismail): wireframe mode = GL_LINE, polygon mode = GL_FILL
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_DEPTH_TEST);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return RendererInitErrors::Success;
}

static bool32 AttachShader(GLuint ShaderHandle, const char *ShaderCode, GLint Length, GLenum ShaderType)
{
    GLuint Shader = tglCreateShader(ShaderType);
    if (!Shader) {
        // TODO (ismail): diagnostic
        return 0; // TODO (ismail): put here actual value
    }

    tglShaderSource(Shader, 1, &ShaderCode, &Length);
    tglCompileShader(Shader);

    GLint Success;
    tglGetShaderiv(Shader, GL_COMPILE_STATUS, &Success);

    if (!Success) {
        // TODO (ismail): handle this case with glGetShaderInfoLog and print it in log
        return 0;
    }

    tglAttachShader(ShaderHandle, Shader);

    return 1;
}

static void LoadObjectToHardware(OpenGLBuffers *RenderContext, ObjFile *ObjectData)
{
    tglGenVertexArrays(1, &(RenderContext->VAO));
    tglBindVertexArray(RenderContext->VAO);

    tglGenBuffers(1, &(RenderContext->VBO));
    tglBindBuffer(GL_ARRAY_BUFFER, RenderContext->VBO);
    tglBufferData(GL_ARRAY_BUFFER, sizeof(*ObjectData->Vertices) * ObjectData->VertexArraySize, ObjectData->Vertices, GL_STATIC_DRAW);

    tglGenBuffers(1, &(RenderContext->IBO));
    tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RenderContext->IBO);
    tglBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*ObjectData->Indices) * ObjectData->IndexArraySize, ObjectData->Indices, GL_STATIC_DRAW);

    // position
    tglEnableVertexAttribArray(0);        
    tglVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(*ObjectData->Vertices), 0);

    // NOTE (ismail): for now i disable this, because i will be put color directly in shader for now
    // color
    // glEnableVertexAttribArray(1);
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(GLfloat)));
    
    tglBindVertexArray(0);
    tglDisableVertexAttribArray(0);
    // glDisableVertexAttribArray(1);
    tglBindBuffer(GL_ARRAY_BUFFER, 0);
    tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}