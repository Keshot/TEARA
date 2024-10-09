#include <windows.h>
#include <windowsx.h>
#include <stdio.h>

#include "TCore/Engine.h"
#include "TLib/Utils/Types.h"
#include "TLib/Math/MatrixTransform.h"
#include "TLib/Math/CoreMath.h"
#include "TLib/Physics/CollisionDetection.h"
#include "TLib/Utils/AssetsLoader.h"
#include "TCore/Rendering/Renderer.cpp"

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

// TODO (ismail): remove this cringe
#define SCENE_OBJECT_SIZE       (0x0A)
#define PLAYER_INDEX            (0x00)
#define SHADERS_PROGRAMS_SIZE   (0x05)

#define WIN_ARROW_UP_KEY_CODE       (VK_UP)
#define WIN_ARROW_DOWN_KEY_CODE     (VK_DOWN)
#define WIN_ARROW_RIGHT_KEY_CODE    (VK_RIGHT)
#define WIN_ARROW_LEFT_KEY_CODE     (VK_LEFT)
#define WIN_A_KEY_CODE              (0x41)
#define WIN_B_KEY_CODE              (0x42)
#define WIN_C_KEY_CODE              (0x43)
#define WIN_D_KEY_CODE              (0x44)
#define WIN_E_KEY_CODE              (0x45)
#define WIN_F_KEY_CODE              (0x46)
#define WIN_G_KEY_CODE              (0x47)
#define WIN_H_KEY_CODE              (0x48)
#define WIN_I_KEY_CODE              (0x49)
#define WIN_J_KEY_CODE              (0x4A)
#define WIN_K_KEY_CODE              (0x4B)
#define WIN_L_KEY_CODE              (0x4C)
#define WIN_M_KEY_CODE              (0x4D)
#define WIN_N_KEY_CODE              (0x4E)
#define WIN_O_KEY_CODE              (0x4F)
#define WIN_P_KEY_CODE              (0x50)
#define WIN_Q_KEY_CODE              (0x51)
#define WIN_R_KEY_CODE              (0x52)
#define WIN_S_KEY_CODE              (0x53)
#define WIN_T_KEY_CODE              (0x54)
#define WIN_U_KEY_CODE              (0x55)
#define WIN_V_KEY_CODE              (0x56)
#define WIN_W_KEY_CODE              (0x57)
#define WIN_X_KEY_CODE              (0x58)
#define WIN_Y_KEY_CODE              (0x59)
#define WIN_Z_KEY_CODE              (0x5A)

struct WorldTransform {
    Vec3        Position;
    Rotation    Rotate;
    Vec3        Scale;
};

struct MovementComponent {
    real32  Speed;
    real32  RotationDelta;
};

struct ObjectMaterial {
    TextureObject   Texture;
    Vec3            FlatColor;
    bool32          RenderInNextFrame;
};

struct WorldObject {
    WorldTransform          Transform;
    BoundingVolume          BoundingVolume;
    ObjectMaterial          Material;
    MovementComponent       Movement;
    i32                     RendererContextIndex;
};

struct SceneObjects {
    WorldObject     Objects[SCENE_OBJECT_SIZE];
    i64             ObjectsAmount;
};

struct SceneObjectsRendering {
    Mat4x4                  PerspectiveProjection;
    ObjectRenderingContext  ObjectsRenderingContext[SCENE_OBJECT_SIZE];
    i64                     ObjectsAmount;
};

struct SceneShaderPrograms {
    ShaderProgram            ShaderPrograms[SHADERS_PROGRAMS_SIZE];
    i64                      ProgramsAmount;
};

struct Camera {
    WorldTransform      Transform;
    MovementComponent   Movement;
};

struct ScreenOptions {
    i32     Width;
    i32     Height;
    real32  AspectRatio;
    i32     CenterW;
    i32     CenterH;
};

struct Win32Context {
    HINSTANCE       AppInstance;
    HWND            Window;
    HDC             WindowDeviceContext;
    HGLRC           GLDeviceContext;
    bool32          Running;
    ScreenOptions   ScreenOpt;
};

// TODO (ismail): check bug with camera when screen scale is biger than 100%
// when i set screen scale on 125% camera control feels worse
enum {
    WINDOW_WIDTH = 1600,
    WINDOW_HEIGHT = 900
};

static Win32Context             Win32App;
static SceneObjects             WorldObjects;
static SceneObjectsRendering    WorldObjectsRendererContext;
static SceneShaderPrograms      WorldShaderPrograms;
static AmbientLight             WorldAmbientLight;
static DirectionLight           WorldDirectionLight;
static Camera                   PlayerCamera;
static GameInput                Inputs;
static real32                   DeltaTime;

