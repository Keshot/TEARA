#include "TCore/Engine.h"
#include "TCore/OpenGL/WinOpenGL.h"
#include "TLib/Math/Vector.h"
#include "TLib/Math/Matrix.h"
#include "TLib/Utils/AssetsLoader.h"

enum OpenGLBuffersLocation {
    PositionLocation        = 0,
    TexturesCoordLocation   = 1,
    NormalsLocation         = 2,
    IndexArrayLocation      = 3,
    LocationMax
};

enum ShaderProgramTypes {
    StaticObjectShader = 0,
};

enum InternalConstants {
    ShaderCompileLogLength = 1024,
};

struct Vertex {
    Vec3 Position;
};

struct OpenGLBuffers {
    u32     VAO;
    u32     Buffers[OpenGLBuffersLocation::LocationMax];
};

struct TextureObject {
    u32 TextureHandle;
};

struct ObjectRenderingContext {
    OpenGLBuffers   Buffers;
    ObjFile         ObjectFile;
    i32             ShaderProgramIndex; 
};

struct AmbientLight {
    Vec3    Color;
    real32  Intensity;
};

struct DirectionLight {
    Vec3    Direction;
    Vec3    Color;
    real32  Intensity;
};

struct StaticObjectRenderingShader_Options {
    i32 PerspProjLocation;

    i32 ObjToCameraSpaceTransformLocation;

    i32 ObjectTranslationLocation;
    i32 ObjectScaleLocation;
    i32 ObjectRotationLocation;

    i32 AmbLightColorLocation;
    i32 AmbLightIntensityLocation;

    i32 DirLightDirectionLocation;
    i32 DirLightColorLocation;
    i32 DirLightIntensityLocation;

    i32 ObjectFlatColorLocation;

    i32 TextureSamplerIDLocation;
};

struct BaseRenderingShader_Options {
    union {
        StaticObjectRenderingShader_Options StaticObjectShader;
    } ShaderOptions;
};

struct StaticObjectRenderingShader_Setup {
    Mat4x4          PerspProj;
    Mat4x4          ObjToCameraSpaceTransform;
    Mat4x4          ObjectTranslation;
    Mat4x4          ObjectScale;
    Mat4x4          ObjectRotation;
    AmbientLight    AmbLight;
    DirectionLight  DirLight;
    Vec3            ObjectFlatColor;
    u32             TextureSamplerID;
    u32             TextureHandle;
    u32             VAO;
};

struct BaseRenderingShader_Setup {
    union {
        StaticObjectRenderingShader_Setup StaticObjectShader;
    } ShaderSetup;
};

// TODO (ismail): rework shaders in order to we can use multiple same uniform types in one shader program
struct ShaderProgram {
    u32                         ProgramHandle;
    BaseRenderingShader_Options Options;
    BaseRenderingShader_Setup   Setup;
    ShaderProgramTypes          Type;
};

enum ShaderType {
    VertexShader    = GL_VERTEX_SHADER,
    FragmentShader  = GL_FRAGMENT_SHADER,
};

struct ShaderSetupOption {
    bool32      LoadFromFile;
    u32         DataLength;
    ShaderType  Type;
    const char* FileName;
    const char* Data;
};

static void LoadTexture2D(const char *FileName, TextureObject *Texture)
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

static void ActivateTexture2D(GLenum TextureUnit, u32 TextureHandle)
{
    tglActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_2D, TextureHandle);
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
    i32 LogLength;
    char ShaderCompileLog[InternalConstants::ShaderCompileLogLength] = {};
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
        tglGetShaderInfoLog(Shader, InternalConstants::ShaderCompileLogLength, &LogLength, ShaderCompileLog);
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

    // normals
    tglBindBuffer(GL_ARRAY_BUFFER, RenderContext->Buffers[OpenGLBuffersLocation::NormalsLocation]);
    tglBufferData(GL_ARRAY_BUFFER, sizeof(*ObjectData->Normals) * ObjectData->NormalsCount, ObjectData->Normals, GL_STATIC_DRAW);

    tglEnableVertexAttribArray(OpenGLBuffersLocation::NormalsLocation);
    tglVertexAttribPointer(OpenGLBuffersLocation::NormalsLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    // index buffer
    tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RenderContext->Buffers[OpenGLBuffersLocation::IndexArrayLocation]);
    tglBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*ObjectData->Indices) * ObjectData->IndicesCount, ObjectData->Indices, GL_STATIC_DRAW);
    
    tglBindVertexArray(0);
    tglDisableVertexAttribArray(OpenGLBuffersLocation::PositionLocation);
    tglDisableVertexAttribArray(OpenGLBuffersLocation::TexturesCoordLocation);

    tglBindBuffer(GL_ARRAY_BUFFER, 0);
    tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

