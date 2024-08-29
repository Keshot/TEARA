#include <SDL3/SDL.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_opengl.h>

#include "TEARA_Lib/Utils/Types.h"
#include "TEARA_Lib/Math/MatrixTransform.h"
#include "TEARA_Lib/Math/CoreMath.h"
#include "TEARA_Lib/Physics/CollisionDetection.h"

#define FILE_READ_BUFFER_LEN 5000
#define MOUSE_EVENT(type) ((type == SDL_EVENT_MOUSE_MOTION)||(type == SDL_EVENT_MOUSE_BUTTON_UP)||(type == SDL_EVENT_MOUSE_BUTTON_DOWN))

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

#include <windows.h>
#include "TEARA_Core/Engine.h"
#include "TEARA_Lib/Utils/AssetsLoader.h"
#include "TEARA_Core/Rendering/Renderer.cpp"

#ifndef WIN32_CLASS_NAME
    #define WIN32_WINDOW_CLASS_NAME ("TEARA Engine")
#endif
#ifndef WIN32_FAKE_CLASS_NAME
    #define WIN32_FAKE_CLASS_NAME ("TEARA_WGL_FAKE")
#endif
#ifndef GAME_NAME
    #define GAME_NAME WIN32_WINDOW_CLASS_NAME
#endif

#define OPENGL_PIXEL_FORMAT_FLAGS (PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER)

struct WorldTransform {
    Vec3        Position;
    Rotation    Rotate;
    Vec3        Scale;
};

struct WorldObject {
    WorldTransform          Transform;
    BoundingVolume          BoundingVolume;
    Vec3                    Color;
    i32                     RendererContextIndex;
};

struct Camera {
    WorldTransform Transform;
};

enum WinLocalErrors {
    Success                         =  0,
    RegisterClassFailed             = -1,
    WindowCreateFailed              = -2,
    GetDCCallFailed                 = -3,
    LibraryDllLoadFailed            = -4,
    FunctionLoadFailed              = -5,
    GLContextCreateFailed           = -6,
};

struct Win32Context {
    HINSTANCE   AppInstance;
    HWND        Window;
    HDC         WindowDeviceContext;
    HGLRC       GLDeviceContext;
    bool32      Running;
};

static Win32Context Win32App;

// TODO (ismail): remove this cringe
#define SCENE_OBJECT_SIZE   (0x0A)
#define PLAYER_INDEX        (0x00)

static WorldObject              SceneObjects[SCENE_OBJECT_SIZE];
static ObjectRenderingContext   SceneObjectsRendererContext[SCENE_OBJECT_SIZE];

static File LoadFile(const char *FileName)
{
    LARGE_INTEGER   FileSize;
    i32             TruncatedFileSize;
    DWORD           BytesRead;
    File            Result = {};

    HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE) {
        if (GetFileSizeEx(FileHandle, &FileSize)) {
            TruncatedFileSize = SafeTruncateI64(FileSize.QuadPart);
            Result.Data = (byte*)VirtualAlloc(0, (SIZE_T)TruncatedFileSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (Result.Data) {
                if (ReadFile(FileHandle, (void*)Result.Data, TruncatedFileSize, &BytesRead, 0) &&
                    TruncatedFileSize == BytesRead) {
                    Result.Size = (u64)BytesRead;
                }
                else {
                    FreeFileMemory(&Result);
                    Result.Data = 0;
                }
            }
            else {
                // TODO (ismail): diagnostic things?    
            }
        }
        else {
            // TODO (ismail): diagnostic things?    
        }

        CloseHandle(FileHandle);
    }
    else {
        // TODO (ismail): diagnostic things?
    }

    return Result;
}

static inline void FreeFileMemory(File *FileData)
{
    if (FileData->Data) {
        VirtualFree(FileData->Data, 0, MEM_RELEASE);
    }
}

