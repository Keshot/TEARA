#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_stdinc.h>
#include <stb/stb_image.h>

#include "Types.h"
#include "MatrixTransform.h"
#include "CoreMath.h"

#define FILE_READ_BUFFER_LEN 5000
#define MOUSE_EVENT(type) ((type == SDL_EVENT_MOUSE_MOTION)||(type == SDL_EVENT_MOUSE_BUTTON_UP)||(type == SDL_EVENT_MOUSE_BUTTON_DOWN))
#define AABB_TEST_EPSILON 0.0001f

const Mat4x4 Identity4x4 = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

const Mat3x3 Identity3x3 = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};

struct Vertex {
    Vec3 Position;
};

struct ObjFile {
    Vertex      *Vertices;
    i64          VertexArraySize;
    u32         *Indices;
    i64          IndexArraySize;
};

struct OpenGLObjectContext {
    GLuint VAO;
    GLuint VBO;
    GLuint IBO;
};

struct AABB {
    Vec3 Center;
    Vec3 Extens; // Rx, Ry, Rz radius of halfwidth
};

struct Sphere {
    Vec3    Center;
    real32  Radius;
};

struct WorldTransform {
    Vec3        Position;
    Rotation    Rotate;
    Vec3        Scale;
};

struct WorldObjectAABBCollider {
    WorldTransform          Transform;
    AABB                    BoundingBox;
};

struct WorldObjectSphereCollider {
    WorldTransform          Transform;
    Sphere                  BoundingSphere;
};

struct Camera {
    WorldTransform Transform;
};

struct FileData {
    char Data[FILE_READ_BUFFER_LEN];
    u64 DataLength;
};

enum {
    SUCCESS = 0,
    SDL_INIT_FAILURE = -1,
    SDL_CREATE_WINDOW_FAILURE = -2,
    SDL_OPENGL_LOAD_FAILURE = -3,
    SDL_OPENGL_CREATE_CONTEXT_FAILURE = -4,
    OPENGL_ERROR = -5,
    SDL_ERROR = -6,

    OVERLAP = 11,
    SEPARATE = -11,

    // TODO (ismail): make these configurable
    WINDOW_WIDTH = 1600,
    WINDOW_HEIGHT = 900
};

static real32 AspectRatio = (real32)(WINDOW_WIDTH) / (real32)(WINDOW_HEIGHT);

// TODO (ismail): reading this options from command arguments
static const SDL_InitFlags SDLInitFlags = SDL_INIT_VIDEO;

// TODO (ismail): reading this options from command arguments or from file or from in game settings
static const SDL_WindowFlags WindowFlags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;

// glGenBuffers
static PFNGLGENBUFFERSPROC                  glGenBuffers;
static PFNGLBINDBUFFERPROC                  glBindBuffer;
static PFNGLBUFFERDATAPROC                  glBufferData;
static PFNGLENABLEVERTEXATTRIBARRAYPROC     glEnableVertexAttribArray;
static PFNGLVERTEXATTRIBPOINTERPROC         glVertexAttribPointer;
static PFNGLDISABLEVERTEXATTRIBARRAYPROC    glDisableVertexAttribArray;
static PFNGLCREATEPROGRAMPROC               glCreateProgram;
static PFNGLCREATESHADERPROC                glCreateShader;
static PFNGLSHADERSOURCEPROC                glShaderSource;
static PFNGLCOMPILESHADERPROC               glCompileShader;
static PFNGLGETSHADERIVPROC                 glGetShaderiv;
static PFNGLATTACHSHADERPROC                glAttachShader;
static PFNGLLINKPROGRAMPROC                 glLinkProgram;
static PFNGLGETPROGRAMIVPROC                glGetProgramiv;
static PFNGLVALIDATEPROGRAMPROC             glValidateProgram;
static PFNGLUSEPROGRAMPROC                  glUseProgram;
static PFNGLGETPROGRAMINFOLOGPROC           glGetProgramInfoLog;
static PFNGLGETUNIFORMLOCATIONPROC          glGetUniformLocation;
static PFNGLUNIFORM1FPROC                   glUniform1f;
static PFNGLUNIFORMMATRIX4FVPROC            glUniformMatrix4fv;
static PFNGLUNIFORM1IPROC                   glUniform1i;
static PFNGLACTIVETEXTUREPROC               glActiveTextureStb;
static PFNGLGENVERTEXARRAYSPROC             glGenVertexArrays;
static PFNGLBINDVERTEXARRAYPROC             glBindVertexArray;
static PFNGLUNIFORM3FVPROC                  glUniform3fv;