static Statuses SetupShader(EnginePlatform *Platform, ShaderProgram *Prog, ShaderSetupOption *Options, u32 OptSize)
{
    u32 ProgramHandle = tglCreateProgram();
    if (!ProgramHandle) {
        // TODO (ismail): diagnostic
        return Statuses::Failed; // TODO (ismail): put here actual value
    }

    switch (Prog->Type) {

        case ShaderProgramTypes::StaticObjectShader: {
            StaticObjectRenderingShader_Options *ShaderOpt = &Prog->Options.ShaderOptions.StaticObjectShader;

            for (i32 OptIndex = 0; OptIndex < OptSize; ++OptIndex) {
                ShaderSetupOption *ShaderOpt = &Options[OptIndex];

                if (ShaderOpt->LoadFromFile) {
                    File ShaderFile = Platform->ReadFile(ShaderOpt->FileName);

                    if (!ShaderFile.Data) {
                        // TODO (ismail): diagnostic
                        return Statuses::Failed;
                    }

                    bool32 AttachRes = AttachShader(ProgramHandle, (const char*)ShaderFile.Data, (GLint)ShaderFile.Size, ShaderOpt->Type);

                    Platform->FreeFileData(&ShaderFile);

                    if (!AttachRes) {
                        // TODO (ismail): diagnostic
                        return Statuses::Failed; // TODO (ismail): more complicated check here
                    }
                }
                else {
                    if (!AttachShader(ProgramHandle, ShaderOpt->Data, ShaderOpt->DataLength, ShaderOpt->Type)) {
                        // TODO (ismail): diagnostic
                        return Statuses::Failed; // TODO (ismail): more complicated check here
                    }
                }
            }

            GLchar ErrorLog[1024] = { 0 };
            tglLinkProgram(ProgramHandle);

            GLint Success;
            tglGetProgramiv(ProgramHandle, GL_LINK_STATUS, &Success);
            if (!Success) {
                // TODO (ismail): handle this case with glGetShaderInfoLog and print it in log
                tglGetProgramInfoLog(ProgramHandle, sizeof(ErrorLog), NULL, ErrorLog);
                return Statuses::Failed;
            }
    
            ShaderOpt->ObjectRotationLocation = tglGetUniformLocation(ProgramHandle, "ObjTransform.ObjectRotation");
            if (ShaderOpt->ObjectRotationLocation == GL_INVALID_UNIFORM_NAME) {
                // TODO (ismail): diagnostic
                return Statuses::Failed;
            }

            ShaderOpt->ObjectScaleLocation = tglGetUniformLocation(ProgramHandle, "ObjTransform.ObjectScale");
            if (ShaderOpt->ObjectScaleLocation == GL_INVALID_UNIFORM_NAME) {
                // TODO (ismail): diagnostic
                return Statuses::Failed;
            }

            ShaderOpt->ObjectTranslationLocation = tglGetUniformLocation(ProgramHandle, "ObjTransform.ObjectTranslation");
            if (ShaderOpt->ObjectTranslationLocation == GL_INVALID_UNIFORM_NAME) {
                // TODO (ismail): diagnostic
                return Statuses::Failed;
            }

            ShaderOpt->PerspProjLocation = tglGetUniformLocation(ProgramHandle, "PerspectiveProjection");
            if (ShaderOpt->PerspProjLocation == GL_INVALID_UNIFORM_NAME) {
                // TODO (ismail): diagnostic
                return Statuses::Failed;
            }

            ShaderOpt->ObjToCameraSpaceTransformLocation = tglGetUniformLocation(ProgramHandle, "ObjToCameraSpaceTransform");
            if (ShaderOpt->ObjToCameraSpaceTransformLocation == GL_INVALID_UNIFORM_NAME) {
                // TODO (ismail): diagnostic
                return Statuses::Failed;
            }

            ShaderOpt->DirLightDirectionLocation = tglGetUniformLocation(ProgramHandle, "DirLight.Direction");
            if (ShaderOpt->DirLightDirectionLocation == GL_INVALID_UNIFORM_NAME) {
                // TODO (ismail): diagnostic
                return Statuses::Failed;
            }

            ShaderOpt->DirLightColorLocation = tglGetUniformLocation(ProgramHandle, "DirLight.Color");
            if (ShaderOpt->DirLightColorLocation == GL_INVALID_UNIFORM_NAME) {
                // TODO (ismail): diagnostic
                return Statuses::Failed;
            }

            ShaderOpt->DirLightIntensityLocation = tglGetUniformLocation(ProgramHandle, "DirLight.Intensity");
            if (ShaderOpt->DirLightIntensityLocation == GL_INVALID_UNIFORM_NAME) {
                // TODO (ismail): diagnostic
                return Statuses::Failed;
            }

            ShaderOpt->AmbLightColorLocation = tglGetUniformLocation(ProgramHandle, "AmbLight.Color");
            if (ShaderOpt->AmbLightColorLocation == GL_INVALID_UNIFORM_NAME) {
                // TODO (ismail): diagnostic
                return Statuses::Failed;
            }

            ShaderOpt->AmbLightIntensityLocation = tglGetUniformLocation(ProgramHandle, "AmbLight.Intensity");
            if (ShaderOpt->AmbLightIntensityLocation == GL_INVALID_UNIFORM_NAME) {
                // TODO (ismail): diagnostic
                return Statuses::Failed;
            }

            ShaderOpt->ObjectFlatColorLocation = tglGetUniformLocation(ProgramHandle, "ColorMultiplyer");
            if (ShaderOpt->ObjectFlatColorLocation == GL_INVALID_UNIFORM_NAME) {
                // TODO (ismail): diagnostic
                return Statuses::Failed;
            }

            ShaderOpt->TextureSamplerIDLocation = tglGetUniformLocation(ProgramHandle, "TextureSampler");
            if (ShaderOpt->TextureSamplerIDLocation == GL_INVALID_UNIFORM_NAME) {
                // TODO (ismail): diagnostic
                return Statuses::Failed;
            }

            tglValidateProgram(ProgramHandle);
            tglGetProgramiv(ProgramHandle, GL_VALIDATE_STATUS, &Success);
            if (!Success) {
                // TODO (ismail): handle this case with glGetShaderInfoLog and print it in log
                return Statuses::Failed;
            }

        } break;

        default: {
            Assert(0);
        }
    }

    Prog->ProgramHandle = ProgramHandle;    
}

