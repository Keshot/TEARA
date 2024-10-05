#include "WinOpenGL.h"

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

    return Statuses::Success;
}