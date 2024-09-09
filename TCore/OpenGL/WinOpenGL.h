#ifndef _TEARA_OPENGL_WIN_H_
#define _TEARA_OPENGL_WIN_H_

#include <windows.h>
#include <gl/GL.h>

#include "TLib/Utils/Types.h"

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
#define GL_LINK_STATUS                              (0x8B82)
#define GL_VERTEX_SHADER                            (0x8B31)
#define GL_FRAGMENT_SHADER                          (0x8B30)
#define GL_VALIDATE_STATUS                          (0x8B83)
#define GL_CLAMP_TO_EDGE                            (0x812F)
#define GL_TEXTURE0                                 (0x84C0)
#define GL_TEXTURE1                                 (0x84C1)
#define GL_TEXTURE2                                 (0x84C2)
#define GL_TEXTURE3                                 (0x84C3)
#define GL_TEXTURE4                                 (0x84C4)
#define GL_TEXTURE5                                 (0x84C5)
#define GL_TEXTURE6                                 (0x84C6)
#define GL_TEXTURE7                                 (0x84C7)
#define GL_TEXTURE8                                 (0x84C8)
#define GL_TEXTURE9                                 (0x84C9)
#define GL_TEXTURE10                                (0x84CA)
#define GL_TEXTURE11                                (0x84CB)
#define GL_TEXTURE12                                (0x84CC)
#define GL_TEXTURE13                                (0x84CD)
#define GL_TEXTURE14                                (0x84CE)
#define GL_TEXTURE15                                (0x84CF)
#define GL_TEXTURE16                                (0x84D0)
#define GL_TEXTURE17                                (0x84D1)
#define GL_TEXTURE18                                (0x84D2)
#define GL_TEXTURE19                                (0x84D3)
#define GL_TEXTURE20                                (0x84D4)
#define GL_TEXTURE21                                (0x84D5)
#define GL_TEXTURE22                                (0x84D6)
#define GL_TEXTURE23                                (0x84D7)
#define GL_TEXTURE24                                (0x84D8)
#define GL_TEXTURE25                                (0x84D9)
#define GL_TEXTURE26                                (0x84DA)
#define GL_TEXTURE27                                (0x84DB)
#define GL_TEXTURE28                                (0x84DC)
#define GL_TEXTURE29                                (0x84DD)
#define GL_TEXTURE30                                (0x84DE)
#define GL_TEXTURE31                                (0x84DF)
#define GL_TEXTURE_UNIT0                            (0x00)

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

Statuses LoadGLFunctions();

#endif