#define glActiveTexture glActiveTextureStb

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
    LoadedFile->Vertices = VertexArray;
    LoadedFile->VertexArraySize = VertexArraySize;
    LoadedFile->Indices = IndexArray;
    LoadedFile->IndexArraySize = IndexArraySize;
}

// TODO (ismail): move it to another file and optimize
static i32 LoadObjFile(const char *Path, ObjFile *LoadedFile)
{
    char DataBuffer[30000] = { };
    SDL_IOStream *DataStream = SDL_IOFromFile(Path, "r");
    
    if (!DataStream) {
        // TODO (ismail): diagnostic
        return SDL_ERROR;
    }

    u64 ReadedSize = SDL_ReadIO(DataStream, DataBuffer, 30000);
    // TODO (ismail): diagnostic
    if (ReadedSize == 30000) {
        // TODO (ismail): crutch for now but later i need to remove this
        abort();
    }

    ParseObj(DataBuffer, LoadedFile);
    // TODO (ismail): diagnostic

    SDL_CloseIO(DataStream); // TODO (ismail): ignoring of the result

    return SUCCESS;
}

void AABBRecalculate(Mat4x4 *Rotation, Vec3 *Translation, AABB *Original, AABB *Result)
{
    // NOTE (ismail): for now aabb center is always x = 0, y = 0, z = 0, fix it in future
    Result->Center.x = Translation->x;
    Result->Center.y = Translation->y;
    Result->Center.z = Translation->z;

    for (i32 i = 0; i < 3; ++i) {
        Result->Extens[i] = 0.0f;

        for (i32 j = 0; j < 3; ++j) {
            Result->Extens[i] += Fabs((*Rotation)[i][j]) * Original->Extens[j];
        }
    }
}

bool32 AABBToAABBTestOverlap(AABB *A, AABB *B)
{
    // TODO (ismail): convert to SIMD
    // TODO (ismail): may be calculate A.Center - B.Center one time and use it?
    
    if (SDL_fabsf(A->Center.x - B->Center.x) > (A->Extens.x + B->Extens.x)) {
        return 0;
    }
    if (SDL_fabsf(A->Center.y - B->Center.y) > (A->Extens.y + B->Extens.y)) {
        return 0;
    }
    if (SDL_fabsf(A->Center.z - B->Center.z) > (A->Extens.z + B->Extens.z)) {
        return 0;
    }

    return 1;
}

bool32 SphereToAABBTestOverlap(AABB *A, Sphere *B)
{
    Vec3 DistanceVector = A->Center - B->Center;
    real32 DistanceSquare = DotProduct(DistanceVector, DistanceVector);

    real32 RadiusSumX = A->Extens.x + B->Radius;
    real32 RadiusSumY = A->Extens.y + B->Radius;
    real32 RadiusSumZ = A->Extens.z + B->Radius;

    if (DistanceSquare > SQUARE(RadiusSumX)) {
        return 0;
    }
    if (DistanceSquare > SQUARE(RadiusSumY)) {
        return 0;
    }
    if (DistanceSquare > SQUARE(RadiusSumZ)) {
        return 0;
    }

    return 1;
}

bool32 SphereToSphereTestOverlap(Sphere *A, Sphere *B)
{
    Vec3 DistanceVector = A->Center - B->Center;
    real32 DistanceSquare = DotProduct(DistanceVector, DistanceVector);

    real32 RadiusSum = A->Radius + B->Radius;
    real32 RadiusSquare = SQUARE(RadiusSum);

    return DistanceSquare < RadiusSquare;
}

static i32 ReadFile(const char *Path, FileData *Buffer)
{
    SDL_IOStream *DataStream = SDL_IOFromFile(Path, "r");
    if (!DataStream) {
        // TODO (ismail): diagnostic
        return SDL_ERROR;
    }

    Buffer->DataLength = SDL_ReadIO(DataStream, Buffer->Data, FILE_READ_BUFFER_LEN);
    // TODO (ismail): diagnostic

    SDL_CloseIO(DataStream); // TODO (ismail): ignoring of the result

    return SUCCESS;
}

