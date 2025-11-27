#ifndef _TEARA_RENERING_OPENGL_H_
#define _TEARA_RENERING_OPENGL_H_

#if (defined(_WIN32) || defined(WIN32))
#include <windows.h>
#include <gl/GL.h>

#define GLAPIENTRY __stdcall*

#define tglGetProcAddress(funcname) wglGetProcAddress(funcname)
#endif

// if system doesn't have glext.h we include local one
#ifndef GL_GLEXT_VERSION
#include "glext.h"
#endif

#include "Core/Types.h"

#define GL_TEXTURE_UNIT0 (0x00)
#define GL_TEXTURE_UNIT1 (GL_TEXTURE_UNIT0 + 1)
#define GL_TEXTURE_UNIT2 (GL_TEXTURE_UNIT1 + 1)
#define GL_TEXTURE_UNIT3 (GL_TEXTURE_UNIT2 + 1)

#define EXTERN_FUNCTION(name) extern TEARA_##name t##name
#define DEF_GL_FUNCTION(return_type, conv, name,...) return_type (conv TEARA_##name)(__VA_ARGS__)

typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glGenBuffers, GLsizei n, GLuint *buffers);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glBindBuffer, GLenum target, GLuint buffer);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glBufferData, GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glEnableVertexAttribArray, GLuint index);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glVertexAttribPointer, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glDisableVertexAttribArray, GLuint index);
typedef DEF_GL_FUNCTION(GLuint, GLAPIENTRY, glCreateProgram, void);
typedef DEF_GL_FUNCTION(GLuint, GLAPIENTRY, glCreateShader, GLenum shaderType);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glShaderSource, GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glCompileShader, GLuint shader);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glGetShaderiv, GLuint shader, GLenum pname, GLint *params);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glAttachShader, GLuint program, GLuint shader);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glLinkProgram, GLuint program);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glGetProgramiv, GLuint program, GLenum pname, GLint *params);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glValidateProgram, GLuint program);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glUseProgram, GLuint program);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glGetProgramInfoLog, GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
typedef DEF_GL_FUNCTION(GLint, GLAPIENTRY, glGetUniformLocation, GLuint program, const GLchar *name);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glUniform1f, GLint location, GLfloat v0);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glUniformMatrix4fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glUniform1i, GLint location, GLint v0);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glActiveTexture, GLenum texture);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glGenVertexArrays, GLsizei n, GLuint *arrays);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glBindVertexArray, GLuint array);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glUniform3fv, GLint location, GLsizei count, const GLfloat *value);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glDrawElementsBaseVertex, GLenum mode, GLsizei count, GLenum type, void *indices, GLint basevertex);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glGetShaderInfoLog, GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glDeleteShader, GLuint shader);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glDrawElements, GLenum mode, GLsizei count, GLenum type, const void *indices);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glGenerateMipmap,GLenum target);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glVertexAttribIPointer, GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glGenFramebuffers, GLsizei n, GLuint *ids);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glBindFramebuffer, GLenum target, GLuint framebuffer);
typedef DEF_GL_FUNCTION(void, GLAPIENTRY, glFramebufferTexture2D, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef DEF_GL_FUNCTION(GLenum, GLAPIENTRY, glCheckFramebufferStatus, GLenum target);

EXTERN_FUNCTION(glGenBuffers);
EXTERN_FUNCTION(glBindBuffer);
EXTERN_FUNCTION(glBufferData);
EXTERN_FUNCTION(glEnableVertexAttribArray);
EXTERN_FUNCTION(glVertexAttribPointer);
EXTERN_FUNCTION(glDisableVertexAttribArray);
EXTERN_FUNCTION(glCreateProgram);
EXTERN_FUNCTION(glCreateShader);
EXTERN_FUNCTION(glShaderSource);
EXTERN_FUNCTION(glCompileShader);
EXTERN_FUNCTION(glGetShaderiv);
EXTERN_FUNCTION(glAttachShader);
EXTERN_FUNCTION(glLinkProgram);
EXTERN_FUNCTION(glGetProgramiv);
EXTERN_FUNCTION(glValidateProgram);
EXTERN_FUNCTION(glUseProgram);
EXTERN_FUNCTION(glGetProgramInfoLog);
EXTERN_FUNCTION(glGetUniformLocation);
EXTERN_FUNCTION(glUniform1f);
EXTERN_FUNCTION(glUniformMatrix4fv);
EXTERN_FUNCTION(glUniform1i);
EXTERN_FUNCTION(glActiveTexture);
EXTERN_FUNCTION(glGenVertexArrays);
EXTERN_FUNCTION(glBindVertexArray);
EXTERN_FUNCTION(glUniform3fv);
EXTERN_FUNCTION(glDrawElementsBaseVertex);
EXTERN_FUNCTION(glGetShaderInfoLog);
EXTERN_FUNCTION(glDeleteShader);
EXTERN_FUNCTION(glDrawElements);
EXTERN_FUNCTION(glGenerateMipmap);
EXTERN_FUNCTION(glVertexAttribIPointer);
EXTERN_FUNCTION(glGenFramebuffers);
EXTERN_FUNCTION(glBindFramebuffer);
EXTERN_FUNCTION(glFramebufferTexture2D);
EXTERN_FUNCTION(glCheckFramebufferStatus);

Statuses LoadGLFunctions();

#endif