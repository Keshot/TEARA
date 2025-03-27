#include <windows.h>
#include <gl/GL.h>

#include "TLib/Utils/Types.h"

#define GLAPIENTRY __stdcall*

#define WGL_CONTEXT_MAJOR_VERSION_ARB               (0x2091)
#define WGL_CONTEXT_MINOR_VERSION_ARB               (0x2092)
#define WGL_CONTEXT_PROFILE_MASK_ARB                (0x9126)
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB            (0x00000001)
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB   (0x00000002)

#define GL_INVALID_UNIFORM_NAME                     ((i8)0xFF)
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
typedef void         (GLAPIENTRY TEARA_glShaderSource)(GLuint, GLsizei, const GLchar **string, const GLint*);
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
typedef void         (GLAPIENTRY TEARA_glDrawElementsBaseVertex)(GLenum, GLsizei, GLenum, void*, GLint);
typedef void         (GLAPIENTRY TEARA_glGetShaderInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef void         (GLAPIENTRY TEARA_glDeleteShader)(GLuint);
typedef void         (GLAPIENTRY TEARA_glDrawElements)(GLenum mode, GLsizei count, GLenum type, const void* indices);

TEARA_glCreateContextAttribsARB      tglCreateContextAttribsARB;
TEARA_glChoosePixelFormatARB         tglChoosePixelFormatARB;
TEARA_glGenBuffers                   tglGenBuffers;
TEARA_glBindBuffer                   tglBindBuffer;
TEARA_glBufferData                   tglBufferData;
TEARA_glEnableVertexAttribArray      tglEnableVertexAttribArray;
TEARA_glVertexAttribPointer          tglVertexAttribPointer;
TEARA_glDisableVertexAttribArray     tglDisableVertexAttribArray;
TEARA_glCreateProgram                tglCreateProgram;
TEARA_glCreateShader                 tglCreateShader;
TEARA_glShaderSource                 tglShaderSource;
TEARA_glCompileShader                tglCompileShader;
TEARA_glGetShaderiv                  tglGetShaderiv;
TEARA_glAttachShader                 tglAttachShader;
TEARA_glLinkProgram                  tglLinkProgram;
TEARA_glGetProgramiv                 tglGetProgramiv;
TEARA_glValidateProgram              tglValidateProgram;
TEARA_glUseProgram                   tglUseProgram;
TEARA_glGetProgramInfoLog            tglGetProgramInfoLog;
TEARA_glGetUniformLocation           tglGetUniformLocation;
TEARA_glUniform1f                    tglUniform1f;
TEARA_glUniformMatrix4fv             tglUniformMatrix4fv;
TEARA_glUniform1i                    tglUniform1i;
TEARA_glActiveTexture                tglActiveTexture;
TEARA_glGenVertexArrays              tglGenVertexArrays;
TEARA_glBindVertexArray              tglBindVertexArray;
TEARA_glUniform3fv                   tglUniform3fv;
TEARA_glDrawElementsBaseVertex       tglDrawElementsBaseVertex;
TEARA_glGetShaderInfoLog             tglGetShaderInfoLog;
TEARA_glDeleteShader                 tglDeleteShader;
TEARA_glDrawElements                 tglDrawElements;

Statuses LoadGLFunctions()
{
    tglGenBuffers = (TEARA_glGenBuffers) wglGetProcAddress("glGenBuffers");
    if (!tglGenBuffers) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglBindBuffer = (TEARA_glBindBuffer) wglGetProcAddress("glBindBuffer");
    if (!tglBindBuffer) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglBufferData = (TEARA_glBufferData) wglGetProcAddress("glBufferData");
    if (!tglBufferData) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglEnableVertexAttribArray = (TEARA_glEnableVertexAttribArray) wglGetProcAddress("glEnableVertexAttribArray");
    if (!tglEnableVertexAttribArray) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglVertexAttribPointer = (TEARA_glVertexAttribPointer) wglGetProcAddress("glVertexAttribPointer");
    if (!tglVertexAttribPointer) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglDisableVertexAttribArray = (TEARA_glDisableVertexAttribArray) wglGetProcAddress("glDisableVertexAttribArray");
    if (!tglDisableVertexAttribArray) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglCreateProgram = (TEARA_glCreateProgram) wglGetProcAddress("glCreateProgram");
    if (!tglCreateProgram) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglCreateShader = (TEARA_glCreateShader) wglGetProcAddress("glCreateShader");
    if (!tglCreateShader) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglShaderSource = (TEARA_glShaderSource) wglGetProcAddress("glShaderSource");
    if (!tglShaderSource) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglCompileShader = (TEARA_glCompileShader) wglGetProcAddress("glCompileShader");
    if (!tglCompileShader) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglGetShaderiv = (TEARA_glGetShaderiv) wglGetProcAddress("glGetShaderiv");
    if (!tglGetShaderiv) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglAttachShader = (TEARA_glAttachShader) wglGetProcAddress("glAttachShader");
    if (!tglAttachShader) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglLinkProgram = (TEARA_glLinkProgram) wglGetProcAddress("glLinkProgram");
    if (!tglLinkProgram) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglGetProgramiv = (TEARA_glGetProgramiv) wglGetProcAddress("glGetProgramiv");
    if (!tglGetProgramiv) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglValidateProgram = (TEARA_glValidateProgram) wglGetProcAddress("glValidateProgram");
    if (!tglValidateProgram) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglUseProgram = (TEARA_glUseProgram) wglGetProcAddress("glUseProgram");
    if (!tglUseProgram) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglGetProgramInfoLog = (TEARA_glGetProgramInfoLog) wglGetProcAddress("glGetProgramInfoLog");
    if (!tglGetProgramInfoLog) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglGetUniformLocation = (TEARA_glGetUniformLocation) wglGetProcAddress("glGetUniformLocation");
    if (!tglGetUniformLocation) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglUniform1f = (TEARA_glUniform1f) wglGetProcAddress("glUniform1f");
    if (!tglUniform1f) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglUniformMatrix4fv = (TEARA_glUniformMatrix4fv) wglGetProcAddress("glUniformMatrix4fv");
    if (!tglUniformMatrix4fv) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglUniform1i = (TEARA_glUniform1i) wglGetProcAddress("glUniform1i");
    if (!tglUniform1i) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglActiveTexture = (TEARA_glActiveTexture) wglGetProcAddress("glActiveTexture");
    if (!tglActiveTexture) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglGenVertexArrays = (TEARA_glGenVertexArrays) wglGetProcAddress("glGenVertexArrays");
    if (!tglGenVertexArrays) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglBindVertexArray = (TEARA_glBindVertexArray) wglGetProcAddress("glBindVertexArray");
    if (!tglBindVertexArray) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglUniform3fv = (TEARA_glUniform3fv) wglGetProcAddress("glUniform3fv");
    if (!tglUniform3fv) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglDrawElementsBaseVertex = (TEARA_glDrawElementsBaseVertex) wglGetProcAddress("glDrawElementsBaseVertex");
    if (!tglDrawElementsBaseVertex) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglGetShaderInfoLog = (TEARA_glGetShaderInfoLog) wglGetProcAddress ("glGetShaderInfoLog");
    if (!tglGetShaderInfoLog) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglDeleteShader = (TEARA_glDeleteShader) wglGetProcAddress ("glDeleteShader");
    if (!tglDeleteShader) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglDrawElements = (TEARA_glDrawElements) wglGetProcAddress ("glDrawElements");
    if (!tglDrawElements) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    return Statuses::Success;
}