static inline void WinGetDesiredPixelFormat(PIXELFORMATDESCRIPTOR *PixelFormat)
{
    PixelFormat->nSize            = sizeof(PIXELFORMATDESCRIPTOR); // size of that struct
    PixelFormat->nVersion         = 1; // format version it is always 1
    PixelFormat->dwFlags          = OPENGL_PIXEL_FORMAT_FLAGS;
    PixelFormat->iPixelType       = PFD_TYPE_RGBA; // that parameter is always here when we use opengl
    PixelFormat->cColorBits       = 32; // RGBA
    PixelFormat->cRedBits         = 0; // ignored
    PixelFormat->cRedShift        = 0; // ignored
    PixelFormat->cGreenBits       = 0; // ignored
    PixelFormat->cGreenShift      = 0; // ignored
    PixelFormat->cBlueBits        = 0; // ignored
    PixelFormat->cBlueShift       = 0; // ignored
    PixelFormat->cAlphaBits       = 8; // 8 bit alpha channel
    PixelFormat->cAlphaShift      = 0; // ignored
    PixelFormat->cAccumBits       = 0; // ignored
    PixelFormat->cAccumRedBits    = 0; // ignored
    PixelFormat->cAccumGreenBits  = 0; // ignored
    PixelFormat->cAccumBlueBits   = 0; // ignored
    PixelFormat->cAccumAlphaBits  = 0; // ignored
    PixelFormat->cDepthBits       = 24; // common 24 bits depth buffer
    PixelFormat->cStencilBits     = 8; // common 8 bits stencil buffer
    PixelFormat->cAuxBuffers      = 0; // ignored
    PixelFormat->iLayerType       = 0; // ignored
    PixelFormat->bReserved        = 0; // ignored
    PixelFormat->dwLayerMask      = 0; // ignored
    PixelFormat->dwVisibleMask    = 0; // ignored
    PixelFormat->dwDamageMask     = 0; // ignored
}

static LRESULT WinFakeMainCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    if (Message == WM_DESTROY) {
        PostQuitMessage(0);
    }
    return DefWindowProcA(Window, Message, WParam, LParam);
}

static WinLocalErrors WinLoadOpenGLExtensions(const char *DllName)
{
    MSG                     Message;
    HGLRC                   FakeGLContext;
    HDC                     FakeDeviceContext;
    i32                     SuggestedPixelFormatIndex;
    PIXELFORMATDESCRIPTOR   SuggestedPixelFormat, DesiredPixelFormat;
    HWND                    FakeWindow;

    FakeWindow = CreateWindowA(WIN32_FAKE_CLASS_NAME, 
                               GAME_NAME, 
                               WS_OVERLAPPEDWINDOW, 
                               CW_USEDEFAULT, CW_USEDEFAULT, 
                               640, 480, 
                               NULL, NULL, 
                               Win32App.AppInstance, NULL);
    
    if (!FakeWindow) {
        // TODO (ismail): file or/and console logging? and Assert
        return WinLocalErrors::WindowCreateFailed;
    }

    FakeDeviceContext = GetDC(FakeWindow);

    WinGetDesiredPixelFormat(&DesiredPixelFormat);

    SuggestedPixelFormatIndex = ChoosePixelFormat(FakeDeviceContext, &DesiredPixelFormat);
    DescribePixelFormat(FakeDeviceContext, SuggestedPixelFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), &SuggestedPixelFormat);
    SetPixelFormat(FakeDeviceContext, SuggestedPixelFormatIndex, &SuggestedPixelFormat);

    FakeGLContext = wglCreateContext(FakeDeviceContext);
    if (!wglMakeCurrent(FakeDeviceContext, FakeGLContext)) {
        // TODO (ismail): file or/and console logging? and Assert
        return WinLocalErrors::GLContextCreateFailed;
    }

    tglCreateContextAttribsARB = (TEARA_glCreateContextAttribsARB) wglGetProcAddress("wglCreateContextAttribsARB");
    if (!tglCreateContextAttribsARB) {
        // TODO (ismail): file or/and console logging and Assert?
        return WinLocalErrors::FunctionLoadFailed;
    }

    tglChoosePixelFormatARB = (TEARA_glChoosePixelFormatARB) wglGetProcAddress("wglChoosePixelFormatARB");
    if (!tglChoosePixelFormatARB) {
        // TODO (ismail): file or/and console logging and Assert?
        return WinLocalErrors::FunctionLoadFailed;
    }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(FakeGLContext);

    ReleaseDC(FakeWindow, FakeDeviceContext);
    DestroyWindow(FakeWindow);

    while (GetMessage(&Message, NULL, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    UnregisterClassA(WIN32_FAKE_CLASS_NAME, Win32App.AppInstance);
    
    return WinLocalErrors::Success;
}

static WinLocalErrors WinInitOpenGLContext()
{
    int                     SuggestedPixelFormatIndex;
    PIXELFORMATDESCRIPTOR   DesiredPixelFormat, SuggestedPixelFormat;

    int GLAttribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 6,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0 // NOTE (ismail): this describe end of attributes
    };

    if (tglChoosePixelFormatARB && false) {
        // TODO (ismail): set here more complicated PixelFormat with tglChoosePixelFormatARB
    }
    else {
        WinGetDesiredPixelFormat(&DesiredPixelFormat);

        SuggestedPixelFormatIndex = ChoosePixelFormat(Win32App.WindowDeviceContext, &DesiredPixelFormat);
        DescribePixelFormat(Win32App.WindowDeviceContext, SuggestedPixelFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), &SuggestedPixelFormat);
    }

    SetPixelFormat(Win32App.WindowDeviceContext, SuggestedPixelFormatIndex, &SuggestedPixelFormat);

    Win32App.GLDeviceContext = tglCreateContextAttribsARB(Win32App.WindowDeviceContext, 0, GLAttribs);

    if (!wglMakeCurrent(Win32App.WindowDeviceContext, Win32App.GLDeviceContext)) {
        // TODO (ismail): file or/and console logging and Assert?
        return WinLocalErrors::GLContextCreateFailed;
    }

    // TODO (ismail): print opengl version to logs
    // const GLubyte *Version = glGetString(GL_VERSION);

    // NOTE (ismail): we do not need call a ReleaseDC because our window class was created with CS_OWNDC

    return WinLocalErrors::Success;
}

