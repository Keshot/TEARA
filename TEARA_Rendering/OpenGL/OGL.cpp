#include "OGL.h"
#include <SDL3/SDL_video.h>

PFNGLGENBUFFERSPROC                  glGenBuffers                   = NULL;
PFNGLBINDBUFFERPROC                  glBindBuffer                   = NULL;
PFNGLBUFFERDATAPROC                  glBufferData                   = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC     glEnableVertexAttribArray      = NULL;
PFNGLVERTEXATTRIBPOINTERPROC         glVertexAttribPointer          = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC    glDisableVertexAttribArray     = NULL;
PFNGLCREATEPROGRAMPROC               glCreateProgram                = NULL;
PFNGLCREATESHADERPROC                glCreateShader                 = NULL;
PFNGLSHADERSOURCEPROC                glShaderSource                 = NULL;
PFNGLCOMPILESHADERPROC               glCompileShader                = NULL;
PFNGLGETSHADERIVPROC                 glGetShaderiv                  = NULL;
PFNGLATTACHSHADERPROC                glAttachShader                 = NULL;
PFNGLLINKPROGRAMPROC                 glLinkProgram                  = NULL;
PFNGLGETPROGRAMIVPROC                glGetProgramiv                 = NULL;
PFNGLVALIDATEPROGRAMPROC             glValidateProgram              = NULL;
PFNGLUSEPROGRAMPROC                  glUseProgram                   = NULL;
PFNGLGETPROGRAMINFOLOGPROC           glGetProgramInfoLog            = NULL;
PFNGLGETUNIFORMLOCATIONPROC          glGetUniformLocation           = NULL;
PFNGLUNIFORM1FPROC                   glUniform1f                    = NULL;
PFNGLUNIFORMMATRIX4FVPROC            glUniformMatrix4fv             = NULL;
PFNGLUNIFORM1IPROC                   glUniform1i                    = NULL;
PFNGLACTIVETEXTUREPROC               glActiveTextureStb             = NULL;
PFNGLGENVERTEXARRAYSPROC             glGenVertexArrays              = NULL;
PFNGLBINDVERTEXARRAYPROC             glBindVertexArray              = NULL;
PFNGLUNIFORM3FVPROC                  glUniform3fv                   = NULL;

// TODO (ismail): more complecated initialization
i32 OGLInit()
{
     // TODO (ismail): move all of this staff into separate functions and we must create windows and then load library!!!
    if (SDL_GL_LoadLibrary("opengl32.dll") < 0) {
        // TODO (ismail): diagnostic
        return OGL_STATS::LOAD_FAILURE;
    }

    // TODO (ismail): check available opengl version
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    // NOTE (ismail): it must be 24 if not opengl will be intialized with 1.1.0 version
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    return OGL_STATS::LOAD_SUCCESS;
}

i32 OGLLoadFunctions()
{
    glGenBuffers = (PFNGLGENBUFFERSPROC) SDL_GL_GetProcAddress("glGenBuffers");
    if (!glGenBuffers) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glBindBuffer = (PFNGLBINDBUFFERPROC) SDL_GL_GetProcAddress("glBindBuffer");
    if (!glBindBuffer) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glBufferData = (PFNGLBUFFERDATAPROC) SDL_GL_GetProcAddress("glBufferData");
    if (!glBufferData) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) SDL_GL_GetProcAddress("glEnableVertexAttribArray");
    if (!glEnableVertexAttribArray) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) SDL_GL_GetProcAddress("glVertexAttribPointer");
    if (!glVertexAttribPointer) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) SDL_GL_GetProcAddress("glDisableVertexAttribArray");
    if (!glDisableVertexAttribArray) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glCreateProgram = (PFNGLCREATEPROGRAMPROC) SDL_GL_GetProcAddress("glCreateProgram");
    if (!glCreateProgram) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glCreateShader = (PFNGLCREATESHADERPROC) SDL_GL_GetProcAddress("glCreateShader");
    if (!glCreateShader) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glShaderSource = (PFNGLSHADERSOURCEPROC) SDL_GL_GetProcAddress("glShaderSource");
    if (!glShaderSource) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glCompileShader = (PFNGLCOMPILESHADERPROC) SDL_GL_GetProcAddress("glCompileShader");
    if (!glCompileShader) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glGetShaderiv = (PFNGLGETSHADERIVPROC) SDL_GL_GetProcAddress("glGetShaderiv");
    if (!glGetShaderiv) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glAttachShader = (PFNGLATTACHSHADERPROC) SDL_GL_GetProcAddress("glAttachShader");
    if (!glAttachShader) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glLinkProgram = (PFNGLLINKPROGRAMPROC) SDL_GL_GetProcAddress("glLinkProgram");
    if (!glLinkProgram) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glGetProgramiv = (PFNGLGETPROGRAMIVPROC) SDL_GL_GetProcAddress("glGetProgramiv");
    if (!glGetProgramiv) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glValidateProgram = (PFNGLVALIDATEPROGRAMPROC) SDL_GL_GetProcAddress("glValidateProgram");
    if (!glValidateProgram) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glUseProgram = (PFNGLUSEPROGRAMPROC) SDL_GL_GetProcAddress("glUseProgram");
    if (!glUseProgram) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) SDL_GL_GetProcAddress("glGetProgramInfoLog");
    if (!glGetProgramInfoLog) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) SDL_GL_GetProcAddress("glGetUniformLocation"); 
    if (!glGetUniformLocation) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glUniform1f = (PFNGLUNIFORM1FPROC) SDL_GL_GetProcAddress("glUniform1f");
    if (!glUniform1f) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) SDL_GL_GetProcAddress("glUniformMatrix4fv");
    if (!glUniformMatrix4fv) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glUniform1i = (PFNGLUNIFORM1IPROC) SDL_GL_GetProcAddress("glUniform1i");
    if (!glUniform1i) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glActiveTexture = (PFNGLACTIVETEXTUREPROC) SDL_GL_GetProcAddress("glActiveTexture");
    if (!glActiveTexture) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) SDL_GL_GetProcAddress("glGenVertexArrays");
    if (!glGenVertexArrays) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC) SDL_GL_GetProcAddress("glBindVertexArray");
    if (!glGenVertexArrays) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    glUniform3fv = (PFNGLUNIFORM3FVPROC) SDL_GL_GetProcAddress("glUniform3fv");
    if (!glUniform3fv) {
        // TODO (ismail): diagnostic
        return OGL_STATS::FUNCTION_LOAD_FAILURE;
    }

    return OGL_STATS::LOAD_SUCCESS;
}