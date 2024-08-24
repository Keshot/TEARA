#include <windows.h>

// TODO (ismail): put it to rendering layer of engine
#include <gl/GL.h>

#include "TEARA_Utils/Types.h"

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

#define GLAPIENTRY __stdcall*

static bool Running;

enum WinLocalErrors { 
    RegisterClassFailed = -1,
    WindowCreateFailed = -2,
};

enum OpenGLLocalErrors { 
    LibraryDllLoadFailed            = -1,
    FunctionLoadFailed              = -2,
    WinRegisterFakeClassFailed      = -3,
    WinCreateFakeWindowFailed       = -4,
    WinFakeGLContextFailed          = -5,
};

typedef HGLRC   (GLAPIENTRY TEARA_glCreateContextAttribsARB)(HDC, HGLRC, const int*);
typedef BOOL    (GLAPIENTRY TEARA_glChoosePixelFormatARB)(HDC,const int*, const FLOAT*,
                                                          UINT, int*, UINT*);

static TEARA_glCreateContextAttribsARB  tglCreateContextAttribsARB;
static TEARA_glChoosePixelFormatARB     tglChoosePixelFormatARB;

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
    return DefWindowProcA(Window, Message, WParam, LParam);
}

static i32 WinLoadOpenGLExtensions(HINSTANCE Instance, const char *DllName)
{
    MSG                     Message;
    HGLRC                   FakeGLContext;
    HDC                     FakeDeviceContext;
    i32                     SuggestedPixelFormatIndex;
    PIXELFORMATDESCRIPTOR   SuggestedPixelFormat, DesiredPixelFormat;
    ATOM                    FakeWinClassAtom;
    WNDCLASSA               FakeWinClass;
    HWND                    FakeWindow;

    FakeWinClass.style          = CS_OWNDC;
    FakeWinClass.lpfnWndProc    = &WinFakeMainCallback;
    FakeWinClass.cbClsExtra     = 0;
    FakeWinClass.cbWndExtra     = 0;
    FakeWinClass.hInstance      = Instance;
    FakeWinClass.hIcon          = 0;
    FakeWinClass.hCursor        = 0;
    FakeWinClass.hbrBackground  = 0; // TODO (ismail): check if i need this
    FakeWinClass.lpszMenuName   = 0;
    FakeWinClass.lpszClassName  = WIN32_FAKE_CLASS_NAME;

    FakeWinClassAtom = RegisterClassA(&FakeWinClass);

    if (!FakeWinClassAtom) {
        // TODO (ismail): file or/and console logging? and Assert
        return OpenGLLocalErrors::WinRegisterFakeClassFailed;
    }

    FakeWindow = CreateWindowA(WIN32_FAKE_CLASS_NAME, GAME_NAME, 
                               WS_OVERLAPPEDWINDOW, 
                               CW_USEDEFAULT, CW_USEDEFAULT, 
                               640, 480, 
                               NULL, NULL, 
                               Instance, NULL);
    
    if (!FakeWindow) {
        // TODO (ismail): file or/and console logging? and Assert
        return OpenGLLocalErrors::WinCreateFakeWindowFailed;
    }

    FakeDeviceContext = GetDC(FakeWindow);

    WinGetDesiredPixelFormat(&DesiredPixelFormat);

    SuggestedPixelFormatIndex = ChoosePixelFormat(FakeDeviceContext, &DesiredPixelFormat);
    DescribePixelFormat(FakeDeviceContext, SuggestedPixelFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), &SuggestedPixelFormat);
    SetPixelFormat(FakeDeviceContext, SuggestedPixelFormatIndex, &SuggestedPixelFormat);

    FakeGLContext = wglCreateContext(FakeDeviceContext);
    if (!wglMakeCurrent(FakeDeviceContext, FakeGLContext)) {
        // TODO (ismail): file or/and console logging? and Assert
        return OpenGLLocalErrors::WinFakeGLContextFailed;
    }

    tglCreateContextAttribsARB = (TEARA_glCreateContextAttribsARB) wglGetProcAddress("wglCreateContextAttribsARB");
    if (!tglCreateContextAttribsARB) {
        // TODO (ismail): file or/and console logging and Assert?
        return OpenGLLocalErrors::FunctionLoadFailed;
    }

    tglChoosePixelFormatARB = (TEARA_glChoosePixelFormatARB) wglGetProcAddress("wglChoosePixelFormatARB");
    if (!tglChoosePixelFormatARB) {
        // TODO (ismail): file or/and console logging and Assert?
        return OpenGLLocalErrors::FunctionLoadFailed;
    }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(FakeGLContext);

    // NOTE (ismail): we do not need call a ReleaseDC because our window class was created with CS_OWNDC
    DestroyWindow(FakeWindow);

    while (GetMessage(&Message, NULL, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    UnregisterClassA(WIN32_FAKE_CLASS_NAME, Instance);
}

static void WinInitOpenGLContext(HWND Window)
{
    int                     SuggestedPixelFormatIndex;
    HGLRC                   GLDeviceContext;
    PIXELFORMATDESCRIPTOR   DesiredPixelFormat, SuggestedPixelFormat;
    HDC                     DeviceContext = GetDC(Window);

    if (tglChoosePixelFormatARB && false) {
        // TODO (ismail): set here more complicated PixelFormat with tglChoosePixelFormatARB
    }
    else {
        WinGetDesiredPixelFormat(&DesiredPixelFormat);

        SuggestedPixelFormatIndex = ChoosePixelFormat(DeviceContext, &DesiredPixelFormat);
        DescribePixelFormat(DeviceContext, SuggestedPixelFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), &SuggestedPixelFormat);
    }

    SetPixelFormat(DeviceContext, SuggestedPixelFormatIndex, &SuggestedPixelFormat);

    
    GLDeviceContext = wglCreateContext(DeviceContext);
    if (!wglMakeCurrent(DeviceContext, GLDeviceContext)) {
        // TODO (ismail): file or/and console logging and Assert?
        return;
    }

    // NOTE (ismail): we do not need call a ReleaseDC because our window class was created with CS_OWNDC
}