TEARA_PLATFORM_ALLOCATE_MEMORY(WinMemoryAllocate)
{
    Assert(Size > 0);
    return VirtualAlloc(0, Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

TEARA_PLATFORM_FREE_FILE_DATA(WinFreeFileData)
{
    if (FileData->Data) {
        VirtualFree(FileData->Data, 0, MEM_RELEASE);
    }
}

TEARA_PLATFORM_READ_FILE(WinReadFile)
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
                    WinFreeFileData(&Result);
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

static Statuses WinLoadOpenGLExtensions(const char *DllName)
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
        return Statuses::WindowCreateFailed;
    }

    FakeDeviceContext = GetDC(FakeWindow);

    WinGetDesiredPixelFormat(&DesiredPixelFormat);

    SuggestedPixelFormatIndex = ChoosePixelFormat(FakeDeviceContext, &DesiredPixelFormat);
    DescribePixelFormat(FakeDeviceContext, SuggestedPixelFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), &SuggestedPixelFormat);
    SetPixelFormat(FakeDeviceContext, SuggestedPixelFormatIndex, &SuggestedPixelFormat);

    FakeGLContext = wglCreateContext(FakeDeviceContext);
    if (!wglMakeCurrent(FakeDeviceContext, FakeGLContext)) {
        // TODO (ismail): file or/and console logging? and Assert
        return Statuses::GLContextCreateFailed;
    }

    tglCreateContextAttribsARB = (TEARA_glCreateContextAttribsARB) wglGetProcAddress("wglCreateContextAttribsARB");
    if (!tglCreateContextAttribsARB) {
        // TODO (ismail): file or/and console logging and Assert?
        return Statuses::FunctionLoadFailed;
    }

    tglChoosePixelFormatARB = (TEARA_glChoosePixelFormatARB) wglGetProcAddress("wglChoosePixelFormatARB");
    if (!tglChoosePixelFormatARB) {
        // TODO (ismail): file or/and console logging and Assert?
        return Statuses::FunctionLoadFailed;
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
    
    return Statuses::Success;
}

static Statuses WinInitOpenGLContext()
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
        return Statuses::GLContextCreateFailed;
    }

    // TODO (ismail): print opengl version to logs
    // const GLubyte *Version = glGetString(GL_VERSION);

    // NOTE (ismail): we do not need call a ReleaseDC because our window class was created with CS_OWNDC

    return Statuses::Success;
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
            Assert(0); // NOTE (ismail): keys get from main loop
        } break;

        case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEMOVE: {
            Assert(0); // NOTE (ismail): keys get from main loop
			break;
		}

        default: {
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        } break;
    }

    return Result;
}

void WinProcessKey(Key *ProcessKey, KeyState NewKeyState)
{
    if (ProcessKey->State != NewKeyState) {
        ProcessKey->State = NewKeyState;
        ++(ProcessKey->TransactionCount);
    }
}