static LRESULT WinMainCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    // NOTE(ismail): normally windows pipeline is GetMessage TranslateMessage and then DispatchMessage
    // DispatchMessage call our user space callback which must handle messages by self or send it in DefWindowProc
    // but sometimes windows can send message in user space callback by yourself

    LRESULT Result = 0;

    switch (Message) {
        case WM_CLOSE: {
            // TODO(ismail): handle this with a message to the user, may be some pop up about save information or something?
            Win32App.Running = false;
            
            OutputDebugStringA("WM_CLOSE\n"); // TODO(ismail): change it on to some log in file or console, maybe only if we build in debug mode?
        } break;

        case WM_ACTIVATEAPP: {
            OutputDebugStringA("WM_ACTIVATEAPP\n"); // TODO(ismail): change it on to some log in file or console, maybe only if we build in debug mode?
        } break;
        
        case WM_DESTROY: {
            // TODO(ismail): hande this as an error, recreate window?
            Win32App.Running = false;

            OutputDebugStringA("WM_DESTROY\n"); // TODO(ismail): change it on to some log in file or console, maybe only if we build in debug mode?
        } break;

        // NOTE(ismail): check what it mean KEYDOWN and KEYUP and what flags can be usefull 
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP: {
            u32 VKCode = WParam; // NOTE(ismail): virtual code of that key that was pressed or released
            bool32 WasDown = ((LParam & (1 << 30)) != 0);
            bool32 IsDown =  ((LParam & (1 << 31)) == 0);

            if (WasDown != IsDown) {
                // NOTE(ismail) may be it will be better if i will be use actual virtual codes instead of ascii code
                switch (VKCode)
                {
                    case 'w':
                    case 'W': {
                        OutputDebugStringA("W\n");
                    } break;
                    
                    case 'a':
                    case 'A': {

                    } break;

                    case 's':
                    case 'S': {

                    } break;
                    
                    case 'd':
                    case 'D': {

                    } break;

                    case 'q':
                    case 'Q': {

                    } break;
                    
                    case 'e':
                    case 'E': {

                    } break;

                    case VK_UP: {

                    } break;

                    case VK_DOWN: {

                    } break;

                    case VK_LEFT: {

                    } break;

                    case VK_RIGHT: {

                    } break;

                    case VK_ESCAPE: {

                    } break;

                    case VK_SPACE: {

                    } break;
                    
                    default: break;
                }
            }

            // NOTE(ismail): 29 bit of this parameter check alt key was down
            bool32 AltWasDown = (LParam & (1 << 29));
            if (VKCode == VK_F4 && AltWasDown) {
                Win32App.Running = false;
            }

        } break;

        default: {
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        } break;
    }

    return Result;
}

