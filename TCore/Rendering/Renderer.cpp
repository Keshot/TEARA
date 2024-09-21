#include "TCore/Engine.h"
#include "TCore/OpenGL/WinOpenGL.h"
#include "TLib/Math/Vector.h"
#include "TLib/Utils/AssetsLoader.h"

#define MAX_SHADERS                 (0x0A)

enum OpenGLBuffersLocation {
    PositionLocation        = 0,
    TexturesCoordLocation   = 1,
    IndexArrayLocation      = 2,
    LocationMax
};

struct Vertex {
    Vec3 Position;
};

struct OpenGLBuffers {
    u32     VAO;
    u32     Buffers[OpenGLBuffersLocation::LocationMax];
};

struct ObjectRenderingContext {
    OpenGLBuffers   Buffers;
    ObjFile         ObjectFile;
    i32             ShaderProgramIndex; 
};

enum ShaderUniformType {
    Matrix4f,
    Vector3f,
    Value1f,
    Value1i,
    Max // NOTE (ismail): last element!
};

struct Shader {
    ShaderUniformType   UniformType;
    i32                 Location;
    bool32              Light;
};

// TODO (ismail): rework shaders in order to we can use multiple same uniform types in one shader program
struct ShaderProgram {
    u32     ProgramHandle;
    i32     ShadersAmount;
    Shader  Shaders[MAX_SHADERS];
};

struct TextureObject {
    u32 TextureHandle;
};

struct AmbientLight {
    Vec3    LightColor;
    real32  AmbientIntensity;
};

struct DirectionLight {
    Vec3    LightDirection;
    Vec3    LightColor;
    real32  LightItensity;
};

void LoadTexture2D(const char *FileName, TextureObject *Texture)
{
    TextureFile TextureFl;
    if ( LoadTextureFile(FileName, &TextureFl) != Statuses::Success ) {
        // TODO (ismail): diagnostic?
        return;
    }

    // TODO (ismail): make for these function t prefix tglGetTextures, tglBindTextures, tglTexImage2D etc.
    glGenTextures(1, &Texture->TextureHandle);
    glBindTexture(GL_TEXTURE_2D, Texture->TextureHandle);

    // TODO (ismail): read about mip mapping
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureFl.Width, TextureFl.Height, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureFl.Internal);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    FreeTextureFile(&TextureFl);
}

void ActivateTexture2D(GLenum TextureUnit, TextureObject *Texture)
{
    tglActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_2D, Texture->TextureHandle);
}

static void RendererInit()
{
    GLclampf Red = 0.f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f;
    GLclampf ColorValueMask = 1.0f / 256.0f;

    // NOTE (ismail): set color values for future call of glClear
    glClearColor(Red * ColorValueMask, Green * ColorValueMask, Blue * ColorValueMask, Alpha);

    // TODO (ismail): learn this staff glEnable(GL_CULL_FACE)
    // TODO (ismail): learn this staff glFrontFace(GL_CW)
    // TODO (ismail): learn this staff glCullFace
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);

    // NOTE (ismail): wireframe mode = GL_LINE, polygon mode = GL_FILL
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_DEPTH_TEST);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static bool32 AttachShader(GLuint ShaderHandle, const char *ShaderCode, GLint Length, GLenum ShaderType)
{
    GLuint Shader = tglCreateShader(ShaderType);
    if (!Shader) {
        // TODO (ismail): diagnostic
        return 0; // TODO (ismail): put here actual value
    }

    tglShaderSource(Shader, 1, &ShaderCode, &Length);
    tglCompileShader(Shader);

    GLint Success;
    tglGetShaderiv(Shader, GL_COMPILE_STATUS, &Success);

    if (!Success) {
        // TODO (ismail): handle this case with glGetShaderInfoLog and print it in log
        return 0;
    }

    tglAttachShader(ShaderHandle, Shader);

    return 1;
}

static void LoadObjectToHardware(OpenGLBuffers *RenderContext, ObjFile *ObjectData)
{
    tglGenVertexArrays(1, &(RenderContext->VAO));
    tglBindVertexArray(RenderContext->VAO);

    tglGenBuffers(OpenGLBuffersLocation::LocationMax, RenderContext->Buffers);

    // position
    tglBindBuffer(GL_ARRAY_BUFFER, RenderContext->Buffers[OpenGLBuffersLocation::PositionLocation]);
    tglBufferData(GL_ARRAY_BUFFER, sizeof(*ObjectData->Positions) * ObjectData->PositionsCount, ObjectData->Positions, GL_STATIC_DRAW);

    tglEnableVertexAttribArray(OpenGLBuffersLocation::PositionLocation);        
    tglVertexAttribPointer(OpenGLBuffersLocation::PositionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // textures coordinate
    tglBindBuffer(GL_ARRAY_BUFFER, RenderContext->Buffers[OpenGLBuffersLocation::TexturesCoordLocation]);
    tglBufferData(GL_ARRAY_BUFFER, sizeof(*ObjectData->TextureCoord) * ObjectData->TexturesCount, ObjectData->TextureCoord, GL_STATIC_DRAW);

    tglEnableVertexAttribArray(OpenGLBuffersLocation::TexturesCoordLocation);
    tglVertexAttribPointer(OpenGLBuffersLocation::TexturesCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    
    // index buffer
    tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RenderContext->Buffers[OpenGLBuffersLocation::IndexArrayLocation]);
    tglBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*ObjectData->Indices) * ObjectData->IndicesCount, ObjectData->Indices, GL_STATIC_DRAW);
    
    tglBindVertexArray(0);
    tglDisableVertexAttribArray(OpenGLBuffersLocation::PositionLocation);
    tglDisableVertexAttribArray(OpenGLBuffersLocation::TexturesCoordLocation);

    tglBindBuffer(GL_ARRAY_BUFFER, 0);
    tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}