void WinProcessMessages()
{
    u32         VirtualKeyCode;
    MSG         Message;
    KeyState    PreviousState;
    KeyState    NewState;
    bool32      AltWasDown;
    POINT       CursorCenterPos;

    while (PeekMessageA(&Message, Win32App.Window, 0, 0, PM_REMOVE)) {
        switch (Message.message) {
            case WM_QUIT: {
                Win32App.Running = false;
            } break;

            // NOTE(ismail): check what it mean KEYDOWN and KEYUP and what flags can be usefull 
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP: {

                VirtualKeyCode  = Message.wParam; // NOTE(ismail): virtual code of that key that was pressed or released
                AltWasDown      = (Message.lParam & (1 << 29)); // NOTE(ismail): 29 bit of this parameter check alt key was down

                PreviousState   = (KeyState)(Message.lParam & (1 << 30));
                NewState        = (KeyState)!(Message.lParam & (1 << 31));

                if (PreviousState != NewState) {
                    // NOTE(ismail) may be it will be better if i will be use actual virtual codes instead of ascii code
                    switch (VirtualKeyCode) {
                        case WIN_W_KEY_CODE: {
                            WinProcessKey(&Inputs.WButton, NewState);
                        } break;

                        case WIN_S_KEY_CODE: {
                            WinProcessKey(&Inputs.SButton, NewState);
                        } break;
                    
                        case WIN_A_KEY_CODE: {
                            WinProcessKey(&Inputs.AButton, NewState);
                        } break;

                        case WIN_D_KEY_CODE: {
                            WinProcessKey(&Inputs.DButton, NewState);
                        } break;

                        case WIN_Q_KEY_CODE: {
                            WinProcessKey(&Inputs.QButton, NewState);
                        } break;
                        
                        case WIN_E_KEY_CODE: {
                            WinProcessKey(&Inputs.EButton, NewState);
                        } break;

                        case WIN_I_KEY_CODE: {
                            WinProcessKey(&Inputs.IButton, NewState);
                        } break;

                        case WIN_K_KEY_CODE: {
                            WinProcessKey(&Inputs.KButton, NewState);
                        } break;

                        case WIN_L_KEY_CODE: {
                            WinProcessKey(&Inputs.LButton, NewState);
                        } break;

                        case WIN_J_KEY_CODE: {
                            WinProcessKey(&Inputs.JButton, NewState);
                        } break;

                        case WIN_ARROW_UP_KEY_CODE: {
                            WinProcessKey(&Inputs.ArrowUp, NewState);
                        } break;

                        case WIN_ARROW_DOWN_KEY_CODE: {
                            WinProcessKey(&Inputs.ArrowDown, NewState);
                        } break;

                        case WIN_ARROW_RIGHT_KEY_CODE: {
                            WinProcessKey(&Inputs.ArrowRight, NewState);
                        } break;

                        case WIN_ARROW_LEFT_KEY_CODE: {
                            WinProcessKey(&Inputs.ArrowLeft, NewState);
                        } break;
                    
                        default: break;
                    }
                }
                else if (VirtualKeyCode == VK_F4 && AltWasDown) {
                    Win32App.Running = false;
                }
            } break;

            case WM_RBUTTONDOWN:
		    case WM_RBUTTONUP:
		    case WM_MBUTTONDOWN:
		    case WM_MBUTTONUP:
		    case WM_MOUSEMOVE: {
                real32 x = (real32)GET_X_LPARAM(Message.lParam);
                real32 y = (real32)GET_Y_LPARAM(Message.lParam);
                Inputs.MouseInput.Moution.x = ((x * Inputs.MouseInput.NormalizedWidth)  - 1.0f) * Inputs.MouseInput.Sensitive;
                Inputs.MouseInput.Moution.y = ((y * Inputs.MouseInput.NormalizedHeight) - 1.0f) * Inputs.MouseInput.Sensitive;

                CursorCenterPos = { 
                    Win32App.ScreenOpt.CenterW, 
                    Win32App.ScreenOpt.CenterH
                };
                ClientToScreen(Win32App.Window, &CursorCenterPos);
                SetCursorPos(CursorCenterPos.x, CursorCenterPos.y);
		    } break;

            default: {
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            } break;
        }
    }
}

static Statuses WinRegClasses()
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
        return Statuses::RegisterClassFailed;
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
        return Statuses::RegisterClassFailed;
    }

    return Statuses::Success;
}

static Statuses WinCreateWindow()
{
    POINT   CursorCenterPos;
    RECT    ClientRect;
    DWORD   WindowExStyles;
    // TODO (ismail): why i need WS_MAXIMIZEBOX or WS_MINIMIZEBOX that include to WS_OVERLAPPEDWINDOW?
    DWORD   WindowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

    // TODO (ismail): if fullscreen style = WS_EX_TOPMOST
    WindowExStyles = 0;

    Win32App.ScreenOpt.Height   = WINDOW_HEIGHT;
    Win32App.ScreenOpt.Width    = WINDOW_WIDTH;

    Win32App.ScreenOpt.CenterH   = WINDOW_HEIGHT / 2;
    Win32App.ScreenOpt.CenterW   = WINDOW_WIDTH  / 2;

    // TODO (ismail): set actual x, y and width and height
    Win32App.Window = CreateWindowExA(WindowExStyles, WIN32_WINDOW_CLASS_NAME, 
                                      GAME_NAME, WindowStyle, 
                                      CW_USEDEFAULT , CW_USEDEFAULT, 
                                      Win32App.ScreenOpt.Width, Win32App.ScreenOpt.Height, 
                                      0, 0, 
                                      Win32App.AppInstance, 0);
    
    if (!Win32App.Window) {
        // TODO (ismail): file or/and console logging?
        return Statuses::WindowCreateFailed;
    }

    Win32App.ScreenOpt.AspectRatio = (real32)WINDOW_WIDTH / (real32)WINDOW_HEIGHT;

    Win32App.WindowDeviceContext = GetDC(Win32App.Window);
    if (!Win32App.WindowDeviceContext) {
        // TODO (ismail): file or/and console logging?
        return Statuses::GetDCCallFailed;
    }

    GetClientRect(Win32App.Window, &ClientRect);
    ClipCursor(&ClientRect);

    ShowCursor(FALSE);

    CursorCenterPos = { 
        Win32App.ScreenOpt.CenterW, 
        Win32App.ScreenOpt.CenterH
    };
    ClientToScreen(Win32App.Window, &CursorCenterPos);
    SetCursorPos(CursorCenterPos.x, CursorCenterPos.y);

    return Statuses::Success;
}

