#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_stdinc.h>

#include "Types.h"
#include "MatrixTransform.h"
#include "CoreMath.h"

#define FILE_READ_BUFFER_LEN 5000
#define MOUSE_EVENT(type) ((type == SDL_EVENT_MOUSE_MOTION)||(type == SDL_EVENT_MOUSE_BUTTON_UP)||(type == SDL_EVENT_MOUSE_BUTTON_DOWN))

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

struct FileData {
    char Data[FILE_READ_BUFFER_LEN];
    u64 DataLength;
};

struct Vertex {
    Vec3 Position;
    Vec3 Color;
};

struct Camera {
    Vec3 Position;
    Rotation Rotation;
};

enum {
    SUCCESS = 0,
    SDL_INIT_FAILURE = -1,
    SDL_CREATE_WINDOW_FAILURE = -2,
    SDL_OPENGL_LOAD_FAILURE = -3,
    SDL_OPENGL_CREATE_CONTEXT_FAILURE = -4,
    OPENGL_ERROR = -5,
    SDL_ERROR = -6,

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

static GLuint CreateSimpleBox()
{
    GLuint VBOResult;

    Vertex Vertices[] = {
        {
            { -0.5f, -0.5f, -0.5f },
            { 1.0f, 0.0f, 0.0f }
        },
        {
            { -0.5f, 0.5f, -0.5f },
            { 0.0f, 1.0f, 0.0f }
        },
        {
            { 0.5f, 0.5f, -0.5f },
            { 0.0f, 0.0f, 1.0f }
        },
        {
            { 0.5f, -0.5f, -0.5f },
            { 0.25f, 0.25f, 0.25f }
        },
        {
            { -0.5f, -0.5f, 0.5f },
            { 1.0f, 0.0f, 0.0f }
        },
        {
            { -0.5f, 0.5f, 0.5f },
            { 0.0f, 1.0f, 0.0f }
        },
        {
            { 0.5f, 0.5f, 0.5f },
            { 0.0f, 0.0f, 1.0f }
        },
        {
            { 0.5f, -0.5f, 0.5f },
            { 0.25f, 0.25f, 0.25f }
        },
    };

    glGenBuffers(1, &VBOResult);
    glBindBuffer(GL_ARRAY_BUFFER, VBOResult);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    
    return VBOResult;
}

static GLuint CreateSimpleBoxVertexIndices()
{
    GLuint IBOResult;

    u32 Indices[] = {
        // face
        0, 1, 2,
        2, 3, 0,
        // top
        1, 5, 6,
        6, 2, 1,
        // bottom
        0, 7, 4,
        0, 3, 7,
        // left
        0, 4, 5,
        5, 1, 0,
        // right
        3, 2, 6,
        3, 6, 7,
        // back
        4, 6, 5,
        4, 7, 6
    };

    glGenBuffers(1, &IBOResult);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBOResult);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
    
    return IBOResult;
}