static i32 LoadGlFunctions()
{
    glGenBuffers = (PFNGLGENBUFFERSPROC) SDL_GL_GetProcAddress("glGenBuffers");
    if (!glGenBuffers) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glBindBuffer = (PFNGLBINDBUFFERPROC) SDL_GL_GetProcAddress("glBindBuffer");
    if (!glBindBuffer) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glBufferData = (PFNGLBUFFERDATAPROC) SDL_GL_GetProcAddress("glBufferData");
    if (!glBufferData) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) SDL_GL_GetProcAddress("glEnableVertexAttribArray");
    if (!glEnableVertexAttribArray) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) SDL_GL_GetProcAddress("glVertexAttribPointer");
    if (!glVertexAttribPointer) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) SDL_GL_GetProcAddress("glDisableVertexAttribArray");
    if (!glDisableVertexAttribArray) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glCreateProgram = (PFNGLCREATEPROGRAMPROC) SDL_GL_GetProcAddress("glCreateProgram");
    if (!glCreateProgram) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glCreateShader = (PFNGLCREATESHADERPROC) SDL_GL_GetProcAddress("glCreateShader");
    if (!glCreateShader) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glShaderSource = (PFNGLSHADERSOURCEPROC) SDL_GL_GetProcAddress("glShaderSource");
    if (!glShaderSource) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glCompileShader = (PFNGLCOMPILESHADERPROC) SDL_GL_GetProcAddress("glCompileShader");
    if (!glCompileShader) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glGetShaderiv = (PFNGLGETSHADERIVPROC) SDL_GL_GetProcAddress("glGetShaderiv");
    if (!glGetShaderiv) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glAttachShader = (PFNGLATTACHSHADERPROC) SDL_GL_GetProcAddress("glAttachShader");
    if (!glAttachShader) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glLinkProgram = (PFNGLLINKPROGRAMPROC) SDL_GL_GetProcAddress("glLinkProgram");
    if (!glLinkProgram) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glGetProgramiv = (PFNGLGETPROGRAMIVPROC) SDL_GL_GetProcAddress("glGetProgramiv");
    if (!glGetProgramiv) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glValidateProgram = (PFNGLVALIDATEPROGRAMPROC) SDL_GL_GetProcAddress("glValidateProgram");
    if (!glValidateProgram) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glUseProgram = (PFNGLUSEPROGRAMPROC) SDL_GL_GetProcAddress("glUseProgram");
    if (!glUseProgram) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) SDL_GL_GetProcAddress("glGetProgramInfoLog");
    if (!glGetProgramInfoLog) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) SDL_GL_GetProcAddress("glGetUniformLocation"); 
    if (!glGetUniformLocation) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glUniform1f = (PFNGLUNIFORM1FPROC) SDL_GL_GetProcAddress("glUniform1f");
    if (!glUniform1f) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) SDL_GL_GetProcAddress("glUniformMatrix4fv");
    if (!glUniformMatrix4fv) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glUniform1i = (PFNGLUNIFORM1IPROC) SDL_GL_GetProcAddress("glUniform1i");
    if (!glUniform1i) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glActiveTexture = (PFNGLACTIVETEXTUREPROC) SDL_GL_GetProcAddress("glActiveTexture");
    if (!glActiveTexture) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) SDL_GL_GetProcAddress("glGenVertexArrays");
    if (!glGenVertexArrays) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC) SDL_GL_GetProcAddress("glBindVertexArray");
    if (!glGenVertexArrays) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    glUniform3fv = (PFNGLUNIFORM3FVPROC) SDL_GL_GetProcAddress("glUniform3fv");
    if (!glUniform3fv) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    return SUCCESS;
}

static bool32 AttachShader(GLuint ShaderHandle, const char *ShaderCode, GLint Length, GLenum ShaderType)
{
    GLuint Shader = glCreateShader(ShaderType);
    if (!Shader) {
        // TODO (ismail): diagnostic
        return 0; // TODO (ismail): put here actual value
    }

    glShaderSource(Shader, 1, &ShaderCode, &Length);
    glCompileShader(Shader);

    GLint Success;
    glGetShaderiv(Shader, GL_COMPILE_STATUS, &Success);

    if (!Success) {
        // TODO (ismail): handle this case with glGetShaderInfoLog and print it in log
        return 0;
    }

    glAttachShader(ShaderHandle, Shader);

    return 1;
}

static void CreateCubeObject(OpenGLObjectContext *RenderContext, ObjFile *ObjectData)
{
    glGenVertexArrays(1, &(RenderContext->VAO));
    glBindVertexArray(RenderContext->VAO);

    glGenBuffers(1, &(RenderContext->VBO));
    glBindBuffer(GL_ARRAY_BUFFER, RenderContext->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(*ObjectData->Vertices) * ObjectData->VertexArraySize, ObjectData->Vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &(RenderContext->IBO));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RenderContext->IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*ObjectData->Indices) * ObjectData->IndexArraySize, ObjectData->Indices, GL_STATIC_DRAW);

    // position
    glEnableVertexAttribArray(0);        
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(*ObjectData->Vertices), 0);

    // NOTE (ismail): for now i disable this, because i will be put color directly in shader for now
    // color
    // glEnableVertexAttribArray(1);
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(GLfloat)));
    
    glBindVertexArray(0);
    glDisableVertexAttribArray(0);
    // glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

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