static void WinPlatformInit(EnginePlatform *PlatformContext)
{
    PlatformContext->AllocMem       = &WinMemoryAllocate;
    PlatformContext->ReadFile       = &WinReadFile;
    PlatformContext->FreeFileData   = &WinFreeFileData;
}

static Statuses WinInit()
{
    Statuses Result;

    if ( (Result = WinRegClasses()) != Statuses::Success) {
        return Result;
    }

    if ( (Result = WinLoadOpenGLExtensions("opengl32.dll")) != Statuses::Success) {
        return Result;
    }

    if ( (Result = WinCreateWindow()) != Statuses::Success) {
        return Result;
    }

    if ( (Result = WinInitOpenGLContext()) != Statuses::Success) {
        return Result;
    }

    return Statuses::Success;
}

void CameraTransform(GameInput * Inputs)
{
    real32 TranslationMultiplyer        = 1.0f;
    real32 TargetTranslationMultiplyer  = 1.0f;
    real32 RightTranslationMultiplyer   = 1.0f;

    Vec3 Target;
    Vec3 Right;
    Vec3 Up;
    Vec3 CameraTargetTranslation;
    Vec3 CameraRightTranslation;

    PlayerCamera.Transform.Rotate.Heading   += DEGREE_TO_RAD(Inputs->MouseInput.Moution.x * 0.003);
    PlayerCamera.Transform.Rotate.Pitch     += DEGREE_TO_RAD(Inputs->MouseInput.Moution.y * 0.003);

    RotationToVectors(&PlayerCamera.Transform.Rotate, &Target, &Right, &Up);

    if ((Inputs->WButton.State || Inputs->SButton.State) && (Inputs->AButton.State || Inputs->DButton.State)) {
        TranslationMultiplyer = 0.707f; // NOTE (ismail): 1 / square root of 2
    }

    TargetTranslationMultiplyer     = TranslationMultiplyer * ((real32)Inputs->WButton.State + (-1.0f * (real32)Inputs->SButton.State)); // 1.0 if W Button -1.0 if S Button and 0 if W and S Button pressed together

    CameraTargetTranslation         = Target * (PlayerCamera.Movement.Speed * 0.003 * TargetTranslationMultiplyer);

    RightTranslationMultiplyer      = TranslationMultiplyer * ((real32)Inputs->DButton.State + (-1.0f * (real32)Inputs->AButton.State));

    CameraRightTranslation          = Right * (PlayerCamera.Movement.Speed * 0.003 * RightTranslationMultiplyer);

    PlayerCamera.Transform.Position += CameraTargetTranslation + CameraRightTranslation;
}

void PlayerTransform(GameInput* Inputs)
{
    real32 TranslationMultiplyer        = 1.0f;
    real32 TargetTranslationMultiplyer  = 1.0f;
    real32 RightTranslationMultiplyer   = 1.0f;

    Vec3 Target;
    Vec3 Right;
    Vec3 Up;
    Vec3 TargetTranslation;
    Vec3 RightTranslation;

    WorldObject *PlayerObject = &WorldObjects.Objects[PLAYER_INDEX];
    
    PlayerObject->Transform.Rotate.Pitch    += DEGREE_TO_RAD(PlayerObject->Movement.RotationDelta * ((real32)Inputs->IButton.State + (-1.0f * (real32)Inputs->KButton.State)) * 0.03);
    PlayerObject->Transform.Rotate.Heading  += DEGREE_TO_RAD(PlayerObject->Movement.RotationDelta * ((real32)Inputs->LButton.State + (-1.0f * (real32)Inputs->JButton.State)) * 0.03);

    RotationToVectors(&PlayerObject->Transform.Rotate, &Target, &Right, &Up);

    if ((Inputs->WButton.State || Inputs->SButton.State) && (Inputs->AButton.State || Inputs->DButton.State)) {
        TranslationMultiplyer = 0.707f; // NOTE (ismail): 1 / square root of 2
    }

    TargetTranslationMultiplyer      = TranslationMultiplyer * ((real32)Inputs->ArrowUp.State + (-1.0f * (real32)Inputs->ArrowDown.State)); // 1.0 if W Button -1.0 if S Button and 0 if W and S Button pressed together
    TargetTranslation                = Target * (PlayerObject->Movement.Speed * 0.003 * TargetTranslationMultiplyer);

    RightTranslationMultiplyer       = TranslationMultiplyer * ((real32)Inputs->ArrowRight.State + (-1.0f * (real32)Inputs->ArrowLeft.State)); // 1.0 if D Button -1.0 if A Button and 0 if D and A Button pressed together
    RightTranslation                 = Right * (PlayerObject->Movement.Speed * 0.003 * RightTranslationMultiplyer);

    PlayerObject->Transform.Position += TargetTranslation + RightTranslation;

    PlayerObject->BoundingVolume.VolumeData.OrientedBox.Axis[0] = Right;
    PlayerObject->BoundingVolume.VolumeData.OrientedBox.Axis[1] = Up;
    PlayerObject->BoundingVolume.VolumeData.OrientedBox.Axis[2] = Target;

    PlayerObject->BoundingVolume.VolumeData.OrientedBox.Center = PlayerObject->Transform.Position;

    PlayerObject->Material.FlatColor = { 1.0f, 1.0f, 1.0f };

    for (i32 ObjectIndex = PLAYER_INDEX + 1; ObjectIndex < WorldObjects.ObjectsAmount; ++ObjectIndex) {
        WorldObject *SceneObject = &WorldObjects.Objects[ObjectIndex];
        
        if (SceneObject->BoundingVolume.VolumeType == BoundingVolumeType::OBBVolume) {
            RotationToVectors(&SceneObject->Transform.Rotate, &Target, &Right, &Up);

            SceneObject->BoundingVolume.VolumeData.OrientedBox.Axis[0] = Right;
            SceneObject->BoundingVolume.VolumeData.OrientedBox.Axis[1] = Up;
            SceneObject->BoundingVolume.VolumeData.OrientedBox.Axis[2] = Target;

            SceneObject->BoundingVolume.VolumeData.OrientedBox.Center = SceneObject->Transform.Position;

            if (OBBToOBBTestOverlap(&PlayerObject->BoundingVolume.VolumeData.OrientedBox, &SceneObject->BoundingVolume.VolumeData.OrientedBox)) {
                PlayerObject->Material.FlatColor = { 0.0f, 1.0f, 0.0f };
            }
        }
    }
}