static void WinInitOpenGL(HINSTANCE Instance, HWND Window)
{
    WinLoadOpenGLExtensions(Instance, "opengl32.dll");
    WinInitOpenGLContext(Window);
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
            Running = false;
            
            OutputDebugStringA("WM_CLOSE\n"); // TODO(ismail): change it on to some log in file or console, maybe only if we build in debug mode?
        } break;

        case WM_ACTIVATEAPP: {
            OutputDebugStringA("WM_ACTIVATEAPP\n"); // TODO(ismail): change it on to some log in file or console, maybe only if we build in debug mode?
        } break;
        
        case WM_DESTROY: {
            // TODO(ismail): hande this as an error, recreate window?
            Running = false;

            OutputDebugStringA("WM_DESTROY\n"); // TODO(ismail): change it on to some log in file or console, maybe only if we build in debug mode?
        } break;
        default: {
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        } break;
    }

    return Result;
}

i32 APIENTRY WinMain( HINSTANCE Instance, HINSTANCE PrevInstance, 
                      LPSTR CommandLine , int ShowCode)
{
    ATOM        WinClassAtom;
    WNDCLASSA   WinClass;
    HWND        Window;
    DWORD       WindowExStyles;
    // TODO (ismail): why i need WS_MAXIMIZEBOX or WS_MINIMIZEBOX that include to WS_OVERLAPPEDWINDOW?
    DWORD       WindowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

    Running = true;

    WinLoadOpenGL(Instance, "opengl32.dll");

    // TODO (ismail): add icon when we will be have one (WindowClass.hIcon)
    // TODO (ismail): add cursor when we will be have one (WindowClass.hCursor)
    WinClass.style          = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    WinClass.lpfnWndProc    = &WinMainCallback;
    WinClass.cbClsExtra     = 0;
    WinClass.cbWndExtra     = 0;
    WinClass.hInstance      = Instance;
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

    // TODO (ismail): if fullscreen style = WS_EX_TOPMOST
    WindowExStyles = 0;

    // TODO (ismail): set actual x, y and width and height
    Window = CreateWindowExA(WindowExStyles, WIN32_WINDOW_CLASS_NAME, GAME_NAME, 
                             WindowStyle, CW_USEDEFAULT , CW_USEDEFAULT, 
                             CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, 
                             Instance, 0);
    
    if (!Window) {
        // TODO (ismail): file or/and console logging?
        return WinLocalErrors::WindowCreateFailed;
    }

    return 0;
}