static WinLocalErrors WinRegClasses()
{
    ATOM        WinClassAtom, FakeWinClassAtom;
    WNDCLASSA   WinClass, FakeWinClass;

    // TODO (ismail): add icon when we will be have one (WindowClass.hIcon)
    // TODO (ismail): add cursor when we will be have one (WindowClass.hCursor)
    WinClass.style          = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    WinClass.lpfnWndProc    = &WinMainCallback;
    WinClass.cbClsExtra     = 0;
    WinClass.cbWndExtra     = 0;
    WinClass.hInstance      = Win32App.AppInstance;
    WinClass.hIcon          = 0;
    WinClass.hCursor        = 0;
    WinClass.hbrBackground  = 0; // TODO (ismail): check if i need this
    WinClass.lpszMenuName   = 0;
    WinClass.lpszClassName  = WIN32_WINDOW_CLASS_NAME;

    WinClassAtom = RegisterClassA(&WinClass);

    if (!WinClassAtom) {
        // TODO (ismail): file or/and console logging?
        return WinLocalErrors::RegisterClassFailed;
    }

    FakeWinClass.style          = CS_OWNDC;
    FakeWinClass.lpfnWndProc    = &WinFakeMainCallback;
    FakeWinClass.cbClsExtra     = 0;
    FakeWinClass.cbWndExtra     = 0;
    FakeWinClass.hInstance      = Win32App.AppInstance;
    FakeWinClass.hIcon          = 0;
    FakeWinClass.hCursor        = 0;
    FakeWinClass.hbrBackground  = 0; // TODO (ismail): check if i need this
    FakeWinClass.lpszMenuName   = 0;
    FakeWinClass.lpszClassName  = WIN32_FAKE_CLASS_NAME;

    FakeWinClassAtom = RegisterClassA(&FakeWinClass);

    if (!FakeWinClassAtom) {
        // TODO (ismail): file or/and console logging? and Assert
        return WinLocalErrors::RegisterClassFailed;
    }

    return WinLocalErrors::Success;
}

static WinLocalErrors WinCreateWindow()
{
    DWORD       WindowExStyles;
    // TODO (ismail): why i need WS_MAXIMIZEBOX or WS_MINIMIZEBOX that include to WS_OVERLAPPEDWINDOW?
    DWORD       WindowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

    // TODO (ismail): if fullscreen style = WS_EX_TOPMOST
    WindowExStyles = 0;

    // TODO (ismail): set actual x, y and width and height
    Win32App.Window = CreateWindowExA(WindowExStyles, WIN32_WINDOW_CLASS_NAME, 
                                      GAME_NAME, WindowStyle, 
                                      CW_USEDEFAULT , CW_USEDEFAULT, 
                                      CW_USEDEFAULT, CW_USEDEFAULT, 
                                      0, 0, 
                                      Win32App.AppInstance, 0);
    
    if (!Win32App.Window) {
        // TODO (ismail): file or/and console logging?
        return WinLocalErrors::WindowCreateFailed;
    }

    Win32App.WindowDeviceContext = GetDC(Win32App.Window);
    if (!Win32App.WindowDeviceContext) {
        // TODO (ismail): file or/and console logging?
        return WinLocalErrors::GetDCCallFailed;
    }

    return WinLocalErrors::Success;
}

static WinLocalErrors WinInit()
{
    WinLocalErrors Result;

    if ( (Result = WinRegClasses()) != WinLocalErrors::Success) {
        return Result;
    }

    if ( (Result = WinLoadOpenGLExtensions("opengl32.dll")) != WinLocalErrors::Success) {
        return Result;
    }

    if ( (Result = WinCreateWindow()) != WinLocalErrors::Success) {
        return Result;
    }

    if ( (Result = WinInitOpenGLContext()) != WinLocalErrors::Success) {
        return Result;
    }

    return WinLocalErrors::Success;
}

void Frame()
{

}