void TransformFrame()
{
    CameraTransform(&Inputs);
    PlayerTransform(&Inputs);
}

// Mat4x4 OrthoProjection = MakeOrtographProjection(-2.0f * AspectRatio, 2.0f * AspectRatio, -2.0f, 2.0f, 0.1f, 100.0f);
void RendererFrame()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Vec3   CameraInversePosition = PlayerCamera.Transform.Position * -1.0f;

    Mat4x4 ObjectToCameraSpaceTranslation       = MakeTranslation(&CameraInversePosition);
    Mat4x4 ObjectToCameraSpaceRotation          = MakeRotation4x4Inverse(&PlayerCamera.Transform.Rotate);
    Mat4x4 ObjectToCameraSpaceTransformation    = ObjectToCameraSpaceRotation * ObjectToCameraSpaceTranslation;

    for (i32 ObjectIndex = 0; ObjectIndex < WorldObjects.ObjectsAmount; ++ObjectIndex) {
        WorldObject*            Object                  = &WorldObjects.Objects[ObjectIndex];
        ObjectRenderingContext* ObjectRenderingContext  = &WorldObjectsRendererContext.ObjectsRenderingContext[Object->RendererContextIndex];
        ShaderProgram*          ShaderProgram           = &WorldShaderPrograms.ShaderPrograms[ObjectRenderingContext->ShaderProgramIndex];

        if (!Object->Material.RenderInNextFrame) {
            continue;
        }

        switch (ShaderProgram->Type) {
            case ShaderProgramTypes::StaticObjectShader: {
                StaticObjectRenderingShader_Setup *Setup = &ShaderProgram->Setup.ShaderSetup.StaticObjectShader;

                Mat4x4 ObjectRotation       = MakeRotation4x4(&Object->Transform.Rotate);
                Mat4x4 ObjectTranslation    = MakeTranslation(&Object->Transform.Position);
                Mat4x4 ObjectScale          = Identity4x4;

                Setup->VAO                          = ObjectRenderingContext->Buffers.VAO;
                Setup->TextureHandle                = Object->Material.Texture.TextureHandle;
                Setup->TextureSamplerID             = GL_TEXTURE_UNIT0;
                Setup->ObjectFlatColor              = Object->Material.FlatColor;
                Setup->ObjectRotation               = ObjectRotation;
                Setup->ObjectScale                  = ObjectScale;
                Setup->ObjectTranslation            = ObjectTranslation;
                Setup->ObjToCameraSpaceTransform    = ObjectToCameraSpaceTransformation;

            } break;

            default: {
                Assert(0);
            }
        }
        
        ProcessShader(ShaderProgram);
        
        tglDrawElementsBaseVertex(GL_TRIANGLES, ObjectRenderingContext->ObjectFile.IndicesCount, GL_UNSIGNED_INT, 0, 0);
    }

    SwapBuffers(Win32App.WindowDeviceContext);

    tglUseProgram(0);
    tglBindVertexArray(0);
}

