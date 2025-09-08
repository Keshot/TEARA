#include "TGL.h"
#include "TLib/Utils/Debug.h"

TEARA_glGenBuffers                  tglGenBuffers;
TEARA_glBindBuffer                  tglBindBuffer;
TEARA_glBufferData                  tglBufferData;
TEARA_glEnableVertexAttribArray     tglEnableVertexAttribArray;
TEARA_glVertexAttribPointer         tglVertexAttribPointer;
TEARA_glDisableVertexAttribArray    tglDisableVertexAttribArray;
TEARA_glCreateProgram               tglCreateProgram;
TEARA_glCreateShader                tglCreateShader;
TEARA_glShaderSource                tglShaderSource;
TEARA_glCompileShader               tglCompileShader;
TEARA_glGetShaderiv                 tglGetShaderiv;
TEARA_glAttachShader                tglAttachShader;
TEARA_glLinkProgram                 tglLinkProgram;
TEARA_glGetProgramiv                tglGetProgramiv;
TEARA_glValidateProgram             tglValidateProgram;
TEARA_glUseProgram                  tglUseProgram;
TEARA_glGetProgramInfoLog           tglGetProgramInfoLog;
TEARA_glGetUniformLocation          tglGetUniformLocation;
TEARA_glUniform1f                   tglUniform1f;
TEARA_glUniformMatrix4fv            tglUniformMatrix4fv;
TEARA_glUniform1i                   tglUniform1i;
TEARA_glActiveTexture               tglActiveTexture;
TEARA_glGenVertexArrays             tglGenVertexArrays;
TEARA_glBindVertexArray             tglBindVertexArray;
TEARA_glUniform3fv                  tglUniform3fv;
TEARA_glDrawElementsBaseVertex      tglDrawElementsBaseVertex;
TEARA_glGetShaderInfoLog            tglGetShaderInfoLog;
TEARA_glDeleteShader                tglDeleteShader;
TEARA_glDrawElements                tglDrawElements;
TEARA_glGenerateMipmap              tglGenerateMipmap;
TEARA_glVertexAttribIPointer        tglVertexAttribIPointer;

#ifndef TEARA_DEBUG

#define LinkDebugFunction()

#else

TEARA_glGenBuffers                  tglGenBuffersOrigin;
TEARA_glBindBuffer                  tglBindBufferOrigin;
TEARA_glBufferData                  tglBufferDataOrigin;
TEARA_glEnableVertexAttribArray     tglEnableVertexAttribArrayOrigin;
TEARA_glVertexAttribPointer         tglVertexAttribPointerOrigin;
TEARA_glDisableVertexAttribArray    tglDisableVertexAttribArrayOrigin;
TEARA_glCreateProgram               tglCreateProgramOrigin;
TEARA_glCreateShader                tglCreateShaderOrigin;
TEARA_glShaderSource                tglShaderSourceOrigin;
TEARA_glCompileShader               tglCompileShaderOrigin;
TEARA_glGetShaderiv                 tglGetShaderivOrigin;
TEARA_glAttachShader                tglAttachShaderOrigin;
TEARA_glLinkProgram                 tglLinkProgramOrigin;
TEARA_glGetProgramiv                tglGetProgramivOrigin;
TEARA_glValidateProgram             tglValidateProgramOrigin;
TEARA_glUseProgram                  tglUseProgramOrigin;
TEARA_glGetProgramInfoLog           tglGetProgramInfoLogOrigin;
TEARA_glGetUniformLocation          tglGetUniformLocationOrigin;
TEARA_glUniform1f                   tglUniform1fOrigin;
TEARA_glUniformMatrix4fv            tglUniformMatrix4fvOrigin;
TEARA_glUniform1i                   tglUniform1iOrigin;
TEARA_glActiveTexture               tglActiveTextureOrigin;
TEARA_glGenVertexArrays             tglGenVertexArraysOrigin;
TEARA_glBindVertexArray             tglBindVertexArrayOrigin;
TEARA_glUniform3fv                  tglUniform3fvOrigin;
TEARA_glDrawElementsBaseVertex      tglDrawElementsBaseVertexOrigin;
TEARA_glGetShaderInfoLog            tglGetShaderInfoLogOrigin;
TEARA_glDeleteShader                tglDeleteShaderOrigin;
TEARA_glDrawElements                tglDrawElementsOrigin;
TEARA_glGenerateMipmap              tglGenerateMipmapOrigin;
TEARA_glVertexAttribIPointer        tglVertexAttribIPointerOrigin;