i32 WorldPrepare()
{    
    // NOTE (ismail): i disable textures for now that call need for load texture
    // GLuint TextureObject = LoadTexture("bricks_textures.jpg");
    // NOTE (ismail): these calls need later for put it in screen
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, TextureObject);
    // glUniform1i(SamplerLocation, 0);

    GLuint ShaderProgram = tglCreateProgram();
    if (!ShaderProgram) {
        // TODO (ismail): diagnostic
        return 0; // TODO (ismail): put here actual value
    }

    File VertexShader = LoadFile("data/shaders/vertex_color.vs");
    File FragmentShader = LoadFile("data/shaders/fragment_color.fs");

    if (!VertexShader.Data || !FragmentShader.Data) {
        // TODO (ismail): diagnostic
        return -1;
    }

    if (!AttachShader(ShaderProgram, (const char*)VertexShader.Data, (GLint)VertexShader.Size, GL_VERTEX_SHADER)) {
        return -1; // TODO (ismail): more complicated check here
    }

    if (!AttachShader(ShaderProgram, (const char*)FragmentShader.Data, (GLint)FragmentShader.Size, GL_FRAGMENT_SHADER)) {
        return -1; // TODO (ismail): more complicated check here
    }

    FreeFileMemory(&VertexShader);
    FreeFileMemory(&FragmentShader);

    GLchar ErrorLog[1024] = { 0 };
    tglLinkProgram(ShaderProgram);

    GLint Success;
    tglGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
    if (!Success) {
        // TODO (ismail): handle this case with glGetShaderInfoLog and print it in log
        tglGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        return -1;
    }
    
    GLint TransformLocation = tglGetUniformLocation(ShaderProgram, "Transform");
    if (TransformLocation == -1) {
        // TODO (ismail): diagnostic
        return OPENGL_ERROR;
    }

    GLint MeshColorLocation = tglGetUniformLocation(ShaderProgram, "MeshColor");
    if (MeshColorLocation == -1) {
        // TODO (ismail): diagnostic
        return OPENGL_ERROR;
    }

    //  GLint SamplerLocation = glGetUniformLocation(ShaderProgram, "Sampler");
    //  if (TransformLocation == -1) {
           // TODO (ismail): diagnostic
    //     return OPENGL_ERROR;
    //  }

    tglValidateProgram(ShaderProgram);
    tglGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        // TODO (ismail): handle this case with glGetShaderInfoLog and print it in log
        return SDL_INIT_FAILURE;
    }

    // NOTE (ismail): new scene objects add here
    u32 IndexBuffer1[4000];
    Vertex VertexBuffer1[4000];

    ObjFile ObjectCube = { };
    ObjectCube.Indices = IndexBuffer1;
    ObjectCube.Vertices = VertexBuffer1;

    LoadObjFile("data/obj/cube.obj", &ObjectCube);

    u32 IndexBuffer2[4000];
    Vertex VertexBuffer2[4000];
    
    ObjFile ObjectSphere = { };
    ObjectSphere.Indices = IndexBuffer2;
    ObjectSphere.Vertices = VertexBuffer2;

    LoadObjFile("data/obj/sphere.obj", &ObjectSphere);

    ObjectRenderingContext CubeObjectRenderContext;
    LoadObjectToHardware(&CubeObjectRenderContext, &ObjectCube);

    ObjectRenderingContext SphereObjectRenderContext;
    LoadObjectToHardware(&SphereObjectRenderContext, &ObjectSphere);

    SceneObjects[PLAYER_INDEX].Transform.Position.x = 1.5f;
    SceneObjects[PLAYER_INDEX].Transform.Position.y = 0.0f;
    SceneObjects[PLAYER_INDEX].Transform.Position.z = 2.0f;

    SceneObjects[PLAYER_INDEX].Transform.Scale.x = 1.0f;
    SceneObjects[PLAYER_INDEX].Transform.Scale.y = 1.0f;
    SceneObjects[PLAYER_INDEX].Transform.Scale.z = 1.0f;

    SceneObjects[PLAYER_INDEX].BoundingVolume.VolumeType = BoundingVolumeType::OBBVolume;

    SceneObjects[PLAYER_INDEX].BoundingVolume.VolumeData.OrientedBox.Center = SceneObjects[PLAYER_INDEX].Transform.Position;

    SceneObjects[PLAYER_INDEX].BoundingVolume.VolumeData.OrientedBox.Axis[0] = { 1.0f, 0.0f, 0.0f };
    SceneObjects[PLAYER_INDEX].BoundingVolume.VolumeData.OrientedBox.Axis[1] = { 0.0f, 1.0f, 0.0f };;
    SceneObjects[PLAYER_INDEX].BoundingVolume.VolumeData.OrientedBox.Axis[2] = { 0.0f, 0.0f, 1.0f };;

    SceneObjects[PLAYER_INDEX].BoundingVolume.VolumeData.OrientedBox.Extens.x = 1.0f; // TODO (ismail): initial AABB compute need to be implemented
    SceneObjects[PLAYER_INDEX].BoundingVolume.VolumeData.OrientedBox.Extens.y = 1.0f; // TODO (ismail): initial AABB compute need to be implemented
    SceneObjects[PLAYER_INDEX].BoundingVolume.VolumeData.OrientedBox.Extens.z = 1.0f; // TODO (ismail): initial AABB compute need to be implemented

    SceneObjects[PLAYER_INDEX].Color = { 0.0f, 0.0f, 1.0f };

    SceneObjects[PLAYER_INDEX].RendererContext = CubeObjectRenderContext;

    WorldObjectOBBCollider NpcCube = {};

    NpcCube.Transform.Position.x = -1.5f;
    NpcCube.Transform.Position.y = 0.0f;
    NpcCube.Transform.Position.z = 2.0f;

    NpcCube.Transform.Scale.x = 1.0f;
    NpcCube.Transform.Scale.y = 1.0f;
    NpcCube.Transform.Scale.z = 1.0f;

    NpcCube.BoundingOrientedBox.Center = NpcCube.Transform.Position;

    NpcCube.BoundingOrientedBox.Axis[0] = { 1.0f, 0.0f, 0.0f };
    NpcCube.BoundingOrientedBox.Axis[1] = { 0.0f, 1.0f, 0.0f };;
    NpcCube.BoundingOrientedBox.Axis[2] = { 0.0f, 0.0f, 1.0f };;

    NpcCube.BoundingOrientedBox.Extens.x = 1.0f; // TODO (ismail): initial AABB compute need to be implemented
    NpcCube.BoundingOrientedBox.Extens.y = 1.0f; // TODO (ismail): initial AABB compute need to be implemented
    NpcCube.BoundingOrientedBox.Extens.z = 1.0f; // TODO (ismail): initial AABB compute need to be implemented

    Vec3 NpcCubeColor = { 1.0f, 0.0f, 0.0f };

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

    /*
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
    */

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

    /*
    WorldObjectSphereCollider PlayerSphere = {};

    PlayerSphere.Transform.Position.x = 1.5f;
    PlayerSphere.Transform.Position.y = 0.0f;
    PlayerSphere.Transform.Position.z = 2.0f;

    PlayerSphere.Transform.Scale.x = 1.0f;
    PlayerSphere.Transform.Scale.y = 1.0f;
    PlayerSphere.Transform.Scale.z = 1.0f;

    PlayerSphere.BoundingSphere.Center = PlayerSphere.Transform.Position; // TODO (ismail): initial Sphere compute need to be implemented
    PlayerSphere.BoundingSphere.Radius = 1.0f; // TODO (ismail): initial Sphere compute need to be implemented
    */
}