void Frame()
{
    TransformFrame();
    RendererFrame();
/*
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

        AABB TransformedPlayerAABB = {};
        AABBRecalculate(&PlayerRotation, &PlayerSphere.Transform.Position, &PlayerCube.BoundingBox, &TransformedPlayerAABB);

        AABB TransformedNpcAABB = {};
        AABBRecalculate(&NpcRotation, &NpcCube.Transform.Position, &NpcCube.BoundingBox, &TransformedNpcAABB);

        PlayerSphere.BoundingSphere.Center = PlayerSphere.Transform.Position;
        NpcSphere.BoundingSphere.Center = NpcSphere.Transform.Position;

        if (SphereToSphereTestOverlap(&PlayerSphere.BoundingSphere, &NpcSphere.BoundingSphere)) {
            PlayerSphereColor.x = 0.0f;
            PlayerSphereColor.y = 1.0f;
            PlayerSphereColor.z = 0.0f;
        }

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



        SDL_GL_SwapWindow(Window);
    }
    */
}

Statuses WorldPrepare(EnginePlatform *Platform)
{    
    // NOTE (ismail): i disable textures for now that call need for load texture
    // GLuint TextureObject = LoadTexture("bricks_textures.jpg");
    // NOTE (ismail): these calls need later for put it in screen
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, TextureObject);
    // glUniform1i(SamplerLocation, 0);

    // TODO (ismail): move perspective projection matrix calculs to another place
    WorldObjectsRendererContext.PerspectiveProjection = MakePerspectiveProjection(60.0f, Win32App.ScreenOpt.AspectRatio, 0.01f, 50.0f);

    // LIGHTNING SETUP

    WorldAmbientLight.Color        = { 1.0f, 1.0f, 1.0f };
    WorldAmbientLight.Intensity    = 0.1f;

    WorldDirectionLight.Direction   = { 0.0f, 0.0f, 1.0f };
    WorldDirectionLight.Color       = { 1.0f, 1.0f, 1.0f };
    WorldDirectionLight.Intensity   = 0.75f;

    // LIGHTNING SETUP END

    // SHADERS PROGRAMS

    ShaderProgram* Shader = &WorldShaderPrograms.ShaderPrograms[0];

    Shader->Type = ShaderProgramTypes::StaticObjectShader;

    ShaderSetupOption ShaderSetups[2] = {};

    ShaderSetups[0].LoadFromFile    = 1;
    
    ShaderSetups[0].FileName        = "data/shaders/common_object_shader.vs";
    ShaderSetups[0].Type            = ShaderType::VertexShader;

    ShaderSetups[1].LoadFromFile    = 1;

    ShaderSetups[1].FileName        = "data/shaders/common_object_shader.fs";
    ShaderSetups[1].Type            = ShaderType::FragmentShader;

    SetupShader(Platform, Shader, ShaderSetups, 2);

    StaticObjectRenderingShader_Setup *Setup = &Shader->Setup.ShaderSetup.StaticObjectShader;

    Setup->AmbLight     = WorldAmbientLight;
    Setup->DirLight     = WorldDirectionLight;
    Setup->PerspProj    = WorldObjectsRendererContext.PerspectiveProjection;

    // SHADERS PROGRAMS END

    // TEXTURE FILE LOADING
    TextureObject TextObj;

    LoadTexture2D("data/textures/bricks_textures.jpg", &TextObj);

    // END TEXTURE FILE LOADING

    // OBJ FILE LOADING

    // TODO (ismail): just for now in future i must rewrite it
    // and also i don't deallocate this buffers now
    WorldObjectsRendererContext.ObjectsRenderingContext[0].ObjectFile.Positions = (Vec3*)VirtualAlloc(0, sizeof(Vec3) * 30000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    WorldObjectsRendererContext.ObjectsRenderingContext[0].ObjectFile.Normals = (Vec3*)VirtualAlloc(0, sizeof(Vec3) * 30000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    WorldObjectsRendererContext.ObjectsRenderingContext[0].ObjectFile.TextureCoord = (Vec2*)VirtualAlloc(0, sizeof(Vec2) * 30000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    WorldObjectsRendererContext.ObjectsRenderingContext[0].ObjectFile.Indices = (u32*)VirtualAlloc(0, sizeof(u32) * 30000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    WorldObjectsRendererContext.ObjectsRenderingContext[1].ObjectFile.Positions = (Vec3*)VirtualAlloc(0, sizeof(Vec3) * 30000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    WorldObjectsRendererContext.ObjectsRenderingContext[1].ObjectFile.Normals = (Vec3*)VirtualAlloc(0, sizeof(Vec3) * 30000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    WorldObjectsRendererContext.ObjectsRenderingContext[1].ObjectFile.TextureCoord = (Vec2*)VirtualAlloc(0, sizeof(Vec2) * 30000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    WorldObjectsRendererContext.ObjectsRenderingContext[1].ObjectFile.Indices = (u32*)VirtualAlloc(0, sizeof(u32) * 30000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    ObjFileLoaderFlags LoadFlags {
        /*GenerateSmoothNormals*/   1,  
        /*SelfGenerateNormals*/     0
    };

    LoadObjFile("data/obj/cuber_textured_normals.obj", &WorldObjectsRendererContext.ObjectsRenderingContext[0].ObjectFile, LoadFlags);

    LoadFlags.GenerateSmoothNormals = 0;

    LoadObjFile("data/obj/cube.obj", &WorldObjectsRendererContext.ObjectsRenderingContext[1].ObjectFile, LoadFlags);

    LoadObjectToHardware(&WorldObjectsRendererContext.ObjectsRenderingContext[0].Buffers, &WorldObjectsRendererContext.ObjectsRenderingContext[0].ObjectFile);
    LoadObjectToHardware(&WorldObjectsRendererContext.ObjectsRenderingContext[1].Buffers, &WorldObjectsRendererContext.ObjectsRenderingContext[1].ObjectFile);

    WorldObjectsRendererContext.ObjectsRenderingContext[0].ShaderProgramIndex = 0;
    WorldObjectsRendererContext.ObjectsRenderingContext[1].ShaderProgramIndex = 0;

    // AMOUNT OF OBJECTS MESHES
    WorldObjectsRendererContext.ObjectsAmount = 2;

    // OBJ FILE LOADING END

    // SCENE OBJECTS

    PlayerCamera.Movement.Speed = 5.0f;
    PlayerCamera.Movement.RotationDelta = 1.0f;

    WorldObjects.Objects[PLAYER_INDEX].Transform.Position.x = 1.5f;
    WorldObjects.Objects[PLAYER_INDEX].Transform.Position.y = 0.0f;
    WorldObjects.Objects[PLAYER_INDEX].Transform.Position.z = 2.0f;

    WorldObjects.Objects[PLAYER_INDEX].Transform.Scale.x = 1.0f;
    WorldObjects.Objects[PLAYER_INDEX].Transform.Scale.y = 1.0f;
    WorldObjects.Objects[PLAYER_INDEX].Transform.Scale.z = 1.0f;

    WorldObjects.Objects[PLAYER_INDEX].BoundingVolume.VolumeType = BoundingVolumeType::OBBVolume;
    WorldObjects.Objects[PLAYER_INDEX].BoundingVolume.VolumeData.OrientedBox.Center = WorldObjects.Objects[PLAYER_INDEX].Transform.Position;

    WorldObjects.Objects[PLAYER_INDEX].BoundingVolume.VolumeData.OrientedBox.Axis[0] = { 1.0f, 0.0f, 0.0f };
    WorldObjects.Objects[PLAYER_INDEX].BoundingVolume.VolumeData.OrientedBox.Axis[1] = { 0.0f, 1.0f, 0.0f };;
    WorldObjects.Objects[PLAYER_INDEX].BoundingVolume.VolumeData.OrientedBox.Axis[2] = { 0.0f, 0.0f, 1.0f };;

    WorldObjects.Objects[PLAYER_INDEX].BoundingVolume.VolumeData.OrientedBox.Extens.x = 1.0f; // TODO (ismail): initial AABB compute need to be implemented
    WorldObjects.Objects[PLAYER_INDEX].BoundingVolume.VolumeData.OrientedBox.Extens.y = 1.0f; // TODO (ismail): initial AABB compute need to be implemented
    WorldObjects.Objects[PLAYER_INDEX].BoundingVolume.VolumeData.OrientedBox.Extens.z = 1.0f; // TODO (ismail): initial AABB compute need to be implemented

    WorldObjects.Objects[PLAYER_INDEX].Movement.Speed = 5.0f;
    WorldObjects.Objects[PLAYER_INDEX].Movement.RotationDelta = 1.0f;

    WorldObjects.Objects[PLAYER_INDEX].Material.Texture             = TextObj;
    WorldObjects.Objects[PLAYER_INDEX].Material.FlatColor           = { 1.0f, 1.0f, 1.0f };
    WorldObjects.Objects[PLAYER_INDEX].Material.RenderInNextFrame   = 1;

    WorldObjects.Objects[PLAYER_INDEX].RendererContextIndex = 0;

    WorldObjects.Objects[1].Transform.Position.x = -1.5f;
    WorldObjects.Objects[1].Transform.Position.y = 0.0f;
    WorldObjects.Objects[1].Transform.Position.z = 2.0f;

    WorldObjects.Objects[1].Transform.Scale.x = 1.0f;
    WorldObjects.Objects[1].Transform.Scale.y = 1.0f;
    WorldObjects.Objects[1].Transform.Scale.z = 1.0f;

    WorldObjects.Objects[1].BoundingVolume.VolumeType = BoundingVolumeType::OBBVolume;
    WorldObjects.Objects[1].BoundingVolume.VolumeData.OrientedBox.Center = WorldObjects.Objects[1].Transform.Position;

    WorldObjects.Objects[1].BoundingVolume.VolumeData.OrientedBox.Axis[0] = { 1.0f, 0.0f, 0.0f };
    WorldObjects.Objects[1].BoundingVolume.VolumeData.OrientedBox.Axis[1] = { 0.0f, 1.0f, 0.0f };;
    WorldObjects.Objects[1].BoundingVolume.VolumeData.OrientedBox.Axis[2] = { 0.0f, 0.0f, 1.0f };;

    WorldObjects.Objects[1].BoundingVolume.VolumeData.OrientedBox.Extens.x = 1.0f; // TODO (ismail): initial AABB compute need to be implemented
    WorldObjects.Objects[1].BoundingVolume.VolumeData.OrientedBox.Extens.y = 1.0f; // TODO (ismail): initial AABB compute need to be implemented
    WorldObjects.Objects[1].BoundingVolume.VolumeData.OrientedBox.Extens.z = 1.0f; // TODO (ismail): initial AABB compute need to be implemented

    WorldObjects.Objects[1].Material.Texture            = TextObj;
    WorldObjects.Objects[1].Material.FlatColor          = { 1.0f, 1.0f, 1.0f };
    WorldObjects.Objects[1].Material.RenderInNextFrame  = 1;

    WorldObjects.Objects[1].Movement.Speed = 5.0f;
    WorldObjects.Objects[1].Movement.RotationDelta = 1.0f;

    WorldObjects.Objects[1].RendererContextIndex = 0;

    // AMOUNT OF OBJECTS ON SCENE
    WorldObjects.ObjectsAmount = 2;

    Inputs.MouseInput.NormalizedHeight = (1.0f / Win32App.ScreenOpt.Height) * 2.0f;
    Inputs.MouseInput.NormalizedWidth = (1.0f / Win32App.ScreenOpt.Width) * 2.0f;

    // TODO (ismail): look at this thing, sensitive 5000 sounds weired
    Inputs.MouseInput.Sensitive = 5000.0f;

    return Statuses::Success;
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
    AssetsLoaderVars    AssetsLoadVars;
    EnginePlatform      WinPlatform;
    Statuses            InitializationStatuses;
    MSG                 Message;

    LARGE_INTEGER PerfomanceCountFrequencyResult;
    QueryPerformanceFrequency(&PerfomanceCountFrequencyResult);
    i64 PerfCountFrequency = PerfomanceCountFrequencyResult.QuadPart;

    Win32App.Running        = true;
    Win32App.AppInstance    = Instance;

    if ( (InitializationStatuses = WinInit()) != Statuses::Success) {
        // TODO (ismail): diagnostics things?
        return InitializationStatuses;
    }

    if ( (InitializationStatuses = LoadGLFunctions()) != Statuses::Success) {
        // TODO (ismail): diagnostics things?
        return InitializationStatuses;
    }

    WinPlatformInit(&WinPlatform);

    RendererInit();

    AssetsLoadVars.AssetsLoaderCacheSize = 100000;
    AssetsLoaderInit(&WinPlatform, &AssetsLoadVars);

    if ( (InitializationStatuses = WorldPrepare(&WinPlatform)) != Statuses::Success) {
        // TODO (ismail): diagnostics things?
        return InitializationStatuses;
    }

    LARGE_INTEGER LastCounter;
    QueryPerformanceCounter(&LastCounter);
    u64 LastCycleCount = __rdtsc();

    while (Win32App.Running) {
        WinProcessMessages();
        Frame();

        // NOTE(ismail): some very usefull thing for perfomance debuging
        u64 EndCycleCount = __rdtsc();

        LARGE_INTEGER EndCounter;
        QueryPerformanceCounter(&EndCounter);

        u64 CyclesElapsed   = EndCycleCount - LastCycleCount;
        i64 CounterElapsed  = EndCounter.QuadPart - LastCounter.QuadPart;
        real64 DeltaTime    = (1000.0f * ((real64)CounterElapsed / (real64)PerfCountFrequency)); // millisecond per frame
        real64 FPS          = (1000.0f / DeltaTime); // frame per seconds
        real64 MCPF         = (((real64)CyclesElapsed) / (1000.0f * 1000.0f)); // mega cycles per frame, how many cycles on CPU take last frame check rdtsc and hh ep 10

        char Buffer[256];
        snprintf(Buffer, sizeof(Buffer), "| %.02fms/f | %.02f f/s | %.02f mc/f |\n", DeltaTime, FPS, MCPF);

        OutputDebugStringA(Buffer);

        LastCycleCount = EndCycleCount;
        LastCounter = EndCounter;
    }

    return 0;
}