#define DECLARE_DEBUG_GL_FUNCTION(return_type, name, args_func, args_to_call) \
    return_type t##name##DEBUG args_func                                      \
    {                                                                         \
        return_type Ret = t##name##Origin args_to_call ;                      \
        i32 ErrorCode = glGetError();                                         \
        if (ErrorCode != GL_NO_ERROR) {                                       \
            Assert(false);                                                    \
        }                                                                     \
        return Ret;                                                           \
    }

#define DECLARE_DEBUG_GL_FUNCTION_NO_RET(name, args_func, args_to_call) \
    void t##name##DEBUG args_func                                       \
    {                                                                   \
        t##name##Origin args_to_call ;                                  \
        i32 ErrorCode = glGetError();                                   \
        if (ErrorCode != GL_NO_ERROR) {                                 \
            Assert(false);                                              \
        }                                                               \
    }

DECLARE_DEBUG_GL_FUNCTION_NO_RET(glGenBuffers, (GLsizei n, GLuint* buffers), (n, buffers))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glBindBuffer, (GLenum target, GLuint buffer), (target, buffer))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glBufferData, (GLenum target, GLsizeiptr size, const void * data, GLenum usage), (target, size, data, usage))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glEnableVertexAttribArray, (GLuint index), (index))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glVertexAttribPointer, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer), (index, size, type, normalized, stride, pointer))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glDisableVertexAttribArray, (GLuint index), (index))
DECLARE_DEBUG_GL_FUNCTION(GLuint, glCreateProgram, (void), ())
DECLARE_DEBUG_GL_FUNCTION(GLuint, glCreateShader, (GLenum shaderType), (shaderType))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glShaderSource, (GLuint shader, GLsizei count, const GLchar **string, const GLint *length), (shader, count, string, length))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glCompileShader, (GLuint shader), (shader))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glGetShaderiv, (GLuint shader, GLenum pname, GLint *params), (shader, pname, params))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glAttachShader, (GLuint program, GLuint shader), (program, shader))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glLinkProgram, (GLuint program), (program))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glGetProgramiv, (GLuint program, GLenum pname, GLint *params), (program, pname, params))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glValidateProgram, (GLuint program), (program))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glUseProgram, (GLuint program), (program))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glGetProgramInfoLog, (GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog), (program, maxLength, length, infoLog))
DECLARE_DEBUG_GL_FUNCTION(GLint, glGetUniformLocation, (GLuint program, const GLchar *name), (program, name))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glUniform1f, (GLint location, GLfloat v0), (location, v0))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glUniformMatrix4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value), (location, count, transpose, value))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glUniform1i, (GLint location, GLint v0), (location, v0))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glActiveTexture, (GLenum texture), (texture))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glGenVertexArrays, (GLsizei n, GLuint *arrays), (n, arrays))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glBindVertexArray, (GLuint array), (array))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glUniform3fv, (GLint location, GLsizei count, const GLfloat *value), (location, count, value))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glDrawElementsBaseVertex, (GLenum mode, GLsizei count, GLenum type, void *indices, GLint basevertex), (mode, count, type, indices, basevertex))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glGetShaderInfoLog, (GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog), (shader, maxLength, length, infoLog))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glDeleteShader, (GLuint shader), (shader))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glDrawElements, (GLenum mode, GLsizei count, GLenum type, const void* indices), (mode, count, type, indices))
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glGenerateMipmap, (GLenum target), (target));
DECLARE_DEBUG_GL_FUNCTION_NO_RET(glVertexAttribIPointer, (GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer), (index, size, type, stride, pointer));