int main(int argc, char *argv[])
{
    //TODO (ismail): read width/height from command arguments, from file, from in game settings.

    SDL_Event Event;
    SDL_Window *Window;
    SDL_Surface *WindowScreenSurface;
    SDL_GLContext GlContext;
    bool32 Quit = false;

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

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

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

    GLuint VBO = CreateSimpleBox();
    GLuint IBO = CreateSimpleBoxVertexIndices();

    GLuint ShaderProgram = glCreateProgram();
    if (!ShaderProgram) {
        // TODO (ismail): diagnostic
        return 0; // TODO (ismail): put here actual value
    }

    FileData VertexShader = {  }, FragmentShader = {  };

    if (ReadFile("../vertex.vs", &VertexShader) != SUCCESS) {
        // TODO (ismail): diagnostic
        return SDL_ERROR;
    }

    if (ReadFile("../fragment.fs", &FragmentShader) != SUCCESS) {
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

    Mat4x4 OrthoProjection = MakeOrtographProjection(-2.0f * AspectRatio, 2.0f * AspectRatio, -2.0f, 2.0f, 0.1f, 100.0f);
    Mat4x4 PerspectiveProjection = MakePerspectiveProjection(60.0f, AspectRatio, 0.1f, 5.0f);

    bool isPerspective = true;

    Mat4x4 Projection = isPerspective ? PerspectiveProjection : OrthoProjection;

    GLfloat BaseTranslationZ = 1.5f;
    GLfloat PerspectiveTranslationZ = 2.0f;

    Camera PlayerCamera = {  };

    GLfloat Speed = 5.0f;

    GLfloat CameraYRotationDelta = 0.0f;
    GLfloat CameraXRotationDelta = 0.0f;

    GLfloat CameraTargetTranslationDelta = 0.0f;
    GLfloat CameraRightTranslationDelta = 0.0f;

    // TODO (ismail): read what is this
    SDL_SetRelativeMouseMode(SDL_TRUE);

    while (!Quit) {
        Vec2 MouseMoution = { };

        while (SDL_PollEvent(&Event)) {
            if (Event.type == SDL_EVENT_QUIT) {
                Quit = true;
            }
            else if (Event.type == SDL_EVENT_KEY_DOWN) {
                switch (Event.key.key) {
                    // TODO (ismail): use raw key code
                    case SDLK_I: {
                        CameraXRotationDelta = -DEGREE_TO_RAD(0.03f);
                    } break;
                    case SDLK_K: {
                        CameraXRotationDelta = DEGREE_TO_RAD(0.03f);
                    } break;
                    case SDLK_L: {
                        CameraYRotationDelta = DEGREE_TO_RAD(0.03f);
                    } break;
                    case SDLK_J: {
                        CameraYRotationDelta = -DEGREE_TO_RAD(0.03f);
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
                        CameraXRotationDelta = 0.0f;
                    } break;
                    case SDLK_K: {
                        CameraXRotationDelta = 0.0f;
                    } break;
                    case SDLK_L: {
                        CameraYRotationDelta = 0.0f;
                    } break;
                    case SDLK_J: {
                        CameraYRotationDelta = 0.0f;
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

        AngelInRadY += RotationDeltaY;
        AngelInRadX += RotationDeltaX;
        Translation += TranslationDelta;
        Scale += ScaleDelta;
        PlayerCamera.Rotation.Heading += CameraYRotationDelta + (MouseMoution.x * 0.005);
        PlayerCamera.Rotation.Pitch += CameraXRotationDelta + (MouseMoution.y * 0.005);

        if ((Scale >= MaxScale) || (Scale <= MinScale)) {
            ScaleDelta *= -1.0f;
        }
        if ((Translation >= MaxTranslation) || (Translation <= MinTranslation)) {
            TranslationDelta *= -1.0f;
        }

        Vec3 ScaleVector = { 1.0f, 1.0f, 1.0f };
        Vec3 TranslationVector = { 0.0f, 0.0f, isPerspective ? PerspectiveTranslationZ : BaseTranslationZ };
        Mat4x4 RotationX = MakeRotationAroundX(AngelInRadX);
        Mat4x4 RotationY = MakeRotationAroundY(AngelInRadY);
        Mat4x4 Translation = MakeTranslation(&TranslationVector);
        Mat4x4 Scale = MakeScale(&ScaleVector);

        // 0.005234f

        Vec3 Target;
        Vec3 Right;
        Vec3 Up;
        // TODO (ismail): camera rotation is so buggy, i need fix this
        // NOTE (ismail): replace RotationToVectors call?
        Mat4x4 CameraWorldRotation = MakeRotation4x4Inverse(&PlayerCamera.Rotation);
        RotationToVectors(&PlayerCamera.Rotation, &Target, &Right, &Up);

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

        PlayerCamera.Position += FinalTranslation;
        Vec3 ObjectCameraTranslation = { 
            -(PlayerCamera.Position.x), 
            -(PlayerCamera.Position.y), 
            -(PlayerCamera.Position.z) 
        };

        Mat4x4 CameraWorldTranslation = MakeTranslation(&ObjectCameraTranslation);

        Mat4x4 WorldTransformation = Translation * Scale * RotationY * RotationX;
        Mat4x4 CameraTransformation = CameraWorldRotation * CameraWorldTranslation;
        Mat4x4 Transform = Projection * CameraTransformation * WorldTransformation;

        // SDL_Log("Y rotation = %.2f\tX rotation = %.2f\n", RAD_TO_DEGREE(AngelInRadY), RAD_TO_DEGREE(AngelInRadX));

        glClear(GL_COLOR_BUFFER_BIT);

        // NOTE (ismail) 3rd argument is transpose that transpose mean memory order of this matrix
        // GL_TRUE for row based memory order: (memory - matrix cell)  0x01 - a11, 0x02 - a12, 0x03 - a13
        // GL_FALSE for column based memory order: (memory - matrix cell) 0x01 - a11, 0x02 - a21, 0x03 - a31
        glUniformMatrix4fv(TransformLocation, 1, GL_TRUE, Transform[0]);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

        // position
        glEnableVertexAttribArray(0);        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);

        // color
        glEnableVertexAttribArray(1);        
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        SDL_GL_SwapWindow(Window);
    }

    return 0;
}