int main(int argc, char *argv[])
{
    //TODO (ismail): read width/height from command arguments, from file, from in game settings.
    SDL_Event Event;
    SDL_Window *Window;
    SDL_Surface *WindowScreenSurface;
    SDL_GLContext GlContext;
    bool32 Quit = false;

    stbi_set_flip_vertically_on_load_thread(1);

    if (SDL_Init(SDLInitFlags) < 0) {
        // TODO (ismail): logging
        return SDL_INIT_FAILURE;
    }

    // TODO (ismail): move all of this staff into separate functions and we must create windows and then load library!!!
    if (SDL_GL_LoadLibrary("opengl32.dll") < 0) {
        // TODO (ismail): diagnostic
        return SDL_OPENGL_LOAD_FAILURE;
    }

    // TODO (ismail): check available opengl version
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    // NOTE (ismail): it must be 24 if not opengl will be intialized with 1.1.0 version
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    Window = SDL_CreateWindow("TEARA", WINDOW_WIDTH, WINDOW_HEIGHT, WindowFlags);
    if (!Window) {
        // TODO (ismail): logging
        return SDL_CREATE_WINDOW_FAILURE;
    }

    GlContext = SDL_GL_CreateContext(Window);
    if (!GlContext) {
        const char *error = SDL_GetError();
        return SDL_OPENGL_CREATE_CONTEXT_FAILURE;
    }

    // TODO (ismail): check value
    bool32 Val = SDL_GL_MakeCurrent(Window, GlContext);
    if (!Val) {

    }

    i32 LoadFunctionsResult;
    if ((LoadFunctionsResult = LoadGlFunctions())) {
        // TODO (ismail): diagnostic
        return LoadFunctionsResult;
    }

    GLclampf Red = 0.0f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f;
    GLclampf ColorValueMask = 1.0f / 256.0f;

    // NOTE (ismail): set color values for future call of glClear
    glClearColor(Red * ColorValueMask, Green * ColorValueMask, Blue * ColorValueMask, Alpha);
    glClear(GL_COLOR_BUFFER_BIT);

    // TODO (ismail): learn this staff glEnable(GL_CULL_FACE)
    // TODO (ismail): learn this staff glFrontFace(GL_CW)
    // TODO (ismail): learn this staff glCullFace

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);

    u32 IndexBuffer1[4000];
    Vertex VertexBuffer1[4000];

    ObjFile CubeFile = { };
    CubeFile.Indices = IndexBuffer1;
    CubeFile.Vertices = VertexBuffer1;

    LoadObjFile("cube.obj", &CubeFile);

    OpenGLObjectContext CubeObjectRenderContext;
    CreateCubeObject(&CubeObjectRenderContext, &CubeFile);

    u32 IndexBuffer2[4000];
    Vertex VertexBuffer2[4000];
    
    ObjFile SphereFile = { };
    SphereFile.Indices = IndexBuffer2;
    SphereFile.Vertices = VertexBuffer2;

    LoadObjFile("sphere.obj", &SphereFile);

    OpenGLObjectContext SphereObjectRenderContext;
    CreateCubeObject(&SphereObjectRenderContext, &SphereFile);
    
    // NOTE (ismail): i disable textures for now that call need for load texture
    // GLuint TextureObject = LoadTexture("bricks_textures.jpg");
    // NOTE (ismail): these calls need later for put it in screen
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, TextureObject);
    // glUniform1i(SamplerLocation, 0);

    GLuint ShaderProgram = glCreateProgram();
    if (!ShaderProgram) {
        // TODO (ismail): diagnostic
        return 0; // TODO (ismail): put here actual value
    }

    FileData VertexShader = {  }, FragmentShader = {  };

    if (ReadFile("../vertex_color.vs", &VertexShader) != SUCCESS) {
        // TODO (ismail): diagnostic
        return SDL_ERROR;
    }

    if (ReadFile("../fragment_color.fs", &FragmentShader) != SUCCESS) {
        // TODO (ismail): diagnostic
        return SDL_ERROR;
    }

    if (!AttachShader(ShaderProgram, VertexShader.Data, VertexShader.DataLength, GL_VERTEX_SHADER)) {
        return SDL_INIT_FAILURE; // TODO (ismail): more complicated check here
    }

    if (!AttachShader(ShaderProgram, FragmentShader.Data, FragmentShader.DataLength, GL_FRAGMENT_SHADER)) {
        return SDL_INIT_FAILURE; // TODO (ismail): more complicated check here
    }

    GLchar ErrorLog[1024] = { 0 };
    glLinkProgram(ShaderProgram);

    GLint Success;
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
    if (!Success) {
        // TODO (ismail): handle this case with glGetShaderInfoLog and print it in log
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        return SDL_INIT_FAILURE;
    }

    GLint TransformLocation = glGetUniformLocation(ShaderProgram, "Transform");
    if (TransformLocation == -1) {
        // TODO (ismail): diagnostic
        return OPENGL_ERROR;
    }

    GLint MeshColorLocation = glGetUniformLocation(ShaderProgram, "MeshColor");
    if (MeshColorLocation == -1) {
        // TODO (ismail): diagnostic
        return OPENGL_ERROR;
    }

    //  GLint SamplerLocation = glGetUniformLocation(ShaderProgram, "Sampler");
    //  if (TransformLocation == -1) {
           // TODO (ismail): diagnostic
    //     return OPENGL_ERROR;
    //  }

    glValidateProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        // TODO (ismail): handle this case with glGetShaderInfoLog and print it in log
        return SDL_INIT_FAILURE;
    }

    glUseProgram(ShaderProgram);

    // GLfloat MaxRotation = DEGREE_TO_RAD(90.0f);
    // GLfloat MinRotation = MaxRotation * -1.0f;
    GLfloat RotationDeltaY = 0.0f;
    GLfloat AngelInRadY = 0.0f;

    GLfloat RotationDeltaX = 0.0f;
    GLfloat AngelInRadX = 0.0f;

    GLfloat MaxTranslation = 1.0f;
    GLfloat MinTranslation = -1.0f;
    GLfloat Translation = 0.0f;
    GLfloat TranslationDelta = 0.0001f;

    GLfloat MaxScale = 0.4f;
    GLfloat MinScale = 0.01f;
    GLfloat Scale = 0.2f;
    GLfloat ScaleDelta = 0.0001f;

    // NOTE (ismail): wireframe mode = GL_LINE, polygon mode = GL_FILL
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);

    Mat4x4 OrthoProjection = MakeOrtographProjection(-2.0f * AspectRatio, 2.0f * AspectRatio, -2.0f, 2.0f, 0.1f, 100.0f);
    Mat4x4 PerspectiveProjection = MakePerspectiveProjection(60.0f, AspectRatio, 0.01f, 50.0f);

    bool isPerspective = true;

    Mat4x4 Projection = isPerspective ? PerspectiveProjection : OrthoProjection;

    Camera PlayerCamera = {  };

    GLfloat Speed = 5.0f;

    GLfloat CameraYRotationDelta = 0.0f;
    GLfloat CameraXRotationDelta = 0.0f;

    GLfloat CameraTargetTranslationDelta = 0.0f;
    GLfloat CameraRightTranslationDelta = 0.0f;

    GLfloat PlayerTargetTranslationDelta = 0.0f;
    GLfloat PlayerRightTranslationDelta = 0.0f;

    // TODO (ismail): read what is this
    SDL_SetRelativeMouseMode(SDL_TRUE);

    // TODO (ismail): check what is that
    // glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);

    /*
    WorldObjectAABBCollider NpcCube = {};

    NpcCube.Transform.Position.x = -1.5f;
    NpcCube.Transform.Position.y = 0.0f;
    NpcCube.Transform.Position.z = 2.0f;

    NpcCube.Transform.Scale.x = 1.0f;
    NpcCube.Transform.Scale.y = 1.0f;
    NpcCube.Transform.Scale.z = 1.0f;

    NpcCube.BoundingBox.Extens.x = 1.0f; // TODO (ismail): initial AABB compute need to be implemented
    NpcCube.BoundingBox.Extens.y = 1.0f; // TODO (ismail): initial AABB compute need to be implemented
    NpcCube.BoundingBox.Extens.z = 1.0f; // TODO (ismail): initial AABB compute need to be implemented

    Vec3 NpcCubeColor = { 1.0f, 0.0f, 0.0f };
    */

    WorldObjectSphereCollider NpcSphere = {};

    NpcSphere.Transform.Position.x = -1.5f;
    NpcSphere.Transform.Position.y = 0.0f;
    NpcSphere.Transform.Position.z = 2.0f;

    NpcSphere.Transform.Scale.x = 1.0f;
    NpcSphere.Transform.Scale.y = 1.0f;
    NpcSphere.Transform.Scale.z = 1.0f;

    NpcSphere.BoundingSphere.Center = NpcSphere.Transform.Position; // TODO (ismail): initial Sphere compute need to be implemented
    NpcSphere.BoundingSphere.Radius = 1.0f; // TODO (ismail): initial Sphere compute need to be implemented

    Vec3 NpcSphereColor = { 1.0f, 0.0f, 0.0f };

    /*
    WorldObjectAABBCollider PlayerCube = {};

    PlayerCube.Transform.Position.x = 1.5f;
    PlayerCube.Transform.Position.y = 0.0f;
    PlayerCube.Transform.Position.z = 2.0f;

    PlayerCube.Transform.Scale.x = 1.0f;
    PlayerCube.Transform.Scale.y = 1.0f;
    PlayerCube.Transform.Scale.z = 1.0f;

    PlayerCube.BoundingBox.Extens.x = 1.0f; // TODO (ismail): initial AABB compute need to be implemented
    PlayerCube.BoundingBox.Extens.y = 1.0f; // TODO (ismail): initial AABB compute need to be implemented
    PlayerCube.BoundingBox.Extens.z = 1.0f; // TODO (ismail): initial AABB compute need to be implemented
    */

    WorldObjectSphereCollider PlayerSphere = {};

    PlayerSphere.Transform.Position.x = 1.5f;
    PlayerSphere.Transform.Position.y = 0.0f;
    PlayerSphere.Transform.Position.z = 2.0f;

    PlayerSphere.Transform.Scale.x = 1.0f;
    PlayerSphere.Transform.Scale.y = 1.0f;
    PlayerSphere.Transform.Scale.z = 1.0f;

    PlayerSphere.BoundingSphere.Center = PlayerSphere.Transform.Position; // TODO (ismail): initial Sphere compute need to be implemented
    PlayerSphere.BoundingSphere.Radius = 1.0f; // TODO (ismail): initial Sphere compute need to be implemented
    
    glBindVertexArray(SphereObjectRenderContext.VAO);

    while (!Quit) {
        Vec3 PlayerSphereColor = { 0.0f, 0.0f, 1.0f };
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Vec2 MouseMoution = { };

        while (SDL_PollEvent(&Event)) {
            if (Event.type == SDL_EVENT_QUIT) {
                Quit = true;
            }
            else if (Event.type == SDL_EVENT_KEY_DOWN) {
                switch (Event.key.key) {
                    // TODO (ismail): use raw key code
                    case SDLK_I: {
                        PlayerTargetTranslationDelta = 1.0f;
                    } break;
                    case SDLK_K: {
                        PlayerTargetTranslationDelta = -1.0f;
                    } break;
                    case SDLK_L: {
                        PlayerRightTranslationDelta = 1.0f;
                    } break;
                    case SDLK_J: {
                        PlayerRightTranslationDelta = -1.0f;
                    } break;
                    case SDLK_UP: {
                        CameraTargetTranslationDelta = 1.0f;
                    } break;
                    case SDLK_DOWN: {
                        CameraTargetTranslationDelta = -1.0f;
                    } break;
                    case SDLK_RIGHT: {
                         CameraRightTranslationDelta = 1.0f;
                    } break;
                    case SDLK_LEFT: {
                         CameraRightTranslationDelta = -1.0f;
                    } break;
                    case SDLK_Q: {
                        RotationDeltaY = -DEGREE_TO_RAD(0.03f);
                    } break;
                    case SDLK_E: {
                        RotationDeltaY = DEGREE_TO_RAD(0.03f);
                    } break;
                    case SDLK_W: {
                        RotationDeltaX = DEGREE_TO_RAD(0.03f);
                    } break;
                    case SDLK_S: {
                        RotationDeltaX = -DEGREE_TO_RAD(0.03f);
                    } break;
                    case SDLK_X: {
                        AngelInRadY = 0.0f;
                    } break;
                    case SDLK_C: {
                        AngelInRadX = 0.0f;
                    } break;
                    case SDLK_V: {
                        Projection = isPerspective ? OrthoProjection : PerspectiveProjection;
                        isPerspective = !isPerspective;
                    } break;
                    default: {
                    }
                }
            }
            else if (Event.type == SDL_EVENT_KEY_UP) {
                switch (Event.key.key) {
                    case SDLK_UP: {
                        CameraTargetTranslationDelta = 0.0f;
                    } break;
                    case SDLK_DOWN: {
                        CameraTargetTranslationDelta = 0.0f;
                    } break;
                    case SDLK_RIGHT: {
                         CameraRightTranslationDelta = 0.0f;
                    } break;
                    case SDLK_LEFT: {
                         CameraRightTranslationDelta = 0.0f;
                    } break;
                    case SDLK_I: {
                        PlayerTargetTranslationDelta = 0.0f;
                    } break;
                    case SDLK_K: {
                        PlayerTargetTranslationDelta = 0.0f;
                    } break;
                    case SDLK_L: {
                        PlayerRightTranslationDelta = 0.0f;
                    } break;
                    case SDLK_J: {
                        PlayerRightTranslationDelta = 0.0f;
                    } break;
                    case SDLK_Q: {
                        RotationDeltaY = 0.0f;
                    } break;
                    case SDLK_E: {
                        RotationDeltaY = 0.0f;
                    } break;
                    case SDLK_W: {
                        RotationDeltaX = 0.0f;
                    } break;
                    case SDLK_S: {
                        RotationDeltaX = 0.0f;
                    } break;
                    default: {

                    }
                }
            }
            else if (MOUSE_EVENT(Event.type)) {
                // TODO (ismail): check this
                SDL_GetRelativeMouseState(&MouseMoution.x, &MouseMoution.y);
                MouseMoution.Normalize();

                SDL_Log("Mouse X = %.2f\tMouse Y = %.2f\n", MouseMoution.x, MouseMoution.y);

                // TODO (ismail): check this
                SDL_WarpMouseInWindow(Window, (real32)(WINDOW_WIDTH / 2), (real32)(WINDOW_HEIGHT / 2));
            }
        }
        // --------------------------------OLD CODE--------------------------------------------
        // Translation += TranslationDelta;
        // Scale += ScaleDelta;
        // if ((Scale >= MaxScale) || (Scale <= MinScale)) {
        //     ScaleDelta *= -1.0f;
        // }
        // if ((Translation >= MaxTranslation) || (Translation <= MinTranslation)) {
        //    TranslationDelta *= -1.0f;
        // }
        // Vec3 ScaleVector = { 1.0f, 1.0f, 1.0f };
        // Mat4x4 Scale = MakeScale(&ScaleVector);
        // -------------------------------------------------------------------------------------

        PlayerSphere.Transform.Rotate.Heading += RotationDeltaY;
        PlayerSphere.Transform.Rotate.Pitch += RotationDeltaX;

        PlayerCamera.Transform.Rotate.Heading += CameraYRotationDelta + (MouseMoution.x * 0.005);
        PlayerCamera.Transform.Rotate.Pitch += CameraXRotationDelta + (MouseMoution.y * 0.005);

        Vec3 PlayerTarget = {};
        Vec3 PlayerRight = {};
        Vec3 PlayerUp = {};

        Mat4x4 PlayerRotation = MakeRotation4x4(&PlayerSphere.Transform.Rotate);
        RotationToVectors(&PlayerSphere.Transform.Rotate, &PlayerTarget, &PlayerRight, &PlayerUp);

        Vec3 PlayerTargetTranslation = { 
            PlayerTarget.x * (PlayerTargetTranslationDelta * Speed * 0.0001234f),
            PlayerTarget.y * (PlayerTargetTranslationDelta * Speed * 0.0001234f),
            PlayerTarget.z * (PlayerTargetTranslationDelta * Speed * 0.0001234f)
        };

        Vec3 PlayerRightTranslation = { 
            PlayerRight.x * (PlayerRightTranslationDelta * Speed * 0.0001234f),
            PlayerRight.y * (PlayerRightTranslationDelta * Speed * 0.0001234f),
            PlayerRight.z * (PlayerRightTranslationDelta * Speed * 0.0001234f)
        };

        Vec3 FinalPlayerTranslation = PlayerTargetTranslation + PlayerRightTranslation;
        PlayerSphere.Transform.Position += FinalPlayerTranslation;
        
        Mat4x4 PlayerTranslation = MakeTranslation(&PlayerSphere.Transform.Position);
        Mat4x4 PlayerScale = Identity4x4;

        Mat4x4 NpcRotation = MakeRotation4x4(&NpcSphere.Transform.Rotate);
        Mat4x4 NpcTranslation = MakeTranslation(&NpcSphere.Transform.Position);
        Mat4x4 NpcScale = Identity4x4;

        /*
        AABB TransformedPlayerAABB = {};
        AABBRecalculate(&PlayerRotation, &PlayerSphere.Transform.Position, &PlayerCube.BoundingBox, &TransformedPlayerAABB);
        */
        /*
        AABB TransformedNpcAABB = {};
        AABBRecalculate(&NpcRotation, &NpcCube.Transform.Position, &NpcCube.BoundingBox, &TransformedNpcAABB);
        */

        PlayerSphere.BoundingSphere.Center = PlayerSphere.Transform.Position;
        NpcSphere.BoundingSphere.Center = NpcSphere.Transform.Position;

        if (SphereToSphereTestOverlap(&PlayerSphere.BoundingSphere, &NpcSphere.BoundingSphere)) {
            PlayerSphereColor.x = 0.0f;
            PlayerSphereColor.y = 1.0f;
            PlayerSphereColor.z = 0.0f;
        }

        Vec3 Target = {};
        Vec3 Right = {};
        Vec3 Up = {};
        // TODO (ismail): camera rotation is so buggy, i need fix this
        // NOTE (ismail): replace RotationToVectors call?
        Mat4x4 CameraWorldRotation = MakeRotation4x4Inverse(&PlayerCamera.Transform.Rotate);
        RotationToVectors(&PlayerCamera.Transform.Rotate, &Target, &Right, &Up);

        // TODO (ismail): if we press up and right(or any other combination of up, down, right, left)
        // we will translate faster because z = 1.0f and x = 1.0f i need fix that
        Vec3 CameraTargetTranslation = { 
            Target.x * (CameraTargetTranslationDelta * Speed * 0.0001234f),
            Target.y * (CameraTargetTranslationDelta * Speed * 0.0001234f),
            Target.z * (CameraTargetTranslationDelta * Speed * 0.0001234f)
        };

        Vec3 CameraRightTranslation = { 
            Right.x * (CameraRightTranslationDelta * Speed * 0.0001234f),
            Right.y * (CameraRightTranslationDelta * Speed * 0.0001234f),
            Right.z * (CameraRightTranslationDelta * Speed * 0.0001234f)
        };

        Vec3 FinalTranslation = CameraTargetTranslation + CameraRightTranslation;

        PlayerCamera.Transform.Position += FinalTranslation;
        Vec3 ObjectToCameraTranslation = { 
            -(PlayerCamera.Transform.Position.x), 
            -(PlayerCamera.Transform.Position.y), 
            -(PlayerCamera.Transform.Position.z) 
        };

        Mat4x4 CameraWorldTranslation = MakeTranslation(&ObjectToCameraTranslation);
        Mat4x4 CameraTransformation = CameraWorldRotation * CameraWorldTranslation;

        Mat4x4 PlayerCubeWorldTransformation = PlayerTranslation * PlayerScale * PlayerRotation;
        Mat4x4 PlayerCubeTransform = Projection * CameraTransformation * PlayerCubeWorldTransformation;

        // SDL_Log("Y rotation = %.2f\tX rotation = %.2f\n", RAD_TO_DEGREE(AngelInRadY), RAD_TO_DEGREE(AngelInRadX));

        // NOTE (ismail) 3rd argument is transpose that transpose mean memory order of this matrix
        // GL_TRUE for row based memory order: (memory - matrix cell)  0x01 - a11, 0x02 - a12, 0x03 - a13
        // GL_FALSE for column based memory order: (memory - matrix cell) 0x01 - a11, 0x02 - a21, 0x03 - a31
        glUniformMatrix4fv(TransformLocation, 1, GL_TRUE, PlayerCubeTransform[0]);
        glUniform3fv(MeshColorLocation, 1, PlayerSphereColor.ValueHolder);

        glDrawElements(GL_TRIANGLES, SphereFile.IndexArraySize, GL_UNSIGNED_INT, 0);

        Mat4x4 NpcCubeWorldTransformation = NpcTranslation * NpcScale * NpcRotation;
        Mat4x4 NpcCubeTransform = Projection * CameraTransformation * NpcCubeWorldTransformation;

        glUniformMatrix4fv(TransformLocation, 1, GL_TRUE, NpcCubeTransform[0]);
        glUniform3fv(MeshColorLocation, 1, NpcSphereColor.ValueHolder);

        glDrawElements(GL_TRIANGLES, SphereFile.IndexArraySize, GL_UNSIGNED_INT, 0);

        SDL_GL_SwapWindow(Window);
    }

    return 0;
}