void LinkDebugFunction()
{
    tglGenBuffers               = tglGenBuffersDEBUG;
    tglBindBuffer               = tglBindBufferDEBUG;
    tglBufferData               = tglBufferDataDEBUG;
    tglEnableVertexAttribArray  = tglEnableVertexAttribArrayDEBUG;
    tglVertexAttribPointer      = tglVertexAttribPointerDEBUG;
    tglDisableVertexAttribArray = tglDisableVertexAttribArrayDEBUG;
    tglCreateProgram            = tglCreateProgramDEBUG;
    tglCreateShader             = tglCreateShaderDEBUG;
    tglShaderSource             = tglShaderSourceDEBUG;
    tglCompileShader            = tglCompileShaderDEBUG;
    tglGetShaderiv              = tglGetShaderivDEBUG;
    tglAttachShader             = tglAttachShaderDEBUG;
    tglLinkProgram              = tglLinkProgramDEBUG;
    tglGetProgramiv             = tglGetProgramivDEBUG;
    tglValidateProgram          = tglValidateProgramDEBUG;
    tglUseProgram               = tglUseProgramDEBUG;
    tglGetProgramInfoLog        = tglGetProgramInfoLogDEBUG;
    tglGetUniformLocation       = tglGetUniformLocationDEBUG;
    tglUniform1f                = tglUniform1fDEBUG;
    tglUniformMatrix4fv         = tglUniformMatrix4fvDEBUG;
    tglUniform1i                = tglUniform1iDEBUG;
    tglActiveTexture            = tglActiveTextureDEBUG;
    tglGenVertexArrays          = tglGenVertexArraysDEBUG;
    tglBindVertexArray          = tglBindVertexArrayDEBUG;
    tglUniform3fv               = tglUniform3fvDEBUG;
    tglDrawElementsBaseVertex   = tglDrawElementsBaseVertexDEBUG;
    tglGetShaderInfoLog         = tglGetShaderInfoLogDEBUG;
    tglDeleteShader             = tglDeleteShaderDEBUG;
    tglDrawElements             = tglDrawElementsDEBUG;
    tglGenerateMipmap           = tglGenerateMipmapDEBUG;
    tglVertexAttribIPointer     = tglVertexAttribIPointerDEBUG;
}

#define tglGenBuffers               tglGenBuffersOrigin 
#define tglBindBuffer               tglBindBufferOrigin 
#define tglBufferData               tglBufferDataOrigin 
#define tglEnableVertexAttribArray  tglEnableVertexAttribArrayOrigin 
#define tglVertexAttribPointer      tglVertexAttribPointerOrigin 
#define tglDisableVertexAttribArray tglDisableVertexAttribArrayOrigin 
#define tglCreateProgram            tglCreateProgramOrigin 
#define tglCreateShader             tglCreateShaderOrigin 
#define tglShaderSource             tglShaderSourceOrigin 
#define tglCompileShader            tglCompileShaderOrigin 
#define tglGetShaderiv              tglGetShaderivOrigin 
#define tglAttachShader             tglAttachShaderOrigin 
#define tglLinkProgram              tglLinkProgramOrigin 
#define tglGetProgramiv             tglGetProgramivOrigin 
#define tglValidateProgram          tglValidateProgramOrigin 
#define tglUseProgram               tglUseProgramOrigin 
#define tglGetProgramInfoLog        tglGetProgramInfoLogOrigin 
#define tglGetUniformLocation       tglGetUniformLocationOrigin 
#define tglUniform1f                tglUniform1fOrigin 
#define tglUniformMatrix4fv         tglUniformMatrix4fvOrigin 
#define tglUniform1i                tglUniform1iOrigin 
#define tglActiveTexture            tglActiveTextureOrigin 
#define tglGenVertexArrays          tglGenVertexArraysOrigin 
#define tglBindVertexArray          tglBindVertexArrayOrigin 
#define tglUniform3fv               tglUniform3fvOrigin 
#define tglDrawElementsBaseVertex   tglDrawElementsBaseVertexOrigin 
#define tglGetShaderInfoLog         tglGetShaderInfoLogOrigin 
#define tglDeleteShader             tglDeleteShaderOrigin 
#define tglDrawElements             tglDrawElementsOrigin 
#define tglGenerateMipmap           tglGenerateMipmapOrigin 
#define tglVertexAttribIPointer     tglVertexAttribIPointerOrigin

#endif