static void ProcessShader(ShaderProgram *Prog)
{
    switch (Prog->Type) {

        case ShaderProgramTypes::StaticObjectShader: {
            StaticObjectRenderingShader_Options *Options    = &Prog->Options.ShaderOptions.StaticObjectShader;
            StaticObjectRenderingShader_Setup *Setup        = &Prog->Setup.ShaderSetup.StaticObjectShader;

            tglBindVertexArray(Setup->VAO);
            tglUseProgram(Prog->ProgramHandle);
            ActivateTexture2D(GL_TEXTURE0, Setup->TextureHandle);

            tglUniformMatrix4fv(Options->PerspProjLocation, 1, GL_TRUE, &Setup->PerspProj.Matrix[0][0]);
    
            tglUniformMatrix4fv(Options->ObjectScaleLocation, 1, GL_TRUE, &Setup->ObjectScale.Matrix[0][0]);
            tglUniformMatrix4fv(Options->ObjectRotationLocation, 1, GL_TRUE, &Setup->ObjectRotation.Matrix[0][0]);
            tglUniformMatrix4fv(Options->ObjectTranslationLocation, 1, GL_TRUE, &Setup->ObjectTranslation.Matrix[0][0]);

            tglUniformMatrix4fv(Options->ObjToCameraSpaceTransformLocation, 1, GL_TRUE, &Setup->ObjToCameraSpaceTransform.Matrix[0][0]);

            tglUniform3fv(Options->DirLightColorLocation, 1, Setup->DirLight.Color.ValueHolder);
            tglUniform3fv(Options->DirLightDirectionLocation, 1, Setup->DirLight.Direction.ValueHolder);
            tglUniform1f(Options->DirLightIntensityLocation, Setup->DirLight.Intensity);

            tglUniform3fv(Options->AmbLightColorLocation, 1, Setup->AmbLight.Color.ValueHolder);
            tglUniform1f(Options->AmbLightIntensityLocation, Setup->AmbLight.Intensity);

            tglUniform3fv(Options->ObjectFlatColorLocation, 1, Setup->ObjectFlatColor.ValueHolder);

            tglUniform1i(Options->TextureSamplerIDLocation, Setup->TextureSamplerID);
        } break;

        default: {
            Assert(0);
        }
    }
}