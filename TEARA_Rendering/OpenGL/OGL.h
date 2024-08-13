#ifndef _TEARA_OPENGL_H_
#define _TEARA_OPENGL_H_

#include "Types.h"
#include <SDL3/SDL_opengl.h>

enum OGL_STATS {
    LOAD_SUCCESS = 0,
    LOAD_FAILURE = -1,
    FUNCTION_LOAD_FAILURE = -2
};

extern PFNGLGENBUFFERSPROC                  glGenBuffers;
extern PFNGLBINDBUFFERPROC                  glBindBuffer;
extern PFNGLBUFFERDATAPROC                  glBufferData;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC     glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC         glVertexAttribPointer;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC    glDisableVertexAttribArray;
extern PFNGLCREATEPROGRAMPROC               glCreateProgram;
extern PFNGLCREATESHADERPROC                glCreateShader;
extern PFNGLSHADERSOURCEPROC                glShaderSource;
extern PFNGLCOMPILESHADERPROC               glCompileShader;
extern PFNGLGETSHADERIVPROC                 glGetShaderiv;
extern PFNGLATTACHSHADERPROC                glAttachShader;
extern PFNGLLINKPROGRAMPROC                 glLinkProgram;
extern PFNGLGETPROGRAMIVPROC                glGetProgramiv;
extern PFNGLVALIDATEPROGRAMPROC             glValidateProgram;
extern PFNGLUSEPROGRAMPROC                  glUseProgram;
extern PFNGLGETPROGRAMINFOLOGPROC           glGetProgramInfoLog;
extern PFNGLGETUNIFORMLOCATIONPROC          glGetUniformLocation;
extern PFNGLUNIFORM1FPROC                   glUniform1f;
extern PFNGLUNIFORMMATRIX4FVPROC            glUniformMatrix4fv;
extern PFNGLUNIFORM1IPROC                   glUniform1i;
extern PFNGLACTIVETEXTUREPROC               glActiveTextureStb;
extern PFNGLGENVERTEXARRAYSPROC             glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC             glBindVertexArray;
extern PFNGLUNIFORM3FVPROC                  glUniform3fv;

i32 OGLInit();
i32 OGLLoadFunctions();

#define glActiveTexture glActiveTextureStb

#endif