Statuses LoadGLFunctions()
{
    LinkDebugFunction();

    tglGenBuffers = (TEARA_glGenBuffers) tglGetProcAddress("glGenBuffers");
    if (!tglGenBuffers) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglBindBuffer = (TEARA_glBindBuffer) tglGetProcAddress("glBindBuffer");
    if (!tglBindBuffer) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglBufferData = (TEARA_glBufferData) tglGetProcAddress("glBufferData");
    if (!tglBufferData) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglEnableVertexAttribArray = (TEARA_glEnableVertexAttribArray) tglGetProcAddress("glEnableVertexAttribArray");
    if (!tglEnableVertexAttribArray) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglVertexAttribPointer = (TEARA_glVertexAttribPointer) tglGetProcAddress("glVertexAttribPointer");
    if (!tglVertexAttribPointer) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglDisableVertexAttribArray = (TEARA_glDisableVertexAttribArray) tglGetProcAddress("glDisableVertexAttribArray");
    if (!tglDisableVertexAttribArray) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglCreateProgram = (TEARA_glCreateProgram) tglGetProcAddress("glCreateProgram");
    if (!tglCreateProgram) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglCreateShader = (TEARA_glCreateShader) tglGetProcAddress("glCreateShader");
    if (!tglCreateShader) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglShaderSource = (TEARA_glShaderSource) tglGetProcAddress("glShaderSource");
    if (!tglShaderSource) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglCompileShader = (TEARA_glCompileShader) tglGetProcAddress("glCompileShader");
    if (!tglCompileShader) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglGetShaderiv = (TEARA_glGetShaderiv) tglGetProcAddress("glGetShaderiv");
    if (!tglGetShaderiv) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglAttachShader = (TEARA_glAttachShader) tglGetProcAddress("glAttachShader");
    if (!tglAttachShader) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglLinkProgram = (TEARA_glLinkProgram) tglGetProcAddress("glLinkProgram");
    if (!tglLinkProgram) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglGetProgramiv = (TEARA_glGetProgramiv) tglGetProcAddress("glGetProgramiv");
    if (!tglGetProgramiv) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglValidateProgram = (TEARA_glValidateProgram) tglGetProcAddress("glValidateProgram");
    if (!tglValidateProgram) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglUseProgram = (TEARA_glUseProgram) tglGetProcAddress("glUseProgram");
    if (!tglUseProgram) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglGetProgramInfoLog = (TEARA_glGetProgramInfoLog) tglGetProcAddress("glGetProgramInfoLog");
    if (!tglGetProgramInfoLog) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglGetUniformLocation = (TEARA_glGetUniformLocation) tglGetProcAddress("glGetUniformLocation");
    if (!tglGetUniformLocation) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglUniform1f = (TEARA_glUniform1f) tglGetProcAddress("glUniform1f");
    if (!tglUniform1f) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglUniformMatrix4fv = (TEARA_glUniformMatrix4fv) tglGetProcAddress("glUniformMatrix4fv");
    if (!tglUniformMatrix4fv) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglUniform1i = (TEARA_glUniform1i) tglGetProcAddress("glUniform1i");
    if (!tglUniform1i) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglActiveTexture = (TEARA_glActiveTexture) tglGetProcAddress("glActiveTexture");
    if (!tglActiveTexture) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglGenVertexArrays = (TEARA_glGenVertexArrays) tglGetProcAddress("glGenVertexArrays");
    if (!tglGenVertexArrays) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglBindVertexArray = (TEARA_glBindVertexArray) tglGetProcAddress("glBindVertexArray");
    if (!tglBindVertexArray) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglUniform3fv = (TEARA_glUniform3fv) tglGetProcAddress("glUniform3fv");
    if (!tglUniform3fv) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglDrawElementsBaseVertex = (TEARA_glDrawElementsBaseVertex) tglGetProcAddress("glDrawElementsBaseVertex");
    if (!tglDrawElementsBaseVertex) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglGetShaderInfoLog = (TEARA_glGetShaderInfoLog) tglGetProcAddress ("glGetShaderInfoLog");
    if (!tglGetShaderInfoLog) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglDeleteShader = (TEARA_glDeleteShader) tglGetProcAddress ("glDeleteShader");
    if (!tglDeleteShader) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglDrawElements = (TEARA_glDrawElements) tglGetProcAddress ("glDrawElements");
    if (!tglDrawElements) {
        // TODO (ismail): diagnostic?
        tglDrawElements = glDrawElements;
    }

    tglGenerateMipmap = (TEARA_glGenerateMipmap) tglGetProcAddress ("glGenerateMipmap");
    if (!tglGenerateMipmap) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    tglVertexAttribIPointer = (TEARA_glVertexAttribIPointer) tglGetProcAddress ("glVertexAttribIPointer");
    if (!tglGenerateMipmap) {
        // TODO (ismail): diagnostic?
        return Statuses::Failed;
    }

    return Statuses::Success;
}