i32 APIENTRY WinMain( HINSTANCE Instance, HINSTANCE PrevInstance, 
                      LPSTR CommandLine , int ShowCode)
{
    WinLocalErrors              WinIntiError;
    OpenGLFunctionLoadStatus    OpenGLInitError;
    MSG                         Message;

    LARGE_INTEGER PerfomanceCountFrequencyResult;
    QueryPerformanceFrequency(&PerfomanceCountFrequencyResult);
    i64 PerfCountFrequency = PerfomanceCountFrequencyResult.QuadPart;

    Win32App.Running        = true;
    Win32App.AppInstance    = Instance;

    if ( (WinIntiError = WinInit()) != WinLocalErrors::Success) {
        // TODO (ismail): diagnostics things?
        return WinIntiError;
    }

    if ( (OpenGLInitError = LoadGLFunctions()) != OpenGLFunctionLoadStatus::Success) {
        // TODO (ismail): diagnostics things?
        return OpenGLInitError;
    }

    RendererInit();

    AssetsLoaderInit();

    LARGE_INTEGER LastCounter;
    QueryPerformanceCounter(&LastCounter);
    u64 LastCycleCount = __rdtsc();

    while (Win32App.Running) {
        while (PeekMessageA(&Message, Win32App.Window, 0, 0, PM_REMOVE)) {
            if (Message.message == WM_QUIT) {
                Win32App.Running = false;
            }
                    
            TranslateMessage(&Message);
            DispatchMessageA(&Message);
        }

        Frame();

        // NOTE(ismail): some very usefull thing for perfomance debuging
        u64 EndCycleCount = __rdtsc();

        LARGE_INTEGER EndCounter;
        QueryPerformanceCounter(&EndCounter);

        u64 CyclesElapsed = EndCycleCount - LastCycleCount;
        i64 CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
        real64 MSPF = (1000.0f * ((real64)CounterElapsed / (real64)PerfCountFrequency)); // millisecond per frame
        real64 FPS = (1000.0f / MSPF); // frame per seconds
        real64 MCPF = (((real64)CyclesElapsed) / (1000.0f * 1000.0f)); // mega cycles per frame, how many cycles on CPU take last frame check rdtsc and hh ep 10

        char Buffer[256];
        snprintf(Buffer, sizeof(Buffer), "| %.02fms/f | %.02f f/s | %.02f mc/f |\n", MSPF, FPS, MCPF);

        OutputDebugStringA(Buffer);

        LastCycleCount = EndCycleCount;
        LastCounter = EndCounter;
    }

    return 0;
}

