#ifndef _TEARA_OPENGL_WIN_H_
#define _TEARA_OPENGL_WIN_H_

#include <windows.h>
#include <gl/GL.h>

#include "TEARA_Lib/Utils/Types.h"

#define GLAPIENTRY __stdcall*

#define WGL_CONTEXT_MAJOR_VERSION_ARB               (0x2091)
#define WGL_CONTEXT_MINOR_VERSION_ARB               (0x2092)
#define WGL_CONTEXT_PROFILE_MASK_ARB                (0x9126)
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB            (0x00000001)
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB   (0x00000002)

#define GL_COMPILE_STATUS                           (0x8B81)
#define GL_ARRAY_BUFFER                             (0x8892)
#define GL_STATIC_DRAW                              (0x88E4)
#define GL_ELEMENT_ARRAY_BUFFER                     (0x8893)

enum OpenGLFunctionLoadStatus {
    Success =  0,
    Failed  = -1,
};

typedef signed long long      GLsizeiptr;
typedef signed long long      GLintptr;
typedef char                  GLchar;

typedef HGLRC        (GLAPIENTRY TEARA_glCreateContextAttribsARB)(HDC, HGLRC, const int*);
typedef BOOL         (GLAPIENTRY TEARA_glChoosePixelFormatARB)(HDC, const int*, const FLOAT*, UINT, int*, UINT*);
typedef void         (GLAPIENTRY TEARA_glGenBuffers)(GLsizei, GLuint*);
typedef void         (GLAPIENTRY TEARA_glBindBuffer)(GLenum, GLuint);
typedef void         (GLAPIENTRY TEARA_glBufferData)(GLenum, GLsizeiptr, const void*, GLenum);
typedef void         (GLAPIENTRY TEARA_glEnableVertexAttribArray)(GLuint);
typedef void         (GLAPIENTRY TEARA_glVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
typedef void         (GLAPIENTRY TEARA_glDisableVertexAttribArray)(GLuint);
typedef GLuint       (GLAPIENTRY TEARA_glCreateProgram)(void);
typedef GLuint       (GLAPIENTRY TEARA_glCreateShader)(GLenum);
typedef void         (GLAPIENTRY TEARA_glShaderSource)(GLuint, GLsizei, const GLchar *const*, const GLint*);
typedef void         (GLAPIENTRY TEARA_glCompileShader)(GLuint);
typedef void         (GLAPIENTRY TEARA_glGetShaderiv)(GLuint, GLenum, GLint*);
typedef void         (GLAPIENTRY TEARA_glAttachShader)(GLuint, GLuint);
typedef void         (GLAPIENTRY TEARA_glLinkProgram)(GLuint);
typedef void         (GLAPIENTRY TEARA_glGetProgramiv)(GLuint, GLenum, GLint*);
typedef void         (GLAPIENTRY TEARA_glValidateProgram)(GLuint);
typedef void         (GLAPIENTRY TEARA_glUseProgram)(GLuint);
typedef void         (GLAPIENTRY TEARA_glGetProgramInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef GLint        (GLAPIENTRY TEARA_glGetUniformLocation)(GLuint, const GLchar*);
typedef void         (GLAPIENTRY TEARA_glUniform1f)(GLint, GLfloat);
typedef void         (GLAPIENTRY TEARA_glUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat*);
typedef void         (GLAPIENTRY TEARA_glUniform1i)(GLint, GLint);
typedef void         (GLAPIENTRY TEARA_glActiveTexture)(GLenum);
typedef void         (GLAPIENTRY TEARA_glGenVertexArrays)(GLsizei, GLuint*);
typedef void         (GLAPIENTRY TEARA_glBindVertexArray)(GLuint);
typedef void         (GLAPIENTRY TEARA_glUniform3fv)(GLint, GLsizei, const GLfloat*);

extern TEARA_glCreateContextAttribsARB      tglCreateContextAttribsARB;
extern TEARA_glChoosePixelFormatARB         tglChoosePixelFormatARB;
extern TEARA_glGenBuffers                   tglGenBuffers;
extern TEARA_glBindBuffer                   tglBindBuffer;
extern TEARA_glBufferData                   tglBufferData;
extern TEARA_glEnableVertexAttribArray      tglEnableVertexAttribArray;
extern TEARA_glVertexAttribPointer          tglVertexAttribPointer;
extern TEARA_glDisableVertexAttribArray     tglDisableVertexAttribArray;
extern TEARA_glCreateProgram                tglCreateProgram;
extern TEARA_glCreateShader                 tglCreateShader;
extern TEARA_glShaderSource                 tglShaderSource;
extern TEARA_glCompileShader                tglCompileShader;
extern TEARA_glGetShaderiv                  tglGetShaderiv;
extern TEARA_glAttachShader                 tglAttachShader;
extern TEARA_glLinkProgram                  tglLinkProgram;
extern TEARA_glGetProgramiv                 tglGetProgramiv;
extern TEARA_glValidateProgram              tglValidateProgram;
extern TEARA_glUseProgram                   tglUseProgram;
extern TEARA_glGetProgramInfoLog            tglGetProgramInfoLog;
extern TEARA_glGetUniformLocation           tglGetUniformLocation;
extern TEARA_glUniform1f                    tglUniform1f;
extern TEARA_glUniformMatrix4fv             tglUniformMatrix4fv;
extern TEARA_glUniform1i                    tglUniform1i;
extern TEARA_glActiveTexture                tglActiveTexture;
extern TEARA_glGenVertexArrays              tglGenVertexArrays;
extern TEARA_glBindVertexArray              tglBindVertexArray;
extern TEARA_glUniform3fv                   tglUniform3fv;

OpenGLFunctionLoadStatus LoadGLFunctions();

#endif