int hui(int argc, char *argv[])
{
    //TODO (ismail): read width/height from command arguments, from file, from in game settings.
    SDL_Event Event;
    SDL_Window *Window;
    SDL_Surface *WindowScreenSurface;
    SDL_GLContext GlContext;
    bool32 Quit = false;
    i32 OGLResult = 0;

    if (SDL_Init(SDLInitFlags) < 0) {
        // TODO (ismail): logging
        return SDL_INIT_FAILURE;
    }

    if ((OGLResult = OGLInit()) != OGL_STATS::LOAD_SUCCESS) {
        // TODO (ismail): logging
        return OGLResult;
    }

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
    if ((LoadFunctionsResult = OGLLoadFunctions())) {
        // TODO (ismail): diagnostic
        return LoadFunctionsResult;
    }



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

    glUseProgram(ShaderProgram);
    glBindVertexArray(CubeObjectRenderContext.VAO);

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

        PlayerCube.Transform.Rotate.Heading += RotationDeltaY;
        PlayerCube.Transform.Rotate.Pitch += RotationDeltaX;

        PlayerCamera.Transform.Rotate.Heading += CameraYRotationDelta + (DEGREE_TO_RAD(MouseMoution.x));
        PlayerCamera.Transform.Rotate.Pitch += CameraXRotationDelta + (DEGREE_TO_RAD(MouseMoution.y));

        Vec3 PlayerTarget = {};
        Vec3 PlayerRight = {};
        Vec3 PlayerUp = {};

        Mat4x4 PlayerRotation = MakeRotation4x4(&PlayerCube.Transform.Rotate);
        RotationToVectors(&PlayerCube.Transform.Rotate, &PlayerTarget, &PlayerRight, &PlayerUp);

        Vec3 PlayerTargetTranslation = { 
            PlayerTarget.x * (PlayerTargetTranslationDelta * Speed * 0.0003702f),
            PlayerTarget.y * (PlayerTargetTranslationDelta * Speed * 0.0003702f),
            PlayerTarget.z * (PlayerTargetTranslationDelta * Speed * 0.0003702f)
        };

        Vec3 PlayerRightTranslation = { 
            PlayerRight.x * (PlayerRightTranslationDelta * Speed * 0.0003702f),
            PlayerRight.y * (PlayerRightTranslationDelta * Speed * 0.0003702f),
            PlayerRight.z * (PlayerRightTranslationDelta * Speed * 0.0003702f)
        };

        Vec3 FinalPlayerTranslation = PlayerTargetTranslation + PlayerRightTranslation;
        PlayerCube.Transform.Position += FinalPlayerTranslation;
        
        Mat4x4 PlayerTranslation = MakeTranslation(&PlayerCube.Transform.Position);
        Mat4x4 PlayerScale = Identity4x4;

        Mat4x4 NpcRotation = MakeRotation4x4(&NpcCube.Transform.Rotate);
        Mat4x4 NpcTranslation = MakeTranslation(&NpcCube.Transform.Position);
        Mat4x4 NpcScale = Identity4x4;

        Vec3 NpcTarget = {};
        Vec3 NpcRight = {};
        Vec3 NpcUp = {};

        RotationToVectors(&NpcCube.Transform.Rotate, &NpcTarget, &NpcRight, &NpcUp);

        /*
        AABB TransformedPlayerAABB = {};
        AABBRecalculate(&PlayerRotation, &PlayerSphere.Transform.Position, &PlayerCube.BoundingBox, &TransformedPlayerAABB);
        */
        /*
        AABB TransformedNpcAABB = {};
        AABBRecalculate(&NpcRotation, &NpcCube.Transform.Position, &NpcCube.BoundingBox, &TransformedNpcAABB);
        */

        /*
        PlayerSphere.BoundingSphere.Center = PlayerSphere.Transform.Position;
        NpcSphere.BoundingSphere.Center = NpcSphere.Transform.Position;

        if (SphereToSphereTestOverlap(&PlayerSphere.BoundingSphere, &NpcSphere.BoundingSphere)) {
            PlayerSphereColor.x = 0.0f;
            PlayerSphereColor.y = 1.0f;
            PlayerSphereColor.z = 0.0f;
        }
        */

        PlayerCube.BoundingOrientedBox.Axis[0] = PlayerRight;
        PlayerCube.BoundingOrientedBox.Axis[1] = PlayerUp;
        PlayerCube.BoundingOrientedBox.Axis[2] = PlayerTarget;

        PlayerCube.BoundingOrientedBox.Center = PlayerCube.Transform.Position;

        NpcCube.BoundingOrientedBox.Axis[0] = NpcRight;
        NpcCube.BoundingOrientedBox.Axis[1] = NpcUp;
        NpcCube.BoundingOrientedBox.Axis[2] = NpcTarget;

        NpcCube.BoundingOrientedBox.Center = NpcCube.Transform.Position;

        if (OBBToOBBTestOverlap(&PlayerCube.BoundingOrientedBox, &NpcCube.BoundingOrientedBox)) {
            PlayerCubeColor.x = 0.0f;
            PlayerCubeColor.y = 1.0f;
            PlayerCubeColor.z = 0.0f;
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
            Target.x * (CameraTargetTranslationDelta * Speed * 0.0003702f),
            Target.y * (CameraTargetTranslationDelta * Speed * 0.0003702f),
            Target.z * (CameraTargetTranslationDelta * Speed * 0.0003702f)
        };

        Vec3 CameraRightTranslation = { 
            Right.x * (CameraRightTranslationDelta * Speed * 0.0003702f),
            Right.y * (CameraRightTranslationDelta * Speed * 0.0003702f),
            Right.z * (CameraRightTranslationDelta * Speed * 0.0003702f)
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
        glUniform3fv(MeshColorLocation, 1, PlayerCubeColor.ValueHolder);

        glDrawElements(GL_TRIANGLES, SphereFile.IndexArraySize, GL_UNSIGNED_INT, 0);

        Mat4x4 NpcCubeWorldTransformation = NpcTranslation * NpcScale * NpcRotation;
        Mat4x4 NpcCubeTransform = Projection * CameraTransformation * NpcCubeWorldTransformation;

        glUniformMatrix4fv(TransformLocation, 1, GL_TRUE, NpcCubeTransform[0]);
        glUniform3fv(MeshColorLocation, 1, NpcCubeColor.ValueHolder);

        glDrawElements(GL_TRIANGLES, CubeFile.IndexArraySize, GL_UNSIGNED_INT, 0);

        SDL_GL_SwapWindow(Window);
    }

    return 0;
}