#include "TLib/Utils/Types.h"
#include "TLib/Math/Matrix.h"
#include "TLib/Utils/Debug.h"
#include "TLib/Utils/AssetsLoader.h"
#include "TLib/3rdparty/ufbx/ufbx.h"
#include "TLib/3rdparty/cgltf/cgltf.h"
#include "EnginePlatform.h"
#include "Game.h"
#include "TGL.h"
#include "TLib/3rdparty/stb/stb_image.h"

ShaderProgram ShadersProgramsCache[ShaderProgramsTypeMax];

struct Translation {
    Mat4x4 Trans;
};

struct UniformScale {
    Mat4x4 Scale;
};

struct NonUniformScale {
    Mat4x4 Scale;
};

// TODO(Ismail): correct for Gimbal Lock
void MakeObjectToUprightRotation(Rotation *Rot, Mat4x4 *Mat)
{
    real32 Cosh, Sinh;
    real32 Cosp, Sinp;
    real32 Cosb, Sinb;

    real32 Heading = DEGREE_TO_RAD(Rot->Heading);
    real32 Pitch = DEGREE_TO_RAD(Rot->Pitch);
    real32 Bank = DEGREE_TO_RAD(Rot->Bank);

    Sinh = sinf(Heading);
    Cosh = cosf(Heading);

    Sinp = sinf(Pitch);
    Cosp = cosf(Pitch);

    Sinb = sinf(Bank);
    Cosb = cosf(Bank);

    *Mat = {
        Cosh * Cosb + Sinh * Sinp * Sinb, -Cosh * Sinb + Sinh * Sinp * Cosb, Sinh * Cosp, 0.0f,
                             Cosp * Sinb,                       Cosp * Cosb,       -Sinp, 0.0f,
       -Sinh * Cosb + Cosh * Sinp * Sinb,  Sinh * Sinb + Cosh * Sinp * Cosb, Cosh * Cosp, 0.0f,
                                    0.0f,                              0.0f,        0.0f, 1.0f
    };
}

void MakeUprightToObjectRotation(Mat4x4 *Mat, Rotation *Rot)
{
    real32 Cosh, Sinh;
    real32 Cosp, Sinp;
    real32 Cosb, Sinb;

    real32 Heading = DEGREE_TO_RAD(Rot->Heading);
    real32 Pitch = DEGREE_TO_RAD(Rot->Pitch);
    real32 Bank = DEGREE_TO_RAD(Rot->Bank);

    Sinh = sinf(Heading);
    Cosh = cosf(Heading);

    Sinp = sinf(Pitch);
    Cosp = cosf(Pitch);

    Sinb = sinf(Bank);
    Cosb = cosf(Bank);

    *Mat = {
         Cosh * Cosb + Sinh * Sinp * Sinb,  Cosp * Sinb,  -Sinh * Cosb + Cosh * Sinp * Sinb, 0.0f,
        -Cosh * Sinb + Sinh * Sinp * Cosb,  Cosp * Cosb,   Sinh * Sinb + Cosh * Sinp * Cosb, 0.0f,
                              Sinh * Cosp,        -Sinp,                        Cosh * Cosp, 0.0f,
                                     0.0f,         0.0f,                               0.0f, 1.0f,
    };
}

void MakeUprightToObjectRotationMat3x3(Mat3x3 *Mat, Rotation *Rot)
{
    real32 Cosh, Sinh;
    real32 Cosp, Sinp;
    real32 Cosb, Sinb;

    real32 Heading = DEGREE_TO_RAD(Rot->Heading);
    real32 Pitch = DEGREE_TO_RAD(Rot->Pitch);
    real32 Bank = DEGREE_TO_RAD(Rot->Bank);

    Sinh = sinf(Heading);
    Cosh = cosf(Heading);

    Sinp = sinf(Pitch);
    Cosp = cosf(Pitch);

    Sinb = sinf(Bank);
    Cosb = cosf(Bank);

    *Mat = {
         Cosh * Cosb + Sinh * Sinp * Sinb,  Cosp * Sinb,  -Sinh * Cosb + Cosh * Sinp * Sinb,
        -Cosh * Sinb + Sinh * Sinp * Cosb,  Cosp * Cosb,   Sinh * Sinb + Cosh * Sinp * Cosb,
                              Sinh * Cosp,        -Sinp,                        Cosh * Cosp,
    };
}

void RotationToDirectionVecotrs(Rotation *Rot, Vec3 *Target, Vec3 *Right, Vec3 *Up)
{
    real32 Cosh, Sinh;
    real32 Cosp, Sinp;
    real32 Cosb, Sinb;

    real32 Heading = DEGREE_TO_RAD(Rot->Heading);
    real32 Pitch = DEGREE_TO_RAD(Rot->Pitch);
    real32 Bank = DEGREE_TO_RAD(Rot->Bank);

    Sinh = sinf(Heading);
    Cosh = cosf(Heading);

    Sinp = sinf(Pitch);
    Cosp = cosf(Pitch);

    Sinb = sinf(Bank);
    Cosb = cosf(Bank);

    *Target = {
        Sinh * Cosp,
              -Sinp,
        Cosh * Cosp,
    };

    *Right = {
        Cosh * Cosb + Sinh * Sinp * Sinb,
                             Cosp * Sinb,
       -Sinh * Cosb + Cosh * Sinp * Sinb
    };

    *Up = {
       -Cosh * Sinb + Sinh * Sinp * Cosb,
                             Cosp * Cosb,
        Sinh * Sinb + Cosh * Sinp * Cosb
    };
}

void CreateUniformScale(real32 ScaleFactor, UniformScale *Result)
{
    Result->Scale = {
           ScaleFactor,           0.0f,          0.0f, 0.0f,
                  0.0f,    ScaleFactor,          0.0f, 0.0f,
                  0.0f,          0.0f,    ScaleFactor, 0.0f,
                  0.0f,          0.0f,           0.0f, 1.0f,
    };
}

void CreateNonUniformScale(Vec3 *ScaleFactor, NonUniformScale *Result)
{
    Result->Scale = {
        ScaleFactor->x,           0.0f,          0.0f, 0.0f,
                  0.0f, ScaleFactor->y,          0.0f, 0.0f,
                  0.0f,          0.0f, ScaleFactor->z, 0.0f,
                  0.0f,          0.0f,           0.0f, 1.0f,
    };
}

void MakeScaleFromVector(Vec3* ScaleFactor, Mat4x4* Result)
{
    *Result = {
        ScaleFactor->x,           0.0f,          0.0f, 0.0f,
                  0.0f, ScaleFactor->y,          0.0f, 0.0f,
                  0.0f,          0.0f, ScaleFactor->z, 0.0f,
                  0.0f,          0.0f,           0.0f, 1.0f,
    };
}

void MakeScaleFromVectorRelative(Vec3* ScaleFactor, Vec3* RelativeTo, Mat4x4* Result)
{
    real32 x = 1.0f / RelativeTo->x;
    real32 y = 1.0f / RelativeTo->y;
    real32 z = 1.0f / RelativeTo->z;

    *Result = {
        ScaleFactor->x * x,               0.0f,               0.0f, 0.0f,
                      0.0f, ScaleFactor->y * y,               0.0f, 0.0f,
                      0.0f,               0.0f, ScaleFactor->z * z, 0.0f,
                      0.0f,               0.0f,               0.0f, 1.0f,
    };
}

void MakeTranslationFromVec(Vec3 *TranslationVec, Mat4x4 *TranslationMat)
{
    *TranslationMat = {
        1.0f,   0.0f,   0.0f, TranslationVec->x,
        0.0f,   1.0f,   0.0f, TranslationVec->y,
        0.0f,   0.0f,   1.0f, TranslationVec->z,
        0.0f,   0.0f,   0.0f,              1.0f,
    };
}

inline Mat4x4 MakeInverseTranslation(Vec3 *Trans)
{
    Mat4x4 Result = {
        1.0f,   0.0f,   0.0f, -Trans->x,
        0.0f,   1.0f,   0.0f, -Trans->y,
        0.0f,   0.0f,   1.0f, -Trans->z,
        0.0f,   0.0f,   0.0f,     1.0f,
    };

    return Result;
}

static Mat4x4 MakePerspProjection(real32 FovInDegree, real32 AspectRatio, real32 NearZ, real32 FarZ)
{
    real32 d = 1 / tanf(DEGREE_TO_RAD(FovInDegree / 2));
    
    real32 x = d / AspectRatio;
    real32 y = d;

    real32 ClipDistance = NearZ - FarZ;
    
    real32 a = (-FarZ - NearZ) / ClipDistance;
    real32 b = (2 * NearZ * FarZ) / ClipDistance;

    Mat4x4 Result = {
           x, 0.0f, 0.0f, 0.0f,
        0.0f,    y, 0.0f, 0.0f,
        0.0f, 0.0f,    a,    b,
        0.0f, 0.0f, 1.0f, 0.0f,
    };

    return Result;
}

void GenerateTerrainMesh(TerrainLoadFile *ToLoad, real32 TextureScale, real32 Size, i32 VertexAmount)
{
    Vec3 *Vertices              = ToLoad->Vertices;
    Vec2 *Textures              = ToLoad->Textures;
    Vec3 *Normals               = ToLoad->Normals;
    u32 *Indices                = ToLoad->Indices;
    i32 RowCubeAmount           = VertexAmount - 1;
    i32 TotalCubeAmount         = SQUARE(RowCubeAmount);
    real32 VertexLen            = Size / (real32)VertexAmount;
    real32 OneOverVertexAmount  = TextureScale / ((real32)VertexAmount - 1.0f);

    for (i32 ZIndex = 0; ZIndex < VertexAmount; ++ZIndex) {
        for (i32 XIndex = 0; XIndex < VertexAmount; ++XIndex) {
            i32 ItemIndex = ZIndex * VertexAmount + XIndex;
            Vertices[ItemIndex] = {
                XIndex * VertexLen,
                0.0f,
                ZIndex * VertexLen,
            };

            Textures[ItemIndex] = {
                (real32)XIndex * OneOverVertexAmount,
                (real32)ZIndex * OneOverVertexAmount,
            };

            Normals[ZIndex * VertexAmount + XIndex] = {
                0.0f,
                1.0f,
                0.0f
            };
        }
    }

    for (i32 CubeIndex = 0, It = 0, BottomLineIt = VertexAmount; CubeIndex < TotalCubeAmount; ++CubeIndex, ++BottomLineIt) {
        u32 BottomLeft  = CubeIndex + (CubeIndex / RowCubeAmount);
        u32 BottomRight = BottomLeft + 1;
        u32 TopLeft     = BottomLeft + VertexAmount;
        u32 TopRight    = TopLeft + 1;

        Indices[It++] = BottomLeft;
        Indices[It++] = TopLeft;
        Indices[It++] = TopRight;

        Indices[It++] = TopRight;
        Indices[It++] = BottomRight;
        Indices[It++] = BottomLeft;
    }
}

void LoadTerrainFile(Terrain *Terrain, TerrainLoadFile *TerrainFile)
{
    u32 *BuffersHandler = Terrain->BuffersHandler;

    tglGenVertexArrays(1, &BuffersHandler[OpenGLBuffersLocation::GLVertexArrayLocation]);  
    tglBindVertexArray(BuffersHandler[OpenGLBuffersLocation::GLVertexArrayLocation]);

    tglGenBuffers(OpenGLBuffersLocation::GLLocationMax - 1, BuffersHandler);

    tglBindBuffer(GL_ARRAY_BUFFER, BuffersHandler[OpenGLBuffersLocation::GLPositionLocation]);
    tglBufferData(GL_ARRAY_BUFFER, sizeof(*TerrainFile->Vertices) * TerrainFile->VerticesAmount, TerrainFile->Vertices, GL_STATIC_DRAW);
    tglVertexAttribPointer(OpenGLBuffersLocation::GLPositionLocation, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    tglEnableVertexAttribArray(OpenGLBuffersLocation::GLPositionLocation);

    tglBindBuffer(GL_ARRAY_BUFFER, BuffersHandler[OpenGLBuffersLocation::GLTextureLocation]);
    tglBufferData(GL_ARRAY_BUFFER, sizeof(*TerrainFile->Textures) * TerrainFile->TexturesAmount, TerrainFile->Textures, GL_STATIC_DRAW);
    tglVertexAttribPointer(OpenGLBuffersLocation::GLTextureLocation, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    tglEnableVertexAttribArray(OpenGLBuffersLocation::GLTextureLocation);

    tglBindBuffer(GL_ARRAY_BUFFER, BuffersHandler[OpenGLBuffersLocation::GLNormalsLocation]);
    tglBufferData(GL_ARRAY_BUFFER, sizeof(*TerrainFile->Normals) * TerrainFile->NormalsAmount, TerrainFile->Normals, GL_STATIC_DRAW);
    tglVertexAttribPointer(OpenGLBuffersLocation::GLNormalsLocation, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    tglEnableVertexAttribArray(OpenGLBuffersLocation::GLNormalsLocation);

    tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BuffersHandler[OpenGLBuffersLocation::GLIndexArrayLocation]);
    tglBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*TerrainFile->Indices) * TerrainFile->IndicesAmount, TerrainFile->Indices, GL_STATIC_DRAW);

    tglBindVertexArray(0);
    tglBindBuffer(GL_ARRAY_BUFFER, 0);
    tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

u32 CreateShaderProgram(Platform *Platform, const char *VertexShaderName, const char *FragmentShaderName)
{
    File VertShader;
    File FragShader;
    u32 VertexShaderHandle;
    u32 FragmentShaderHandle;
    u32 FinalShaderProgram;

    i32 Success;
    char InfoLog[512] = {};

    VertShader = Platform->ReadFile(VertexShaderName);

    VertexShaderHandle = tglCreateShader(GL_VERTEX_SHADER);

    tglShaderSource(VertexShaderHandle, 1, (const char**)(&VertShader.Data), NULL);

    Platform->FreeFileData(&VertShader);

    tglCompileShader(VertexShaderHandle);
    tglGetShaderiv(VertexShaderHandle, GL_COMPILE_STATUS, &Success);

    if (!Success) {
        tglGetShaderInfoLog(VertexShaderHandle, 512, NULL, InfoLog);

        Assert(false);
    }

    FragShader = Platform->ReadFile(FragmentShaderName);

    FragmentShaderHandle = tglCreateShader(GL_FRAGMENT_SHADER);

    tglShaderSource(FragmentShaderHandle, 1, (const char**)(&FragShader.Data), NULL);

    Platform->FreeFileData(&FragShader);

    tglCompileShader(FragmentShaderHandle);

    tglGetShaderiv(FragmentShaderHandle, GL_COMPILE_STATUS, &Success);

    if (!Success) {
        tglGetShaderInfoLog(FragmentShaderHandle, 512, NULL, InfoLog);

        Assert(false);
    }

    FinalShaderProgram = tglCreateProgram();

    tglAttachShader(FinalShaderProgram, VertexShaderHandle);
    tglAttachShader(FinalShaderProgram, FragmentShaderHandle);
    tglLinkProgram(FinalShaderProgram);

    tglGetProgramiv(FinalShaderProgram, GL_LINK_STATUS, &Success);
    if(!Success) {
        tglGetProgramInfoLog(FinalShaderProgram, 512, NULL, InfoLog);

        Assert(false);
    }

    tglDeleteShader(VertexShaderHandle);
    tglDeleteShader(FragmentShaderHandle);

    return FinalShaderProgram;
}

struct TextureFile {
    i32 Width;
    i32 Height;
    i32 Channels;
    u8* Data;
};

Statuses LoadTextureFile(const char *Path, TextureFile *ReadedFile, bool32 VerticalFlip)
{
    stbi_set_flip_vertically_on_load_thread(VerticalFlip);

    u8* ImageData = stbi_load(Path, &ReadedFile->Width, &ReadedFile->Height, &ReadedFile->Channels, STBI_default);

    if (!ImageData) {
        // TODO (ismail): diagnostic and write to log some info
        Assert(ImageData);
        return Statuses::FileLoadFailed;
    }

    ReadedFile->Data = ImageData;

    return Statuses::Success;
}

void FreeTextureFile(TextureFile *ReadedFile)
{
    stbi_image_free(ReadedFile->Data);

    ReadedFile->Data = 0;
}

struct ShadersName {
    const char *VertexShaderName;
    const char *FragmentShaderName;
};

ShadersName ShaderProgramNames[ShaderProgramsType::ShaderProgramsTypeMax] = {
    { 
        "../mesh_component_shader.vs", 
        "../mesh_component_shader.fs" 
    },
    { 
        "../skeletal_mesh_component_shader.vs", 
        "../skeletal_mesh_component_shader.fs" 
    },
    { 
        "../particle_shader.vs", 
        "../particle_shader.fs" 
    },
    {
        "../debug_draw.vs", 
        "../debug_draw.fs" 
    }
    /*,
    { 
        "data/shaders/terrain_shader.vs", 
        "data/shaders/terrain_shader.fs" 
    },*/
};

#define DIFFUSE_TEXTURE_UNIT            GL_TEXTURE0
#define DIFFUSE_TEXTURE_UNIT_NUM        GL_TEXTURE_UNIT0

#define SPECULAR_EXPONENT_MAP_UNIT      GL_TEXTURE1
#define SPECULAR_EXPONENT_MAP_UNIT_NUM  GL_TEXTURE_UNIT1

void InitShaderProgramsCache(Platform *Platform)
{
    enum { StringBufferSize = 120 };

    char HelperStringBuffer[StringBufferSize];

    for (i32 Index = 0; Index < ShaderProgramsType::ShaderProgramsTypeMax; ++Index) {
        ShaderProgram*                  Shader                  = &ShadersProgramsCache[Index];
        ShadersName*                    Names                   = &ShaderProgramNames[Index];
        ShaderProgramVariablesStorage*  ShaderVariablesStorage  = &Shader->ProgramVarsStorage;

        u32 ShaderProgram = CreateShaderProgram(Platform, Names->VertexShaderName, Names->FragmentShaderName);

        ShaderVariablesStorage->Transform.ObjectToCameraSpaceTransformationLocation = tglGetUniformLocation(ShaderProgram, "ObjectToCameraSpaceTransformation");
        ShaderVariablesStorage->Transform.ObjectGeneralTransformationLocation       = tglGetUniformLocation(ShaderProgram, "ObjectGeneralTransformation");

        ShaderVariablesStorage->MaterialInfo.MaterialAmbientColorLocation   = tglGetUniformLocation(ShaderProgram, "MeshMaterial.AmbientColor");
        ShaderVariablesStorage->MaterialInfo.MaterialDiffuseColorLocation   = tglGetUniformLocation(ShaderProgram, "MeshMaterial.DiffuseColor");
        ShaderVariablesStorage->MaterialInfo.MaterialSpecularColorLocation  = tglGetUniformLocation(ShaderProgram, "MeshMaterial.SpecularColor");
        ShaderVariablesStorage->MaterialInfo.DiffuseTexture.Location        = tglGetUniformLocation(ShaderProgram, "DiffuseTexture");
        ShaderVariablesStorage->MaterialInfo.DiffuseTexture.Unit            = DIFFUSE_TEXTURE_UNIT;
        ShaderVariablesStorage->MaterialInfo.DiffuseTexture.UnitNum         = DIFFUSE_TEXTURE_UNIT_NUM;
        ShaderVariablesStorage->MaterialInfo.SpecularExpMap.Location        = tglGetUniformLocation(ShaderProgram, "SpecularExponentMap");
        ShaderVariablesStorage->MaterialInfo.SpecularExpMap.Unit            = SPECULAR_EXPONENT_MAP_UNIT;
        ShaderVariablesStorage->MaterialInfo.SpecularExpMap.UnitNum         = SPECULAR_EXPONENT_MAP_UNIT_NUM;

        ShaderVariablesStorage->Animation.BoneIDLocation    = tglGetUniformLocation(ShaderProgram, "BoneID");

        ShaderVariablesStorage->Light.DirectionalLightSpecLocations.ColorLocation               = tglGetUniformLocation(ShaderProgram, "SceneDirectionalLight.Specification.Color");
        ShaderVariablesStorage->Light.DirectionalLightSpecLocations.IntensityLocation           = tglGetUniformLocation(ShaderProgram, "SceneDirectionalLight.Specification.Intensity");
        ShaderVariablesStorage->Light.DirectionalLightSpecLocations.AmbientIntensityLocation    = tglGetUniformLocation(ShaderProgram, "SceneDirectionalLight.Specification.AmbientIntensity");
        ShaderVariablesStorage->Light.DirectionalLightSpecLocations.SpecularIntensityLocation   = tglGetUniformLocation(ShaderProgram, "SceneDirectionalLight.Specification.SpecularIntensity");
        ShaderVariablesStorage->Light.DirectionalLightDirectionLocation                         = tglGetUniformLocation(ShaderProgram, "SceneDirectionalLight.Direction");

        ShaderVariablesStorage->Light.ViewerPositionLocation    = tglGetUniformLocation(ShaderProgram, "ViewerPosition");
        ShaderVariablesStorage->Light.PointLightsAmountLocation = tglGetUniformLocation(ShaderProgram, "PointLightsAmount");
        ShaderVariablesStorage->Light.SpotLightsAmountLocation  = tglGetUniformLocation(ShaderProgram, "SpotLightsAmount");

        for (i32 Index = 0; Index < MAX_POINTS_LIGHTS; ++Index) {
            ShaderProgramVariablesStorage::LightWork::PointLightLocations* CurrentPointLight = &ShaderVariablesStorage->Light.PointLightsLocations[Index];
            
            snprintf(HelperStringBuffer, sizeof(HelperStringBuffer), "PointLights[%d].Specification.Color", Index);
            CurrentPointLight->SpecLocation.ColorLocation = tglGetUniformLocation(ShaderProgram, HelperStringBuffer);

            snprintf(HelperStringBuffer, sizeof(HelperStringBuffer), "PointLights[%d].Specification.Intensity", Index);
            CurrentPointLight->SpecLocation.IntensityLocation = tglGetUniformLocation(ShaderProgram, HelperStringBuffer);

            snprintf(HelperStringBuffer, sizeof(HelperStringBuffer), "PointLights[%d].Specification.AmbientIntensity", Index);
            CurrentPointLight->SpecLocation.AmbientIntensityLocation = tglGetUniformLocation(ShaderProgram, HelperStringBuffer);

            snprintf(HelperStringBuffer, sizeof(HelperStringBuffer), "PointLights[%d].Specification.SpecularIntensity", Index);
            CurrentPointLight->SpecLocation.SpecularIntensityLocation = tglGetUniformLocation(ShaderProgram, HelperStringBuffer);

            snprintf(HelperStringBuffer, sizeof(HelperStringBuffer), "PointLights[%d].AttenuationSpec.Position", Index);
            CurrentPointLight->AttenuationLocation.PositionLocation = tglGetUniformLocation(ShaderProgram, HelperStringBuffer);

            snprintf(HelperStringBuffer, sizeof(HelperStringBuffer), "PointLights[%d].AttenuationSpec.DisctanceMax", Index);
            CurrentPointLight->AttenuationLocation.DisctanceMaxLocation = tglGetUniformLocation(ShaderProgram, HelperStringBuffer);

            snprintf(HelperStringBuffer, sizeof(HelperStringBuffer), "PointLights[%d].AttenuationSpec.DisctanceMin", Index);
            CurrentPointLight->AttenuationLocation.DisctanceMinLocation = tglGetUniformLocation(ShaderProgram, HelperStringBuffer);

            snprintf(HelperStringBuffer, sizeof(HelperStringBuffer), "PointLights[%d].AttenuationSpec.AttenuationFactor", Index);
            CurrentPointLight->AttenuationLocation.AttenuationFactorLocation = tglGetUniformLocation(ShaderProgram, HelperStringBuffer);
        }

        for (i32 Index = 0; Index < MAX_SPOT_LIGHTS; ++Index) {
            ShaderProgramVariablesStorage::LightWork::SpotLightLocations* CurrentSpotLight = &ShaderVariablesStorage->Light.SpotLightsLocations[Index];
            
            snprintf(HelperStringBuffer, sizeof(HelperStringBuffer), "SpotLights[%d].Specification.Color", Index);
            CurrentSpotLight->SpecLocation.ColorLocation = tglGetUniformLocation(ShaderProgram, HelperStringBuffer);

            snprintf(HelperStringBuffer, sizeof(HelperStringBuffer), "SpotLights[%d].Specification.Intensity", Index);
            CurrentSpotLight->SpecLocation.IntensityLocation = tglGetUniformLocation(ShaderProgram, HelperStringBuffer);

            snprintf(HelperStringBuffer, sizeof(HelperStringBuffer), "SpotLights[%d].Specification.AmbientIntensity", Index);
            CurrentSpotLight->SpecLocation.AmbientIntensityLocation = tglGetUniformLocation(ShaderProgram, HelperStringBuffer);

            snprintf(HelperStringBuffer, sizeof(HelperStringBuffer), "SpotLights[%d].Specification.SpecularIntensity", Index);
            CurrentSpotLight->SpecLocation.SpecularIntensityLocation = tglGetUniformLocation(ShaderProgram, HelperStringBuffer);

            snprintf(HelperStringBuffer, sizeof(HelperStringBuffer), "SpotLights[%d].AttenuationSpec.Position", Index);
            CurrentSpotLight->AttenuationLocation.PositionLocation = tglGetUniformLocation(ShaderProgram, HelperStringBuffer);

            snprintf(HelperStringBuffer, sizeof(HelperStringBuffer), "SpotLights[%d].AttenuationSpec.DisctanceMax", Index);
            CurrentSpotLight->AttenuationLocation.DisctanceMaxLocation = tglGetUniformLocation(ShaderProgram, HelperStringBuffer);

            snprintf(HelperStringBuffer, sizeof(HelperStringBuffer), "SpotLights[%d].AttenuationSpec.DisctanceMin", Index);
            CurrentSpotLight->AttenuationLocation.DisctanceMinLocation = tglGetUniformLocation(ShaderProgram, HelperStringBuffer);

            snprintf(HelperStringBuffer, sizeof(HelperStringBuffer), "SpotLights[%d].AttenuationSpec.AttenuationFactor", Index);
            CurrentSpotLight->AttenuationLocation.AttenuationFactorLocation = tglGetUniformLocation(ShaderProgram, HelperStringBuffer);

            snprintf(HelperStringBuffer, sizeof(HelperStringBuffer), "SpotLights[%d].Direction", Index);
            CurrentSpotLight->DirectionLocation = tglGetUniformLocation(ShaderProgram, HelperStringBuffer);

            snprintf(HelperStringBuffer, sizeof(HelperStringBuffer), "SpotLights[%d].CosCutoffAngle", Index);
            CurrentSpotLight->CosCutoffAngleLocation = tglGetUniformLocation(ShaderProgram, HelperStringBuffer);

            snprintf(HelperStringBuffer, sizeof(HelperStringBuffer), "SpotLights[%d].CutoffAttenuationFactor", Index);
            CurrentSpotLight->CutoffAttenuationFactorLocation = tglGetUniformLocation(ShaderProgram, HelperStringBuffer);
        }

        if (Index == ShaderProgramsType::SkeletalMeshShader) {
            for (i32 MatrixIndex = 0; MatrixIndex < MAX_BONES; ++MatrixIndex) {
                i32* MatrixLoc = &Shader->ProgramVarsStorage.Animation.AnimationMatricesLocation[MatrixIndex];

                snprintf(HelperStringBuffer, sizeof(HelperStringBuffer), "AnimationBonesMatrices[%d]", MatrixIndex);

                *MatrixLoc = tglGetUniformLocation(ShaderProgram, HelperStringBuffer);
            }
        }

        tglUseProgram(ShaderProgram);

        tglUniform1i(ShaderVariablesStorage->MaterialInfo.DiffuseTexture.Location, ShaderVariablesStorage->MaterialInfo.DiffuseTexture.UnitNum);
        tglUniform1i(ShaderVariablesStorage->MaterialInfo.SpecularExpMap.Location, ShaderVariablesStorage->MaterialInfo.SpecularExpMap.UnitNum);

        tglUseProgram(0);

        Shader->Program = ShaderProgram;
    }
}

void LoadTerrain(Platform *Platform, Terrain *ToLoad, const char *TerrainTerxtureName)
{
    TerrainLoadFile TerrainFile = {};

    WorldTransform* TerrainTransform = &ToLoad->Transform;

    TerrainTransform->Position  = { 0.0f, 0.0f, 0.0f };
    TerrainTransform->Rotation  = { 0.0f, 0.0f, 0.0f };
    TerrainTransform->Scale     = { 1.0f, 1.0f, 1.0f };

    TerrainFile.VerticesAmount  = sizeof(TerrainFile.Vertices) / sizeof(*TerrainFile.Vertices);
    TerrainFile.TexturesAmount  = sizeof(TerrainFile.Textures) / sizeof(*TerrainFile.Textures);
    TerrainFile.NormalsAmount   = sizeof(TerrainFile.Normals)  / sizeof(*TerrainFile.Normals);
    TerrainFile.IndicesAmount   = sizeof(TerrainFile.Indices)  / sizeof(*TerrainFile.Indices);

    GenerateTerrainMesh(&TerrainFile, 4.0f, 200.0f, BATTLE_AREA_GRID_VERT_AMOUNT);
    LoadTerrainFile(ToLoad, &TerrainFile);

    TextureFile TerrainTexture = {};
    if (LoadTextureFile(TerrainTerxtureName, &TerrainTexture, 0) != Statuses::Success) {
        Assert(false);
    }

    u32 TerrainTextureHandle;

    glGenTextures(1, &TerrainTextureHandle);
    glBindTexture(GL_TEXTURE_2D, TerrainTextureHandle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TerrainTexture.Width, TerrainTexture.Height, 0, GL_RGB, GL_UNSIGNED_BYTE, TerrainTexture.Data);
    tglGenerateMipmap(GL_TEXTURE_2D);

    ToLoad->IndicesAmount   = TerrainFile.IndicesAmount;
    ToLoad->TextureHandle   = TerrainTextureHandle;

    glBindTexture(GL_TEXTURE_2D, 0);

    FreeTextureFile(&TerrainTexture);
}

void AllocateDebugObjFile(ObjFile *File)
{
    File->Meshes        = (Mesh*)   VirtualAlloc(0, sizeof(Mesh) *     10,   MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    File->Positions     = (Vec3*)   VirtualAlloc(0, sizeof(Vec3) * 140000,   MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    File->Normals       = (Vec3*)   VirtualAlloc(0, sizeof(Vec3) * 140000,   MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    File->TextureCoord  = (Vec2*)   VirtualAlloc(0, sizeof(Vec2) * 140000,   MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    File->Indices       = (u32*)    VirtualAlloc(0, sizeof(u32)  * 140000,   MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

void InitMeshComponent(Platform *Platform, MeshComponent *ToLoad, ObjFileLoaderFlags  LoadFlags)
{
    u32*                Buffers     = ToLoad->BuffersHandler;
    ObjFile             LoadFile    = {};

    AllocateDebugObjFile(&LoadFile);

    LoadObjFile(ToLoad->ObjectPath, &LoadFile, LoadFlags);

    MeshComponentObjects* ComponentObjects = (MeshComponentObjects*)VirtualAlloc(0, sizeof(MeshComponentObjects) * LoadFile.MeshesCount, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    tglGenVertexArrays(1, &Buffers[OpenGLBuffersLocation::GLVertexArrayLocation]); 
    tglBindVertexArray(Buffers[OpenGLBuffersLocation::GLVertexArrayLocation]);

    tglGenBuffers(OpenGLBuffersLocation::GLLocationMax - 1, Buffers); // NOTE(Ismail): -1 because we already allocate GLVertexArray

    tglBindBuffer(GL_ARRAY_BUFFER, Buffers[OpenGLBuffersLocation::GLPositionLocation]);
    tglBufferData(GL_ARRAY_BUFFER, sizeof(*LoadFile.Positions) * LoadFile.PositionsCount, LoadFile.Positions, GL_STATIC_DRAW);
    tglVertexAttribPointer(OpenGLBuffersLocation::GLPositionLocation, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    tglEnableVertexAttribArray(OpenGLBuffersLocation::GLPositionLocation);

    tglBindBuffer(GL_ARRAY_BUFFER, Buffers[OpenGLBuffersLocation::GLTextureLocation]);
    tglBufferData(GL_ARRAY_BUFFER, sizeof(*LoadFile.TextureCoord) * LoadFile.TexturesCount, LoadFile.TextureCoord, GL_STATIC_DRAW);
    tglVertexAttribPointer(OpenGLBuffersLocation::GLTextureLocation, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    tglEnableVertexAttribArray(OpenGLBuffersLocation::GLTextureLocation);

    tglBindBuffer(GL_ARRAY_BUFFER, Buffers[OpenGLBuffersLocation::GLNormalsLocation]);
    tglBufferData(GL_ARRAY_BUFFER, sizeof(*LoadFile.Normals) * LoadFile.NormalsCount, LoadFile.Normals, GL_STATIC_DRAW);
    tglVertexAttribPointer(OpenGLBuffersLocation::GLNormalsLocation, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    tglEnableVertexAttribArray(OpenGLBuffersLocation::GLNormalsLocation);

    tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[OpenGLBuffersLocation::GLIndexArrayLocation]);
    tglBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*LoadFile.Indices) * LoadFile.IndicesCount, LoadFile.Indices, GL_STATIC_DRAW);

    tglBindVertexArray(0);
    tglBindBuffer(GL_ARRAY_BUFFER, 0);
    tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    for (i32 MeshesIndex = 0; MeshesIndex < LoadFile.MeshesCount; ++MeshesIndex) {
        MeshComponentObjects*   CurrentComponentObject      = &ComponentObjects[MeshesIndex];
        MeshMaterial*           CurrentComponentMaterial    = &CurrentComponentObject->Material;
        Mesh*                   CurrentMesh                 = &LoadFile.Meshes[MeshesIndex];
        Material*               CurrentMeshMaterial         = &CurrentMesh->Material;

        CurrentComponentObject->IndexOffset     = CurrentMesh->IndexOffset;
        CurrentComponentObject->VertexOffset    = CurrentMesh->VertexOffset;
        CurrentComponentObject->NumIndices      = CurrentMesh->IndicesAmount;
    
        if (CurrentMeshMaterial->HaveTexture) {
            TextureFile Texture = {};

            if (LoadTextureFile(CurrentMeshMaterial->TextureFilePath, &Texture, 1) != Statuses::Success) {
                Assert(false);
            }

            CurrentComponentMaterial->HaveTexture = 1;
    
            glGenTextures(1, &CurrentComponentMaterial->TextureHandle);
            glBindTexture(GL_TEXTURE_2D, CurrentComponentMaterial->TextureHandle);
        
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Texture.Width, Texture.Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture.Data);
            tglGenerateMipmap(GL_TEXTURE_2D);
        
            FreeTextureFile(&Texture);
    
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        if (CurrentMeshMaterial->HaveSpecularExponent) {
            TextureFile SpecularTexture = {};

            if (LoadTextureFile(CurrentMeshMaterial->SpecularExpFilePath, &SpecularTexture, 1) != Statuses::Success) {
                Assert(false);
            }

            CurrentComponentMaterial->HaveSpecularExponent = 1;
    
            glGenTextures(1, &CurrentComponentMaterial->SpecularExponentMapTextureHandle);
            glBindTexture(GL_TEXTURE_2D, CurrentComponentMaterial->SpecularExponentMapTextureHandle);
        
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SpecularTexture.Width, SpecularTexture.Height, 0, GL_RED, GL_UNSIGNED_BYTE, SpecularTexture.Data);
        
            FreeTextureFile(&SpecularTexture);
    
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        CurrentComponentMaterial->AmbientColor  = CurrentMeshMaterial->AmbientColor;
        CurrentComponentMaterial->DiffuseColor  = CurrentMeshMaterial->DiffuseColor;
        CurrentComponentMaterial->SpecularColor = CurrentMeshMaterial->SpecularColor;
    }

    ToLoad->MeshesAmount    = LoadFile.MeshesCount;
    ToLoad->MeshesInfo      = ComponentObjects;
}

void InitParticleSystem(ParticleSystem *Sys)
{
    Vec3 ParticleSquareAppearance[4] {
        {  0.5f,  0.5f, 0.0f },
        {  0.5f, -0.5f, 0.0f },
        { -0.5f, -0.5f, 0.0f },
        { -0.5f,  0.5f, 0.0f },
    };

    u32 ParticlesIndices[6] {
        0, 1, 2,
        2, 3, 0
    };

    u32* Buffers = Sys->BuffersHandler;

    Sys->IndicesAmount = sizeof(ParticlesIndices) / sizeof(*ParticlesIndices);

    tglGenVertexArrays(1, &Buffers[OpenGLBuffersLocation::GLVertexArrayLocation]); 
    tglBindVertexArray(Buffers[OpenGLBuffersLocation::GLVertexArrayLocation]);

    tglGenBuffers(OpenGLBuffersLocation::GLLocationMax - 1, Buffers); // NOTE(Ismail): -1 because we already allocate GLVertexArray

    tglBindBuffer(GL_ARRAY_BUFFER, Buffers[OpenGLBuffersLocation::GLPositionLocation]);
    tglBufferData(GL_ARRAY_BUFFER, sizeof(ParticleSquareAppearance), ParticleSquareAppearance, GL_STATIC_DRAW);
    tglVertexAttribPointer(OpenGLBuffersLocation::GLPositionLocation, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    tglEnableVertexAttribArray(OpenGLBuffersLocation::GLPositionLocation);

    tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[OpenGLBuffersLocation::GLIndexArrayLocation]);
    tglBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ParticlesIndices), ParticlesIndices, GL_STATIC_DRAW);

    tglBindVertexArray(0);
    tglBindBuffer(GL_ARRAY_BUFFER, 0);
    tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

struct AABBCollider {
    Vec3    Center;
    Vec3    Extens;
};

struct ColliderDebugDraw {
    Vec3    LinesColor;
    u32     BuffersHandler[GLLocationMax];
};

bool32 TestAABBIntersection(AABBCollider* A, AABBCollider* B)
{
    if (fabsf(A->Center.x - B->Center.x) > (A->Extens.x + B->Extens.x)) return 0;
    if (fabsf(A->Center.y - B->Center.y) > (A->Extens.y + B->Extens.y)) return 0;
    if (fabsf(A->Center.z - B->Center.z) > (A->Extens.z + B->Extens.z)) return 0;

    return 1;
}

void InitColliderDebugDraw(AABBCollider *Collider, ColliderDebugDraw *DebugDraw)
{
    Vec3 ColliderDebugSquareAppearance[8] {
        {  -0.5f,  -0.5f, -0.5f },
        {   0.5f,  -0.5f, -0.5f },
        {   0.5f,  -0.5f, -0.5f },
        {   0.5f,  -0.5f,  0.5f },
        {   0.5f,  -0.5f,  0.5f },
        {  -0.5f,  -0.5f,  0.5f },
        {  -0.5f,  -0.5f,  0.5f },
        {  -0.5f,  -0.5f, -0.5f },
    };

    u32* Buffers = DebugDraw->BuffersHandler;

    tglGenVertexArrays(1, &Buffers[OpenGLBuffersLocation::GLVertexArrayLocation]); 
    tglBindVertexArray(Buffers[OpenGLBuffersLocation::GLVertexArrayLocation]);

    tglGenBuffers(OpenGLBuffersLocation::GLLocationMax - 1, Buffers); // NOTE(Ismail): -1 because we already allocate GLVertexArray

    tglBindBuffer(GL_ARRAY_BUFFER, Buffers[OpenGLBuffersLocation::GLPositionLocation]);
    tglBufferData(GL_ARRAY_BUFFER, sizeof(ColliderDebugSquareAppearance), ColliderDebugSquareAppearance, GL_STATIC_DRAW);
    tglVertexAttribPointer(OpenGLBuffersLocation::GLPositionLocation, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    tglEnableVertexAttribArray(OpenGLBuffersLocation::GLPositionLocation);

    tglBindVertexArray(0);
    tglBindBuffer(GL_ARRAY_BUFFER, 0);
}

struct MeshLoaderNode {
    const char*         ObjName;
    ObjFileLoaderFlags  Flags;
    Vec3                InitialScale;
};

const MeshLoaderNode SceneObjectsName[] = {
    {
        "data/obj/Golem.obj",
        {
            /* GenerateSmoothNormals */  1,
            /* SelfGenerateNormals   */  0
        },
        {
            1.0f,
            1.0f,
            1.0f
        }
    },
    {
        "data/obj/antique_ceramic_vase_01_4k.obj",
        {
            /* GenerateSmoothNormals */  0,
            /* SelfGenerateNormals   */  0
        },
        {
            4.0f,
            4.0f,
            4.0f
        }
    },
    {
        "data/obj/cube_brick_texture.obj",
        {
            /* GenerateSmoothNormals */  0,
            /* SelfGenerateNormals   */  0
        },
        {
            1.0f,
            1.0f,
            1.0f
        }
    }
};

struct DynamicSceneObjectLoader {
    const char* Mesh;
    const char* Animations[MAX_CHARACTER_ANIMATIONS];
    i32         Amount;
};

DynamicSceneObjectLoader DynamicSceneObjectsName[] = {
    {
        "data/obj/Idle.gltf",
        {
            "data/obj/Walk.gltf",
        },
        1
    },
};

void SetupDirectionalLight(DirectionalLight* Light, ShaderProgramsType ShaderType)
{
    ShaderProgramVariablesStorage *VarStorage = &ShadersProgramsCache[ShaderType].ProgramVarsStorage;

    tglUniform3fv(VarStorage->Light.DirectionalLightDirectionLocation, 1, &Light->Direction[0]);
    tglUniform3fv(VarStorage->Light.DirectionalLightSpecLocations.ColorLocation, 1, &Light->Specification.Color[0]);

    tglUniform1f(VarStorage->Light.DirectionalLightSpecLocations.IntensityLocation, Light->Specification.Intensity);
    tglUniform1f(VarStorage->Light.DirectionalLightSpecLocations.AmbientIntensityLocation, Light->Specification.AmbientIntensity);
    tglUniform1f(VarStorage->Light.DirectionalLightSpecLocations.SpecularIntensityLocation, Light->Specification.SpecularIntensity);
}

void SetupPointLights(PointLight* Lights, u32 LightAmount, ShaderProgramsType ShaderType, Vec3* ObjectPosition)
{
    ShaderProgramVariablesStorage *VarStorage = &ShadersProgramsCache[ShaderType].ProgramVarsStorage;

    for (u32 Index = 0; Index < LightAmount; ++Index) {
        PointLight* CurrentPointLight = &Lights[Index];
        ShaderProgramVariablesStorage::LightWork::PointLightLocations* PointLightsVarLocations = &VarStorage->Light.PointLightsLocations[Index];

        Vec3 LightInObjectUprightPosition  = CurrentPointLight->Attenuation.Position - *ObjectPosition;

        tglUniform3fv(PointLightsVarLocations->AttenuationLocation.PositionLocation, 1, &LightInObjectUprightPosition[0]);
        tglUniform1f(PointLightsVarLocations->AttenuationLocation.DisctanceMaxLocation, CurrentPointLight->Attenuation.DisctanceMax);
        tglUniform1f(PointLightsVarLocations->AttenuationLocation.DisctanceMinLocation, CurrentPointLight->Attenuation.DisctanceMin);
        tglUniform1f(PointLightsVarLocations->AttenuationLocation.AttenuationFactorLocation, CurrentPointLight->Attenuation.AttenuationFactor);

        tglUniform3fv(PointLightsVarLocations->SpecLocation.ColorLocation, 1, &CurrentPointLight->Specification.Color[0]);
        tglUniform1f(PointLightsVarLocations->SpecLocation.IntensityLocation, CurrentPointLight->Specification.Intensity);
        tglUniform1f(PointLightsVarLocations->SpecLocation.AmbientIntensityLocation, CurrentPointLight->Specification.AmbientIntensity);
        tglUniform1f(PointLightsVarLocations->SpecLocation.SpecularIntensityLocation, CurrentPointLight->Specification.SpecularIntensity);
    }

    tglUniform1i(VarStorage->Light.PointLightsAmountLocation, LightAmount);
}

void SetupSpotLights(SpotLight* Lights, u32 LightAmount, ShaderProgramsType ShaderType, Vec3* ObjectPosition)
{
    ShaderProgramVariablesStorage *VarStorage = &ShadersProgramsCache[ShaderType].ProgramVarsStorage;

    for (u32 Index = 0; Index < LightAmount; ++Index) {
        SpotLight* CurrentSpotLight = &Lights[Index];
        ShaderProgramVariablesStorage::LightWork::SpotLightLocations* SpotLightsVarLocations = &VarStorage->Light.SpotLightsLocations[Index];

        Vec3 LightInObjectUprightPosition  = CurrentSpotLight->Attenuation.Position - *ObjectPosition;

        tglUniform3fv(SpotLightsVarLocations->AttenuationLocation.PositionLocation, 1, &LightInObjectUprightPosition[0]);
        tglUniform1f(SpotLightsVarLocations->AttenuationLocation.DisctanceMaxLocation, CurrentSpotLight->Attenuation.DisctanceMax);
        tglUniform1f(SpotLightsVarLocations->AttenuationLocation.DisctanceMinLocation, CurrentSpotLight->Attenuation.DisctanceMin);
        tglUniform1f(SpotLightsVarLocations->AttenuationLocation.AttenuationFactorLocation, CurrentSpotLight->Attenuation.AttenuationFactor);

        tglUniform3fv(SpotLightsVarLocations->SpecLocation.ColorLocation, 1, &CurrentSpotLight->Specification.Color[0]);
        tglUniform1f(SpotLightsVarLocations->SpecLocation.IntensityLocation, CurrentSpotLight->Specification.Intensity);
        tglUniform1f(SpotLightsVarLocations->SpecLocation.AmbientIntensityLocation, CurrentSpotLight->Specification.AmbientIntensity);
        tglUniform1f(SpotLightsVarLocations->SpecLocation.SpecularIntensityLocation, CurrentSpotLight->Specification.SpecularIntensity);

        tglUniform3fv(SpotLightsVarLocations->DirectionLocation, 1, &Lights->Direction[0]);
        tglUniform1f(SpotLightsVarLocations->CosCutoffAngleLocation, CurrentSpotLight->CosCutoffAngle);
        tglUniform1f(SpotLightsVarLocations->CutoffAttenuationFactorLocation, CurrentSpotLight->CutoffAttenuationFactor);
    }

    tglUniform1i(VarStorage->Light.SpotLightsAmountLocation, LightAmount);
}

struct iVec4 {
    i32 x, y, z, w;
};

struct uVec4 {
    union {
        struct {
            u32 x, y, z, w;
        };
        u32 ValueHolder[4];   
    };
};

struct u8Vec4 {
    union {
        struct {
            u8 x, y, z, w;
        };
        u32 ValueHolder;   
    };
};

inline void UfbxVec3Convert(ufbx_vec3 *From, Vec3 *To)
{
    To->x = From->x;
    To->y = From->y;
    To->z = From->z;
}

struct glTF2Primitives {
    Material    MeshMaterial;
    Vec3*       Positions;
    Vec3*       Normals;
    Vec2*       TextureCoord;
    Vec4*       BoneWeights;
    iVec4*      BoneIds;
    u32*        Indices;
    u32         PositionsCount;
    u32         NormalsCount;
    u32         TexturesCount;
    u32         BoneWeightsCount;
    u32         BoneIdsCount;
    u32         IndicesCount;
};

struct glTF2Mesh {
    glTF2Primitives MeshPrimitives[MAX_MESH_PRIMITIVES];
    i32             PrimitivesAmount;
};

struct glTF2File {
    Skinning            Skelet;
    AnimationsArray*    Animations;
    glTF2Mesh           Meshes[MAX_MESHES];
    i32                 MeshesAmount;
};

void MakeInverseTranslation(Mat4x4 *Result, real32 *Translation)
{
    *Result = {
        1.0f, 0.0f, 0.0f, -Translation[_x_],
        0.0f, 1.0f, 0.0f, -Translation[_y_],
        0.0f, 0.0f, 1.0f, -Translation[_z_],
        0.0f, 0.0f, 0.0f, 1.0f,
    };
}

void MakeInverseScale(Mat4x4 *Result, real32 *Scale)
{
    real32 x = 1.0f / Scale[_x_];
    real32 y = 1.0f / Scale[_y_];
    real32 z = 1.0f / Scale[_z_];

    *Result = {
           x, 0.0f, 0.0f, 0.0f,
        0.0f,    y, 0.0f, 0.0f,
        0.0f, 0.0f,    z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
}

void MakeInverseScaleMat3x3(Mat3x3 *Result, real32 *Scale)
{
    real32 x = 1.0f / Scale[_x_];
    real32 y = 1.0f / Scale[_y_];
    real32 z = 1.0f / Scale[_z_];

    *Result = {
           x,   0.0f,    0.0f,
        0.0f,      y,    0.0f,
        0.0f,   0.0f,       z
    };
}

// NOTE(ismail): here rotation represented as quaternion
void MakeInverseRotation(Mat4x4 *Result, real32 *Rotation)
{
    Quat Rot = {
        Rotation[_w_],
        Rotation[_x_],
        Rotation[_y_],
        Rotation[_z_],
    };

    Rot.ToUprightToObjectMat4(Result);
}

void ReadJointNode(Skinning* Skin, cgltf_node* Joint, JointsInfo* ParentJoint, i32& Index, cgltf_node** RootJoints, i32 Len)
{
    if (!Joint) {
        return;
    }

    JointsInfo* CurrentJointInfo    = &Skin->Joints[Index];
    std::string BoneName            = Joint->name;

    cgltf_float*    Translation = Joint->translation;
    cgltf_float*    Scale       = Joint->scale;
    cgltf_float*    Rotation    = Joint->rotation;

    Mat4x4 InverseTranslationMatrix = {};
    Mat4x4 InverseScaleMatrix       = {};
    Mat4x4 InverseRotationMatrix    = {};
    Mat4x4 ParentMatrix             = !ParentJoint ? Identity4x4 : ParentJoint->InverseBindMatrix;

    MakeInverseTranslation(&InverseTranslationMatrix, Translation);
    MakeInverseScale(&InverseScaleMatrix, Scale);
    MakeInverseRotation(&InverseRotationMatrix, Rotation);

    i32 ChildreJointsAmount = (i32)Joint->children_count;

    if (ParentJoint) {
        i32 ChildrenIndex = ParentJoint->ChildrenAmount;

        Assert(ChildrenIndex <= MAX_JOINT_CHILDREN_AMOUNT);

        ++ParentJoint->ChildrenAmount;

        ParentJoint->Children[ChildrenIndex] = CurrentJointInfo;
    }

    CurrentJointInfo->BoneName.resize(BoneName.size());

    CurrentJointInfo->BoneName              = BoneName;
    CurrentJointInfo->Parent                = ParentJoint;
    CurrentJointInfo->InverseBindMatrix     = InverseScaleMatrix * InverseRotationMatrix * InverseTranslationMatrix * ParentMatrix;
    CurrentJointInfo->DefaultScale          = { Scale[_x_], Scale[_y_], Scale[_z_] };
    CurrentJointInfo->DefaultRotation       = Rotation;
    CurrentJointInfo->DefaultTranslation    = { Translation[_x_], Translation[_y_], Translation[_z_] };

    i32 OriginalID = -1;
    for (i32 RootJointID = 0; RootJointID < Len; ++RootJointID) {
        cgltf_node** OriginalJoint = &RootJoints[RootJointID];

        if (*OriginalJoint == Joint) {
            OriginalID = OriginalJoint - RootJoints;

            break;
        }
    }

    Assert(OriginalID != -1);

    BoneIDs& Ids = Skin->Bones[BoneName];

    Ids.BoneID          = Index;
    Ids.OriginalBoneID  = OriginalID;

    ++Index;

    for (i32 ChildrenJointIndex = 0; ChildrenJointIndex < ChildreJointsAmount; ++ChildrenJointIndex) {
        ReadJointNode(Skin, Joint->children[ChildrenJointIndex], CurrentJointInfo, Index, RootJoints, Len);
    }
}

#define DEFAULT_BUFFER_SIZE 80000

void glTFReadAnimations(cgltf_animation* Animations, i32 AnimationsCount, AnimationsArray& AnimArray, Skinning& Skin)
{
    Assert(AnimationsCount == 1); // TODO(ismail): restrictions for now in future we should handle that case

    for (i32 AnimationIndex = 0, AnimationArrayIndex = AnimArray.AnimsAmount; 
            AnimationIndex < AnimationsCount; 
            ++AnimationIndex, ++AnimationArrayIndex) {
            cgltf_animation&    CurrentAnimation    = Animations[AnimationIndex];
            Animation&          AnimationNode       = AnimArray.Anims[AnimationArrayIndex];

            i32                             ChannelsCount   = (i32)CurrentAnimation.channels_count;
            cgltf_animation_channel*        Channels        = CurrentAnimation.channels;
            std::map<std::string, BoneIDs>& Bones           = Skin.Bones;

            real32          AnimationDuration   = 0.0f;
            i32             BoneIndex           = 0;
            cgltf_node*     LastTargetNode      = 0;
            AnimationFrame* Frame               = 0;

            for (i32 ChannelIndex = 0; ChannelIndex < ChannelsCount; ++ChannelIndex) {
                cgltf_animation_channel*    CurrentChannel  = &Channels[ChannelIndex];
                cgltf_animation_sampler*    CurrentSampler  = CurrentChannel->sampler;
                cgltf_node*                 TargetNode      = CurrentChannel->target_node;

                bool32 BoneFind = Bones.find(TargetNode->name) != Bones.end();
                if (!BoneFind) {
                    // TODO(Ismail): now we just continue but need to handle that case
                    Assert(false);
                }

                if (LastTargetNode != TargetNode) {
                    Frame           = &AnimationNode.PerBonesFrame[BoneIndex++];
                    LastTargetNode  = TargetNode;

                    BoneIDs&    Ids = Bones.at(TargetNode->name);

                    Frame->Target           = Ids.BoneID;
                    Frame->OriginalBoneID   = Ids.OriginalBoneID;
                }

                cgltf_animation_path_type   ChannelType         = CurrentChannel->target_path;
                cgltf_interpolation_type    InterpalationType   = CurrentSampler->interpolation;

                Assert(ChannelType != cgltf_animation_path_type::cgltf_animation_path_type_invalid &&
                       ChannelType != cgltf_animation_path_type::cgltf_animation_path_type_weights);

                Assert(CurrentSampler->input->count == CurrentSampler->output->count);

                AnimationTransformation* Transform;
                switch(ChannelType) {
                    case cgltf_animation_path_type::cgltf_animation_path_type_translation: {
                        Transform = &Frame->Transformations[ATranslation];

                        cgltf_accessor* TransformAccessor   = CurrentSampler->output;
                        i32             TransformsCount     = TransformAccessor->count;
                        for (i32 TransformIndex = 0; TransformIndex < TransformsCount; ++TransformIndex) {
                            Vec3 Elem = {};
                            cgltf_accessor_read_float(TransformAccessor, TransformIndex, Elem.ValueHolder, sizeof(Elem));

                            Transform->Transforms[TransformIndex].Translation = Elem;
                        }
                    } break;

                    case cgltf_animation_path_type::cgltf_animation_path_type_rotation: {
                        Transform = &Frame->Transformations[ARotation];

                        cgltf_accessor* TransformAccessor   = CurrentSampler->output;
                        i32             TransformsCount     = TransformAccessor->count;
                        for (i32 TransformIndex = 0; TransformIndex < TransformsCount; ++TransformIndex) {
                            real32 Elem[4] = {};
                            cgltf_accessor_read_float(TransformAccessor, TransformIndex, Elem, sizeof(Elem));

                            Quat *Rot = &Transform->Transforms[TransformIndex].Rotation;
                            Rot->w = Elem[_w_];
                            Rot->x = Elem[_x_];
                            Rot->y = Elem[_y_];
                            Rot->z = Elem[_z_];
                        }
                    } break;

                    case cgltf_animation_path_type::cgltf_animation_path_type_scale: {
                        Transform = &Frame->Transformations[AScale];

                        cgltf_accessor* TransformAccessor   = CurrentSampler->output;
                        i32             TransformsCount     = TransformAccessor->count;
                        for (i32 TransformIndex = 0; TransformIndex < TransformsCount; ++TransformIndex) {
                            Vec3 Elem = {};
                            cgltf_accessor_read_float(TransformAccessor, TransformIndex, Elem.ValueHolder, sizeof(Elem));

                            Transform->Transforms[TransformIndex].Scale = Elem;
                        }
                    } break;
                }

                i32 KeyframesAmount = (i32)CurrentSampler->input->count;

                Assert(KeyframesAmount <= MAX_KEYFRAMES);

                for (i32 KeyframeIndex = 0; KeyframeIndex < KeyframesAmount; ++KeyframeIndex) {
                    real32 Keyframe = 0.0f;
                    cgltf_accessor_read_float(CurrentSampler->input, KeyframeIndex, &Keyframe, sizeof(Keyframe));

                    Transform->Keyframes[KeyframeIndex] = Keyframe;

                    if (Keyframe > AnimationDuration) {
                        AnimationDuration = Keyframe;
                    }
                }
                
                Transform->Amount   = KeyframesAmount;
                Transform->IType    = InterpalationType == cgltf_interpolation_type::cgltf_interpolation_type_linear ? ILinear : IStep;
                Transform->Valid    = 1;
            }

            AnimationNode.MaxDuration  = AnimationDuration;
            AnimationNode.FramesAmount = BoneIndex;
        }

        AnimArray.AnimsAmount += AnimationsCount;
}

void glTFLoadFile(const char *Path, cgltf_data** Mesh)
{
    cgltf_options   LoadOptions = {};

    cgltf_result CallResult = cgltf_parse_file(&LoadOptions, Path, Mesh);

    if (CallResult != cgltf_result::cgltf_result_success) {
        Assert(false);
    }

    CallResult = cgltf_load_buffers(&LoadOptions, *Mesh, Path);

    if (CallResult != cgltf_result::cgltf_result_success) {
        Assert(false);
    }
}

void glTFReadAnimations(const char* Path, AnimationsArray* AnimArray, Skinning& Skin)
{
    cgltf_data* Mesh = 0;

    glTFLoadFile(Path, &Mesh);

    cgltf_animation*    Animations      = Mesh->animations;
    i32                 AnimationsCount = Mesh->animations_count;

    glTFReadAnimations(Animations, AnimationsCount, *AnimArray, Skin);

    cgltf_free(Mesh);
}

void glTFRead(const char *Path, Platform* Platform, glTF2File *FileOut)
{
    u32             IndicesRead;
    u32             PositionsRead;
    u32             NormalsRead;
    u32             TexturesCoordRead;
    u32             BoneWeightsRead;
    u32             BoneIdsRead;

    const cgltf_accessor* AccessorPositions;
    const cgltf_accessor* AccessorNormals;
    const cgltf_accessor* AccessorTexturesCoord;
    const cgltf_accessor* AccessorWeights;
    const cgltf_accessor* AccessorJoints;

    cgltf_options   LoadOptions = {};
    cgltf_data*     Mesh        = NULL;

    glTFLoadFile(Path, &Mesh);

    i32         MeshesCount = (i32)Mesh->meshes_count;
    glTF2Mesh*  MeshesOut   = FileOut->Meshes;

    Assert(MeshesCount <= MAX_MESHES);

    for (i32 MeshIndex = 0; MeshIndex < MeshesCount; ++MeshIndex) {
        cgltf_mesh* CurrentMesh     = &Mesh->meshes[MeshIndex];
        glTF2Mesh*  CurrentMeshOut  = &MeshesOut[MeshIndex];

        i32                 PrimitivesAmount    = (i32)CurrentMesh->primitives_count;
        cgltf_primitive*    PrimitivesBase      = CurrentMesh->primitives;
        glTF2Primitives*    MeshPrimitivesOut   = CurrentMeshOut->MeshPrimitives;

        Assert(PrimitivesAmount <= MAX_MESH_PRIMITIVES);
        
        for (i32 PrimitiveIndex = 0; PrimitiveIndex < PrimitivesAmount; ++PrimitiveIndex) {
            cgltf_primitive*    CurrentMeshPrimitive            = &PrimitivesBase[PrimitiveIndex];
            cgltf_material*     CurrentMeshPrimitiveMaterial    = CurrentMeshPrimitive->material;
            glTF2Primitives*    CurrentPrimitiveOut             = &MeshPrimitivesOut[PrimitiveIndex];
            Material*           CurrentPrimitiveMaterial        = &CurrentPrimitiveOut->MeshMaterial;

            Vec3*    Positions      = (Vec3*)   Platform->AllocMem(sizeof(*CurrentPrimitiveOut->Positions)      * DEFAULT_BUFFER_SIZE);
            Vec3*    Normals        = (Vec3*)   Platform->AllocMem(sizeof(*CurrentPrimitiveOut->Normals)        * DEFAULT_BUFFER_SIZE);
            Vec2*    TextureCoords  = (Vec2*)   Platform->AllocMem(sizeof(*CurrentPrimitiveOut->TextureCoord)   * DEFAULT_BUFFER_SIZE);
            iVec4*   BoneIDs        = (iVec4*)  Platform->AllocMem(sizeof(*CurrentPrimitiveOut->BoneIds)        * DEFAULT_BUFFER_SIZE);
            Vec4*    BoneWeights    = (Vec4*)   Platform->AllocMem(sizeof(*CurrentPrimitiveOut->BoneWeights)    * DEFAULT_BUFFER_SIZE);
            u32*     Indices        = (u32*)    Platform->AllocMem(sizeof(*CurrentPrimitiveOut->Indices)        * DEFAULT_BUFFER_SIZE * 2);

            Assert(CurrentMeshPrimitive->type == cgltf_primitive_type::cgltf_primitive_type_triangles);

            AccessorPositions     = cgltf_find_accessor(CurrentMeshPrimitive, cgltf_attribute_type::cgltf_attribute_type_position,    0);
            AccessorNormals       = cgltf_find_accessor(CurrentMeshPrimitive, cgltf_attribute_type::cgltf_attribute_type_normal,      0);
            AccessorTexturesCoord = cgltf_find_accessor(CurrentMeshPrimitive, cgltf_attribute_type::cgltf_attribute_type_texcoord,    0);
            AccessorWeights       = cgltf_find_accessor(CurrentMeshPrimitive, cgltf_attribute_type::cgltf_attribute_type_weights,     0);
            AccessorJoints        = cgltf_find_accessor(CurrentMeshPrimitive, cgltf_attribute_type::cgltf_attribute_type_joints,      0);

            Assert(AccessorPositions->component_type    == cgltf_component_type::cgltf_component_type_r_32f && 
                   AccessorPositions->type              == cgltf_type::cgltf_type_vec3 &&
                   AccessorPositions->stride            == 12);
            Assert(AccessorNormals->component_type  == cgltf_component_type::cgltf_component_type_r_32f && 
                   AccessorNormals->type            == cgltf_type::cgltf_type_vec3 &&
                   AccessorNormals->stride          == 12);
            Assert(AccessorTexturesCoord->component_type    == cgltf_component_type::cgltf_component_type_r_32f && 
                   AccessorTexturesCoord->type              == cgltf_type::cgltf_type_vec2 &&
                   AccessorTexturesCoord->stride            == 8);
            Assert(AccessorWeights->component_type  == cgltf_component_type::cgltf_component_type_r_32f &&
                   AccessorWeights->type            == cgltf_type_vec4 &&
                   AccessorWeights->stride          == 16);
            Assert(AccessorJoints->component_type   == cgltf_component_type::cgltf_component_type_r_8u &&
                   AccessorJoints->type             == cgltf_type_vec4 &&
                   AccessorJoints->stride           == 4);

            const cgltf_accessor* NextJoints    = cgltf_find_accessor(CurrentMeshPrimitive, cgltf_attribute_type::cgltf_attribute_type_joints,      1);
            const cgltf_accessor* NextWeights   = cgltf_find_accessor(CurrentMeshPrimitive, cgltf_attribute_type::cgltf_attribute_type_weights,     1);

            Assert(NextJoints == NULL && NextWeights == NULL);
            
            PositionsRead       = (u32)cgltf_accessor_unpack_floats(AccessorPositions,     (real32*)Positions,     DEFAULT_BUFFER_SIZE);
            NormalsRead         = (u32)cgltf_accessor_unpack_floats(AccessorNormals,       (real32*)Normals,       DEFAULT_BUFFER_SIZE);
            TexturesCoordRead   = (u32)cgltf_accessor_unpack_floats(AccessorTexturesCoord, (real32*)TextureCoords, DEFAULT_BUFFER_SIZE);
            BoneWeightsRead     = (u32)cgltf_accessor_unpack_floats(AccessorWeights,       (real32*)BoneWeights,   DEFAULT_BUFFER_SIZE);

            BoneIdsRead = (u32)cgltf_accessor_unpack_indices_32bit_package(AccessorJoints, BoneIDs, DEFAULT_BUFFER_SIZE);

            IndicesRead = (u32)cgltf_accessor_unpack_indices(CurrentMeshPrimitive->indices, Indices, sizeof(*Indices), DEFAULT_BUFFER_SIZE * 2);

            CurrentPrimitiveOut->Positions      = Positions;
            CurrentPrimitiveOut->Normals        = Normals;
            CurrentPrimitiveOut->TextureCoord   = TextureCoords;
            CurrentPrimitiveOut->BoneIds        = BoneIDs;
            CurrentPrimitiveOut->BoneWeights    = BoneWeights;
            CurrentPrimitiveOut->Indices        = Indices;

            CurrentPrimitiveOut->PositionsCount   = AccessorPositions->count;
            CurrentPrimitiveOut->NormalsCount     = AccessorNormals->count;
            CurrentPrimitiveOut->TexturesCount    = AccessorTexturesCoord->count;
            CurrentPrimitiveOut->BoneWeightsCount = AccessorWeights->count;
            CurrentPrimitiveOut->BoneIdsCount     = AccessorJoints->count;
            CurrentPrimitiveOut->IndicesCount     = CurrentMeshPrimitive->indices->count;

            Assert(CurrentMeshPrimitiveMaterial->has_pbr_metallic_roughness);

            CurrentPrimitiveMaterial->AmbientColor = { 1.0f, 1.0f, 1.0f };

            cgltf_pbr_metallic_roughness* Diffuse = &CurrentMeshPrimitiveMaterial->pbr_metallic_roughness;

            char* DiffuseTextureFileName = Diffuse->base_color_texture.texture->image->uri;

            memcpy_s(CurrentPrimitiveMaterial->TextureFilePath, 
                     sizeof(CurrentPrimitiveMaterial->TextureFilePath), 
                     DiffuseTextureFileName, 
                     strlen(DiffuseTextureFileName));
            
            CurrentPrimitiveMaterial->DiffuseColor = { Diffuse->base_color_factor[_x_], Diffuse->base_color_factor[_y_], Diffuse->base_color_factor[_z_] };
            CurrentPrimitiveMaterial->HaveTexture = 1;

            if (CurrentMeshPrimitiveMaterial->has_specular) {
                cgltf_specular* Specular = &CurrentMeshPrimitiveMaterial->specular;

                char *SpecularTextureFileName = Specular->specular_texture.texture->image->uri;
                memcpy_s(CurrentPrimitiveMaterial->SpecularExpFilePath, 
                         sizeof(CurrentPrimitiveMaterial->SpecularExpFilePath), 
                         SpecularTextureFileName, 
                         strlen(SpecularTextureFileName));

                CurrentPrimitiveMaterial->SpecularColor = { 
                    0.1f, //Specular->specular_color_factor[_x_], 
                    0.1f, //Specular->specular_color_factor[_y_], 
                    0.1f  //Specular->specular_color_factor[_z_] 
                };
                CurrentPrimitiveMaterial->HaveSpecularExponent  = 1;
            }
        }

        CurrentMeshOut->PrimitivesAmount = PrimitivesAmount;
    }

    FileOut->MeshesAmount = MeshesCount;

    if (Mesh->skins_count > 0) {

        Assert(Mesh->skins_count == 1);

        // TODO(Ismail): use already created inverse_bind_matrix in CurrentSkin
        cgltf_skin* CurrentSkin = &Mesh->skins[0];
        Skinning*   Skelet      = &FileOut->Skelet;

        u32             JointsCount = (u32)CurrentSkin->joints_count;
        cgltf_node**    Joints      = CurrentSkin->joints;
        cgltf_node*     RootJoint   = Joints[0];

        Skelet->Joints = (JointsInfo*)Platform->AllocMem(sizeof(*Skelet->Joints) * JointsCount);

        i32 IndexCounter = 0;

        ReadJointNode(Skelet, *Joints, NULL, IndexCounter, Joints, (i32)JointsCount);

        FileOut->Skelet.JointsAmount = JointsCount;

        i32 AnimationsCount = (i32)Mesh->animations_count;

        Assert(AnimationsCount == 1);

        cgltf_animation*    Animations  = Mesh->animations;
        AnimationsArray*    AnimArray   = FileOut->Animations;

        glTFReadAnimations(Animations, AnimationsCount, *AnimArray, *Skelet);
    }

    cgltf_free(Mesh);
}

const char *ResourceFolderLocation = "data/obj/";

void InitSkeletalMeshComponent(Platform *Platform, SkeletalMeshComponent *SkeletalMesh, DynamicSceneObjectLoader& Loader)
{
    char                FullFileName[500]   = {};
    glTF2File           LoadFile            = {};
    SkeletalComponent*  Skelet              = &SkeletalMesh->Skelet;

    LoadFile.Animations = (AnimationsArray*)Platform->AllocMem(sizeof(*LoadFile.Animations));

    glTFRead(SkeletalMesh->ObjectPath, Platform, &LoadFile);

    i32 AnimCount = Loader.Amount;
    for (i32 AnimIndex = 0; AnimIndex < AnimCount; ++AnimIndex) {
        const char* AnimPath = Loader.Animations[AnimIndex];

        glTFReadAnimations(AnimPath, LoadFile.Animations, LoadFile.Skelet);
    }

    i32         MeshesAmount    = LoadFile.MeshesAmount;
    glTF2Mesh*  Meshes          = LoadFile.Meshes;
    for (i32 MeshIndex = 0; MeshIndex < MeshesAmount; ++MeshIndex) {
        glTF2Mesh* CurrentMesh = &Meshes[MeshIndex];

        i32                 PrimitivesAmount    = CurrentMesh->PrimitivesAmount;
        glTF2Primitives*    Primitives          = CurrentMesh->MeshPrimitives;
        for (i32 PrimitiveIndex = 0; PrimitiveIndex < PrimitivesAmount; ++PrimitiveIndex) {
            glTF2Primitives*    CurrentPrimitive        = &Primitives[PrimitiveIndex];
            MeshPrimitives*     CurrentPrimitiveOut     = &SkeletalMesh->Primitives[PrimitiveIndex];
            u32*                CurrentPrimitiveBuffers = CurrentPrimitiveOut->BuffersHandler;
            Material*           CurrentPrimitiveMat     = &CurrentPrimitive->MeshMaterial;
            MeshMaterial*       CurrentPrimitiveOutMat  = &CurrentPrimitiveOut->Material;

            tglGenVertexArrays(1,  &CurrentPrimitiveBuffers[OpenGLBuffersLocation::GLVertexArrayLocation]); 
            tglBindVertexArray(CurrentPrimitiveBuffers[OpenGLBuffersLocation::GLVertexArrayLocation]);

            tglGenBuffers(OpenGLBuffersLocation::GLLocationMax - 1, CurrentPrimitiveBuffers);

            tglBindBuffer(GL_ARRAY_BUFFER, CurrentPrimitiveBuffers[OpenGLBuffersLocation::GLPositionLocation]);
            tglBufferData(GL_ARRAY_BUFFER, sizeof(*CurrentPrimitive->Positions) * CurrentPrimitive->PositionsCount, CurrentPrimitive->Positions, GL_STATIC_DRAW);
            tglVertexAttribPointer(OpenGLBuffersLocation::GLPositionLocation, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
            tglEnableVertexAttribArray(OpenGLBuffersLocation::GLPositionLocation);

            tglBindBuffer(GL_ARRAY_BUFFER, CurrentPrimitiveBuffers[OpenGLBuffersLocation::GLTextureLocation]);
            tglBufferData(GL_ARRAY_BUFFER, sizeof(*CurrentPrimitive->TextureCoord) * CurrentPrimitive->TexturesCount, CurrentPrimitive->TextureCoord, GL_STATIC_DRAW);
            tglVertexAttribPointer(OpenGLBuffersLocation::GLTextureLocation, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
            tglEnableVertexAttribArray(OpenGLBuffersLocation::GLTextureLocation);

            tglBindBuffer(GL_ARRAY_BUFFER, CurrentPrimitiveBuffers[OpenGLBuffersLocation::GLNormalsLocation]);
            tglBufferData(GL_ARRAY_BUFFER, sizeof(*CurrentPrimitive->Normals) * CurrentPrimitive->NormalsCount, CurrentPrimitive->Normals, GL_STATIC_DRAW);
            tglVertexAttribPointer(OpenGLBuffersLocation::GLNormalsLocation, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
            tglEnableVertexAttribArray(OpenGLBuffersLocation::GLNormalsLocation);

            tglBindBuffer(GL_ARRAY_BUFFER, CurrentPrimitiveBuffers[OpenGLBuffersLocation::GLBoneIndicesLocation]);
            tglBufferData(GL_ARRAY_BUFFER, sizeof(*CurrentPrimitive->BoneIds) * CurrentPrimitive->BoneIdsCount, CurrentPrimitive->BoneIds, GL_STATIC_DRAW);
            tglVertexAttribIPointer(OpenGLBuffersLocation::GLBoneIndicesLocation, 4, GL_INT, 0, (void*)0);
            tglEnableVertexAttribArray(OpenGLBuffersLocation::GLBoneIndicesLocation);

            tglBindBuffer(GL_ARRAY_BUFFER, CurrentPrimitiveBuffers[OpenGLBuffersLocation::GLBoneWeightsLocation]);
            tglBufferData(GL_ARRAY_BUFFER, sizeof(*CurrentPrimitive->BoneWeights) * CurrentPrimitive->BoneWeightsCount, CurrentPrimitive->BoneWeights, GL_STATIC_DRAW);
            tglVertexAttribPointer(OpenGLBuffersLocation::GLBoneWeightsLocation, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
            tglEnableVertexAttribArray(OpenGLBuffersLocation::GLBoneWeightsLocation);

            tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CurrentPrimitiveBuffers[OpenGLBuffersLocation::GLIndexArrayLocation]);
            tglBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*CurrentPrimitive->Indices) * CurrentPrimitive->IndicesCount, CurrentPrimitive->Indices, GL_STATIC_DRAW);

            tglBindVertexArray(0);
            tglBindBuffer(GL_ARRAY_BUFFER, 0);
            tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            if (CurrentPrimitiveMat->HaveTexture) {
                TextureFile Texture = {};

                snprintf(FullFileName, sizeof(FullFileName), "%s%s", ResourceFolderLocation, CurrentPrimitiveMat->TextureFilePath);

                if (LoadTextureFile(FullFileName, &Texture, 0) != Statuses::Success) {
                    Assert(false);
                }

                CurrentPrimitiveOutMat->HaveTexture = 1;

                glGenTextures(1, &CurrentPrimitiveOutMat->TextureHandle);
                glBindTexture(GL_TEXTURE_2D, CurrentPrimitiveOutMat->TextureHandle);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Texture.Width, Texture.Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture.Data);
                tglGenerateMipmap(GL_TEXTURE_2D);

                FreeTextureFile(&Texture);
    
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            if (CurrentPrimitiveMat->HaveSpecularExponent) {
                TextureFile SpecularTexture = {};

                snprintf(FullFileName, sizeof(FullFileName), "%s%s", ResourceFolderLocation, CurrentPrimitiveMat->SpecularExpFilePath);

                if (LoadTextureFile(FullFileName, &SpecularTexture, 1) != Statuses::Success) {
                    Assert(false);
                }

                CurrentPrimitiveOutMat->HaveSpecularExponent = 1;

                glGenTextures(1, &CurrentPrimitiveOutMat->SpecularExponentMapTextureHandle);
                glBindTexture(GL_TEXTURE_2D, CurrentPrimitiveOutMat->SpecularExponentMapTextureHandle);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SpecularTexture.Width, SpecularTexture.Height, 0, GL_RED, GL_UNSIGNED_BYTE, SpecularTexture.Data);

                FreeTextureFile(&SpecularTexture);
            
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            CurrentPrimitiveOutMat->AmbientColor  = CurrentPrimitiveMat->AmbientColor;
            CurrentPrimitiveOutMat->DiffuseColor  = CurrentPrimitiveMat->DiffuseColor;
            CurrentPrimitiveOutMat->SpecularColor = CurrentPrimitiveMat->SpecularColor;

            CurrentPrimitiveOut->InidicesAmount = CurrentPrimitive->IndicesCount;
        }

        SkeletalMesh->PrimitivesAmount = PrimitivesAmount;
    }

    Skelet->Skin        = LoadFile.Skelet;
    Skelet->Animations  = LoadFile.Animations;
}

/*
void FbxTestRead(Platform *Platform)
{
    u64 MaxPositions    = 80000;
    u64 MaxNormals      = 80000;
    u64 MaxTexures      = 80000;
    u64 MaxIndices      = MaxPositions * 2;

    struct FbxMesh {
        u32 NumVertices;
        u32 NumIndices;
    };

    const char *Path = "data/obj/SimpleTest.fbx";
    u64 PathSize = strlen("data/obj/SimpleTest.fbx");

    ufbx_string_view PathView(Path, PathSize);

    ufbx_scene *FbxScene = ufbx_load_file(PathView, NULL, NULL);

    u32*    Indices         = (u32*)Platform->AllocMem(sizeof(*Indices) * MaxIndices);
    Vec3*   Position        = (Vec3*)Platform->AllocMem(sizeof(*Position) * MaxPositions);
    Vec3*   Normals         = (Vec3*)Platform->AllocMem(sizeof(*Normals) * MaxNormals);
    Vec2*   TextureCoords   = (Vec2*)Platform->AllocMem(sizeof(*TextureCoords) * MaxTexures);

    u32*    FlatIndices         = (u32*)Platform->AllocMem(sizeof(*Indices) * MaxIndices);
    Vec3*   FlatPosition        = (Vec3*)Platform->AllocMem(sizeof(*Position) * MaxPositions);
    Vec3*   FlatNormals         = (Vec3*)Platform->AllocMem(sizeof(*Normals) * MaxNormals);
    Vec2*   FlatTextureCoords   = (Vec2*)Platform->AllocMem(sizeof(*TextureCoords) * MaxTexures);
    iVec4*  FlatBoneIds         = (iVec4*)Platform->AllocMem(sizeof(*FlatBoneIds) * MaxTexures);
    Vec4*   FlatBoneWeights     = (Vec4*)Platform->AllocMem(sizeof(*FlatBoneWeights) * MaxTexures);

    u64 MaxMeshAmount = FbxScene->meshes.count;

    for (u64 MeshIndex = 0; MeshIndex < MaxMeshAmount; ++MeshIndex) {
        ufbx_mesh*  Mesh            = FbxScene->meshes[MeshIndex];
        u64         AmountOfIndices = Mesh->num_indices;

        for (u64 Index = 0; Index < AmountOfIndices; ++Index) {
            ufbx_vertex_vec3*   VertexPositon           = &Mesh->vertex_position;
            u32                 VertexPositionsIndex    = VertexPositon->indices[Index];

            UfbxVec3Convert(&VertexPositon->values[VertexPositionsIndex], &FlatPosition[Index]);

            ufbx_vertex_vec3*   VertexNormal        = &Mesh->vertex_normal;
            u32                 VertexNormalIndex   = VertexNormal->indices[Index];

            UfbxVec3Convert(&VertexNormal->values[VertexNormalIndex], &FlatNormals[Index]);

            FlatIndices[Index] = Index;
        }

        if (Mesh->skin_deformers.count >= 1) {
            Assert(false);
        }

        ufbx_skin_deformer*     SkinDeformers           = Mesh->skin_deformers[0];
        ufbx_skin_vertex_list*  SkinVertices            = &SkinDeformers->vertices;
        ufbx_skin_weight_list*  SkinWeights             = &SkinDeformers->weights;
        u64                     AmountOfSkinVertices    = SkinVertices->count;
        for (u64 SkinVertexIndex = 0; SkinVertexIndex < AmountOfSkinVertices; ++SkinVertexIndex) {
            ufbx_skin_vertex*   SkinVertex      = &SkinVertices->data[SkinVertexIndex];
            u32                 WeightBegin     = SkinVertex->weight_begin;
            u32                 AmountOfWeights = SkinVertex->num_weights;
            
            for (u64 WeightsIndex = WeightsIndex; WeightsIndex < SkinCluster->num_weights; WeightsIndex++) {
                u32 Vertex = SkinCluster->vertices[WeightsIndex];
                real32 weight = SkinCluster->weights[WeightsIndex];

                for (int k = 0; k < 4; k++) {
                    if (temp[v].Weights[k] == 0.0f) {
                        temp[v].BoneIDs[k] = boneIndex;
                        temp[v].Weights[k] = weight;
                        break;
                    }
                }
            }
        }

    }

    for (u64 MeshIndex = 0; MeshIndex < MaxMeshAmount; ++MeshIndex) {
        ufbx_mesh* Mesh = FbxScene->meshes[MeshIndex];

        for () {

        }
        
        ufbx_vec3 *VertexPositions  = &Mesh->vertex_position[0];
        ufbx_vec3 *VertexNormals    = &Mesh->vertex_normal[0];
        ufbx_vec2 *VertexTextures   = &Mesh->vertex_uv[0];
    }
}
*/

/*
void AssimpFbxTestRead(Platform *Platform)
{
    Assimp::Importer Importer;
    u32 AssimpFlags = aiProcess_GenNormals | aiProcess_JoinIdenticalVertices;

    const aiScene *Scene = Importer.ReadFile("data/obj/SimpleTest.fbx", AssimpFlags);

    if (!Scene) {
        Assert(false);
    }

    i32 TotalVertices   = 0;
    i32 TotalIndices    = 0;
    i32 TotalBones      = 0;

    u32*    Indices     = (u32*)Platform->AllocMem(sizeof(*Indices)     * 100000);
    Vec3*   Position    = (Vec3*)Platform->AllocMem(sizeof(*Position)   * 10000);
    Vec3*   Normals     = (Vec3*)Platform->AllocMem(sizeof(*Normals)    * 10000);
    iVec4*  BoneIDs     = (iVec4*)Platform->AllocMem(sizeof(*BoneIDs)   * 10000);
    Vec4*   Weights     = (Vec4*)Platform->AllocMem(sizeof(*Weights)    * 10000);

    for (i32 MeshIndex = 0; MeshIndex < Scene->mNumMeshes; ++MeshIndex) {
        const aiMesh *Mesh = Scene->mMeshes[MeshIndex];

        i32 MeshVertices    = Mesh->mNumVertices;
        i32 MeshIndices     = Mesh->mNumFaces * 3;
        i32 MeshBones       = Mesh->mNumBones;

        TotalVertices   += MeshVertices;
        TotalIndices    += MeshIndices;
        TotalBones      += MeshBones;
        

        aiVector3D Vertices     = Mesh->mVertices[MeshIndex];
        aiVector3D Normals      = Mesh->mNormals[MeshIndex];



        if (TotalIndices > 100000 || TotalVertices > 10000) {
            Assert(false);
        }

        if (Mesh->HasBones()) {
            for (i32 BonesIndex = 0; BonesIndex < Mesh->mNumBones; ++BonesIndex) {
                const aiBone *Bone = Mesh->mBones[BonesIndex];

                for (i32 WeightsIndex = 0; WeightsIndex < Bone->mNumWeights; ++WeightsIndex) {
                    aiVertexWeight Weight = Bone->mWeights[WeightsIndex];
                }
            }
        }
    }
}
*/

inline real32 CalcT(real32 t, real32 StartKeyframe, real32 EndKeyframe)
{
    return 1.0f - ((EndKeyframe - t) / (EndKeyframe - StartKeyframe));
}

void PrepareFrame(Platform *Platform, GameContext *Cntx)
{
    /*
    Vec3 n = {
        0.0f,
        0.0f,
        1.0f
    };

    Vec3 Pos = {
        1.0f,
        0.0f,
        0.0f
    };

    Quat a(DEGREE_TO_RAD(45.0f), n);
    Quat b(DEGREE_TO_RAD(135.0f), n);

    Mat3x3 TestMat = {};

    Vec3 Test1 = b * Pos;
    b.ToMat3(&TestMat);
    Vec3 Test2 = TestMat * Pos;

    Quat DeltaQuat = Quat::Slerp(a, b, 1.0f);

    Quat RotQuat = (DeltaQuat * a);

    Vec3 WooDooMagic = RotQuat * Pos;
    */

    real32 TrianglePosition[] = {
        // position 1
        0.5f, -0.5f, 0.0f,
        // color 1
        0.0f, 0.0f, 1.0f,
        // position 2 
        -0.5f, -0.5f, 0.0f,
        // color 2
        0.0f, 1.0f, 0.0f,
        // position 3
        0.0f, 0.5f, 0.0f,
        // color 3
        1.0f, 0.0f, 1.0f,
        
    };

    real32 Colors[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };

    real32 RayPosition[] = {
        // position 1
        0.0f, 0.0f, 0.0f,
        // color 1
        0.0f, 0.0f, 1.0f,
        // position 2 
        0.5f, 0.0f, 0.0f,
        // color 2
        0.0f, 1.0f, 0.0f,
        
    };

    // FbxTestRead(Platform);
    // AssimpFbxTestRead(Platform);

    // NOTE(Ismail): values specified by glClearColor are clamped to the range [0,1]
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    
    glViewport(0, 0, Platform->ScreenOpt.ActualWidth, Platform->ScreenOpt.ActualHeight);
    
    InitShaderProgramsCache(Platform);

    Cntx->BoneID = 0;

    DirectionalLight* SceneMainLight                    = &Cntx->LightSource;
    SceneMainLight->Specification.Color                 = { 1.0f, 1.0f, 1.0f };
    SceneMainLight->Specification.Intensity             = 0.85f;
    SceneMainLight->Specification.AmbientIntensity      = 0.3;
    SceneMainLight->Specification.SpecularIntensity     = 1.0f;
    SceneMainLight->Direction                           = { 0.707, -0.707f, 0.0f };

    Vec3        PointLightPosition  = { 20.0, 12.0f, 10.0f };
    Vec3        PointLightColor     = { 1.0f, 0.3f, 0.3f };
    PointLight* ScenePointLights    = Cntx->PointLights;
    for (i32 Index = 0; Index < MAX_POINTS_LIGHTS; ++Index) {
        PointLight* CurrentScenePointLight = &ScenePointLights[Index];

        CurrentScenePointLight->Specification.Color             = PointLightColor;
        CurrentScenePointLight->Specification.Intensity         = 1.0f;
        CurrentScenePointLight->Specification.AmbientIntensity  = 0.2;
        CurrentScenePointLight->Specification.SpecularIntensity = 1.0f;
        CurrentScenePointLight->Attenuation.DisctanceMax        = 35.0f;
        CurrentScenePointLight->Attenuation.DisctanceMin        = 5.0f;
        CurrentScenePointLight->Attenuation.AttenuationFactor   = 2.0f;
        CurrentScenePointLight->Attenuation.Position            = PointLightPosition;

        PointLightPosition.x    += 40.f;
        PointLightColor         = { 0.3f, 0.3f, 1.0f };
    }

    SpotLight* SceneSpotLight = &Cntx->SpotLights[0];
    SceneSpotLight->Specification.Color                 = { 1.0f, 1.0f, 1.0f };
    SceneSpotLight->Specification.Intensity             = 0.0f;
    SceneSpotLight->Specification.AmbientIntensity      = 0.0f;
    SceneSpotLight->Specification.SpecularIntensity     = 0.0f;
    SceneSpotLight->Attenuation.DisctanceMax            = 75.0f;
    SceneSpotLight->Attenuation.DisctanceMin            = 50.0f;
    SceneSpotLight->Attenuation.AttenuationFactor       = 2.0f;
    SceneSpotLight->CutoffAttenuationFactor             = 2.0f;
    SceneSpotLight->CosCutoffAngle                      = cosf(DEGREE_TO_RAD(25.0f));

    real32 Position = 10.0f;
    for (i32 Index = 0; Index < sizeof(SceneObjectsName) / sizeof(*SceneObjectsName); ++Index) {
        SceneObject*            Object          = &Cntx->TestSceneObjects[Index];
        const MeshLoaderNode*   CurrentMeshNode = &SceneObjectsName[Index];

        Object->ObjMesh.ObjectPath = CurrentMeshNode->ObjName;

        Object->Transform.Rotation  = { 0.0f, 0.0f, 0.0f };
        Object->Transform.Position  = { 0.0f, 0.0f, Index == 1 ? 0.0f : Position };
        Object->Transform.Scale     = CurrentMeshNode->InitialScale;

        InitMeshComponent(Platform, &Object->ObjMesh, CurrentMeshNode->Flags);

        Position += 10.0f;
    }

    for (i32 Index = 0; Index < DYNAMIC_SCENE_OBJECTS_MAX; ++Index) {
        DynamicSceneObject&         Object          = Cntx->TestDynamocSceneObjects[Index];
        DynamicSceneObjectLoader&   CurrentObject   = DynamicSceneObjectsName[Index];

        Object.ObjMesh.ObjectPath  = CurrentObject.Mesh;

        InitSkeletalMeshComponent(Platform, &Object.ObjMesh, CurrentObject);

        Object.Transform.Rotation = { 0.0f, 0.0f, 0.0f };
        Object.Transform.Position = { 0.0f, 0.0f, Position };
        Object.Transform.Scale = { 0.04f, 0.04f, 0.04f };
        
        Position += 10.0f;
    }

    Cntx->TestSceneObjects[1].Nesting.Parent            = &Cntx->TestDynamocSceneObjects[0];
    Cntx->TestSceneObjects[1].Nesting.AttachedToBone    = "mixamorig5:LeftHand";

    Particle *SceneParticles = Cntx->SceneParticles;
    for (i32 Index = 0; Index < PARTICLES_MAX; ++Index) {
        Particle*       Particle            = &SceneParticles[Index];
        WorldTransform* ParticleTransform   = &Particle->Transform;

        *ParticleTransform = {};

        ParticleTransform->Scale    = { 5.0f, 5.0f, 5.0f };
        ParticleTransform->Position = { 0.0f,  0.0f, 2.0f };

        Particle->Velocity      = { 0.0f, 25.0f, 0.0f };
        Particle->Damping       = 0.8f;
        Particle->InverseMass   = 1.0f / 2.0f;

        Particle->Acceleration  = GRAVITY;
    }

    InitParticleSystem(&Cntx->ParticleSystem);

    LoadTerrain(Platform, &Cntx->Terrain, "data/textures/grass_texture.jpg");

    Cntx->TranslationDelta = 0.1f;
    Cntx->RotationDelta = 0.1f;
}

void ReadAndCalculateAnimationMatrices(Skinning* Skin, SkinningMatricesStorage* Matrices, Animation* Anim, JointsInfo* Joint, Mat4x4* ParentMat, real32 CurrentTime)
{
    BoneIDs& Ids = Skin->Bones[Joint->BoneName];

    i32             FramesAmount    = Anim->FramesAmount;
    AnimationFrame* Frames          = Anim->PerBonesFrame;
    AnimationFrame* CurrentFrame    = 0;

    for (i32 FrameIndex = 0; FrameIndex < FramesAmount; ++FrameIndex) {
        AnimationFrame* Tmp = &Frames[FrameIndex];

        if (Tmp->Target == Ids.BoneID) {
            CurrentFrame = Tmp;

            break;
        }
    }

    Mat4x4 Parent           = ParentMat ? *ParentMat : Identity4x4;
    Mat4x4 CurrentJointMat  = Identity4x4;

    if (CurrentFrame) {
        for (i32 TransformIndex = AScale; TransformIndex >= ATranslation; --TransformIndex) {
            AnimationTransformation*    Transform           = &CurrentFrame->Transformations[TransformIndex];
            Mat4x4                      CurrentIterationMat = {};
            bool32                      KeyframesFound      = 0;

            Assert(Transform->Valid);

            i32 KeyframesAmount     = Transform->Amount;
            i32 FirstKeyframeIndex  = 0;
            i32 SecondKeyframeIndex = 0;

            real32* Keyframes = Transform->Keyframes;
            for (i32 KeyframeIndex = 0; KeyframeIndex < KeyframesAmount; ++KeyframeIndex) {
                Assert(KeyframeIndex < (KeyframesAmount - 1));

                real32 FrKeyframe = Keyframes[KeyframeIndex];
                real32 ScKeyframe = Keyframes[KeyframeIndex + 1];
                if (FrKeyframe <= CurrentTime && CurrentTime <= ScKeyframe) {
                    FirstKeyframeIndex  = KeyframeIndex;
                    SecondKeyframeIndex = KeyframeIndex + 1;

                    KeyframesFound = 1;

                    break;
                }
                else if (FrKeyframe > CurrentTime) {
                    switch (TransformIndex) {

                        case ATranslation: {
                            MakeTranslationFromVec(&Joint->DefaultTranslation, &CurrentIterationMat); 
                        } break;

                        case ARotation: {
                            Joint->DefaultRotation.ToMat4(&CurrentIterationMat);
                        } break;

                        case AScale: {
                            MakeScaleFromVector(&Joint->DefaultScale, &CurrentIterationMat);
                        } break;
                    }

                    break;
                }
            }

            if (KeyframesFound) {
                TransformationStorage* FrAnimTransform = &Transform->Transforms[FirstKeyframeIndex];
                TransformationStorage* ScAnimTransform = &Transform->Transforms[SecondKeyframeIndex];

                if (Transform->IType == ILinear) {
                    real32 T = CalcT(CurrentTime, Keyframes[FirstKeyframeIndex], Keyframes[SecondKeyframeIndex]);

                    switch (TransformIndex) {

                        case ATranslation: {
                            Vec3 DeltaTranslation = Lerp(FrAnimTransform->Translation, ScAnimTransform->Translation, T);

                            Vec3 FinalTranslation = FrAnimTransform->Translation + DeltaTranslation;

                            MakeTranslationFromVec(&FinalTranslation, &CurrentIterationMat); 
                        } break;

                        case ARotation: {
                            Quat DeltaRotation = Quat::Slerp(FrAnimTransform->Rotation, ScAnimTransform->Rotation, T);
                        
                            Quat    FinalRotationQuat = DeltaRotation;
                            Mat4x4  FinalRotation = {};

                            FinalRotationQuat.ToMat4(&CurrentIterationMat);
                        } break;

                        case AScale: {
                            real32 X1 = FrAnimTransform->Scale[_x_];
                            real32 Y1 = FrAnimTransform->Scale[_y_];
                            real32 Z1 = FrAnimTransform->Scale[_z_];

                            real32 X2 = ScAnimTransform->Scale[_x_];
                            real32 Y2 = ScAnimTransform->Scale[_y_];
                            real32 Z2 = ScAnimTransform->Scale[_z_];

                            real32 FX = ((X2 - X1) * T) + X1;
                            real32 FY = ((Y2 - Y1) * T) + Y1;
                            real32 FZ = ((Z2 - Z1) * T) + Z1;

                            Vec3 FinalScale = { FX, FY, FZ };

                            MakeScaleFromVector(&FinalScale, &CurrentIterationMat);
                        } break;

                    }
                }
                else {
                    switch (TransformIndex) {

                        case ATranslation: {
                            MakeTranslationFromVec(&FrAnimTransform->Translation, &CurrentIterationMat); 
                        } break;

                        case ARotation: {
                            FrAnimTransform->Rotation.ToMat4(&CurrentIterationMat);
                        } break;

                        case AScale: {
                            MakeScaleFromVector(&FrAnimTransform->Scale, &CurrentIterationMat);
                        } break;
                    }
                }
            }

            CurrentJointMat = CurrentIterationMat * CurrentJointMat;
        }
    }
    else {
        Assert(false);
    }

    Mat4x4 ExportMat = Parent * CurrentJointMat;
    Matrices->Matrices[Ids.OriginalBoneID] = ExportMat;

    i32 ChildrenAmount = Joint->ChildrenAmount;
    for (i32 ChildrenIndex = 0; ChildrenIndex < ChildrenAmount; ++ChildrenIndex) {
        ReadAndCalculateAnimationMatrices(Skin, Matrices, Anim, Joint->Children[ChildrenIndex], &ExportMat, CurrentTime);
    }
}

void CalculateFrameSkinningMatrices(Skinning* Skin, SkinningMatricesStorage* FrameMatrixStorage)
{
    i32                             JointsAmount    = Skin->JointsAmount;
    std::map<std::string, BoneIDs>  Bones           = Skin->Bones;

    Mat4x4 (&Matrices)[200] = FrameMatrixStorage->Matrices;

    for (i32 i = 0; i < JointsAmount; ++i) {
        JointsInfo *JointInfo = &Skin->Joints[i];

        BoneIDs& Ids = Bones[JointInfo->BoneName];

        Mat4x4& CurrentMat = Matrices[Ids.OriginalBoneID];

        CurrentMat = CurrentMat * JointInfo->InverseBindMatrix;
    }
}

void FillSkinMatrix(SkinningMatricesStorage* Matrices, SkeletalComponent* Skelet, real32& AnimationDuration, i32 AnimationToPlay, bool32 Loop)
{
    AnimationsArray*    AnimArray = Skelet->Animations;
    Skinning*           Skin = &Skelet->Skin;

    Assert(AnimArray->AnimsAmount >= AnimationToPlay);

    Animation* AnimToPlay = &AnimArray->Anims[AnimationToPlay];

    if (Loop) {
        AnimationDuration = fmodf(AnimationDuration, AnimToPlay->MaxDuration);
    }
    else {
        if (AnimationDuration > AnimToPlay->MaxDuration) {
            return;
        }
    }

    JointsInfo* RootJoint = &Skin->Joints[0];

    ReadAndCalculateAnimationMatrices(Skin, Matrices, AnimToPlay, RootJoint, 0, AnimationDuration);

    Matrices->Amount = Skelet->Skin.JointsAmount;
}

static void RenderPrepareFrame(GameContext *Cntx, FrameData *Data)
{
    SkinningMatricesStorage *Storage = &Data->SkinMatrix;

    i32 AnimationToPlay = Cntx->ArrowUpWasTriggered;

    for (i32 Index = 0; Index < DYNAMIC_SCENE_OBJECTS_MAX; ++Index) {
        DynamicSceneObject*     CurrentSceneObject  = &Cntx->TestDynamocSceneObjects[Index];
        SkeletalComponent*      Skin                = &CurrentSceneObject->ObjMesh.Skelet;

        FillSkinMatrix(Storage, Skin, Cntx->AnimationDuration, AnimationToPlay, 1);
    }
}

static void SetupObjectRendering(GameContext* Ctx, FrameData* Data, WorldTransform& ObjectTransform,
                                 ShaderProgramVariablesStorage*  VarStorage, ObjectNesting& Nesting, 
                                 ShaderProgramsType ShaderType)
{
    Mat4x4  ObjectToCameraSpaceTransformation   = {};
    Mat4x4  ObjectGeneralTransformation         = {};
    Mat4x4  RootTransformation                  = Identity4x4; 
    Mat4x4  ObjectTranslation                   = {};
    Mat4x4  ObjectRotation                      = {};
    Mat4x4  ObjectScale                         = {};
    Vec3    CameraPosition                      = Ctx->PlayerCamera.Transform.Position;
    Vec3    ObjectPosition;

    MakeTranslationFromVec(&ObjectTransform.Position, &ObjectTranslation);
    MakeObjectToUprightRotation(&ObjectTransform.Rotation, &ObjectRotation);

    if (Nesting.Parent) {
        DynamicSceneObject*             ParentObject    = Nesting.Parent;
        WorldTransform&                 ParentTransform = ParentObject->Transform;
        std::map<std::string, BoneIDs>& Bones           = ParentObject->ObjMesh.Skelet.Skin.Bones;

        Mat4x4 RootToWorldTranslation   = {};
        Mat4x4 RootToWorldRotation      = {};
        Mat4x4 RootToWorldScale         = {};

        MakeTranslationFromVec(&ParentTransform.Position, &RootToWorldTranslation);
        MakeObjectToUprightRotation(&ParentTransform.Rotation, &RootToWorldRotation);
        MakeScaleFromVector(&ParentTransform.Scale, &RootToWorldScale);

        MakeScaleFromVectorRelative(&ObjectTransform.Scale, &ParentTransform.Scale, &ObjectScale);

        BoneIDs& Ids = Bones[Nesting.AttachedToBone];
        
        Mat4x4& AttachedBoneMat = Data->SkinMatrix.Matrices[Ids.OriginalBoneID];

        ObjectToCameraSpaceTransformation   = Data->CameraTransformation * RootToWorldTranslation;
        ObjectGeneralTransformation         = RootToWorldRotation * RootToWorldScale * AttachedBoneMat * 
                                              ObjectTranslation * ObjectRotation * ObjectScale;
                                        
        ObjectPosition = ParentTransform.Position;
    }
    else {
        MakeScaleFromVector(&ObjectTransform.Scale, &ObjectScale);

        ObjectToCameraSpaceTransformation   = Data->CameraTransformation * ObjectTranslation;
        ObjectGeneralTransformation         = ObjectRotation * ObjectScale;

        ObjectPosition = ObjectTransform.Position;
    }

    tglUniformMatrix4fv(VarStorage->Transform.ObjectToCameraSpaceTransformationLocation, 1, GL_TRUE, ObjectToCameraSpaceTransformation[0]);
    tglUniformMatrix4fv(VarStorage->Transform.ObjectGeneralTransformationLocation, 1, GL_TRUE, ObjectGeneralTransformation[0]);

    Vec3 CameraPositionInObjectUprightSpace = CameraPosition - ObjectPosition;
    tglUniform3fv(VarStorage->Light.ViewerPositionLocation, 1, &CameraPositionInObjectUprightSpace[0]);

    SetupPointLights(Ctx->PointLights, MAX_POINTS_LIGHTS, ShaderType, &ObjectPosition);
    SetupSpotLights(Ctx->SpotLights, MAX_SPOT_LIGHTS, ShaderType, &ObjectPosition);
}

static void DrawSkeletalMesh(Platform *Platform, GameContext *Cntx, FrameData *Data)
{
    ShaderProgram* Shader = &ShadersProgramsCache[ShaderProgramsType::SkeletalMeshShader];
    ShaderProgramVariablesStorage* VarStorage = &Shader->ProgramVarsStorage;
    
    tglUseProgram(Shader->Program);

    SkinningMatricesStorage* MatrixStorage = &Data->SkinMatrix;

    SetupDirectionalLight(&Cntx->LightSource, ShaderProgramsType::SkeletalMeshShader);

    Cntx->AnimationDuration += Cntx->DeltaTimeSec;
    for (i32 Index = 0; Index < DYNAMIC_SCENE_OBJECTS_MAX; ++Index) {
        DynamicSceneObject*     CurrentSceneObject  = &Cntx->TestDynamocSceneObjects[Index];
        SkeletalMeshComponent*  Comp                = &CurrentSceneObject->ObjMesh;
        WorldTransform*         Transform           = &CurrentSceneObject->Transform;
        SkeletalComponent*      Skin                = &CurrentSceneObject->ObjMesh.Skelet;

        if (Platform->Input.EButton.State == KeyState::Pressed && !Cntx->EWasPressed) {
            Cntx->AnimationDuration += 0.01;
            Cntx->EWasPressed = 1;
        }
        else if (Platform->Input.EButton.State == KeyState::Released) {
            Cntx->EWasPressed = 0;
        }
        CalculateFrameSkinningMatrices(&Skin->Skin, MatrixStorage);

        ShaderProgramVariablesStorage::AnimationInfo* AnimVar = &VarStorage->Animation;
        for (i32 MatrixIndex = 0; MatrixIndex < MatrixStorage->Amount; ++MatrixIndex) {
            Mat4x4* Mat = &MatrixStorage->Matrices[MatrixIndex];

            tglUniformMatrix4fv(AnimVar->AnimationMatricesLocation[MatrixIndex], 1, GL_TRUE, (*Mat)[0]);
        }

        SetupObjectRendering(Cntx, Data, *Transform, VarStorage, CurrentSceneObject->Nesting, ShaderProgramsType::SkeletalMeshShader);

        // tglUniform1i(VarStorage->Animation.BoneIDLocation, Cntx->BoneID); TODO(ismail): add in future for debugging

        i32             PrimitivesAmount    = Comp->PrimitivesAmount;
        MeshPrimitives* Primitives          = Comp->Primitives;
        for (i32 PrimitiveIndex = 0; PrimitiveIndex < PrimitivesAmount; ++PrimitiveIndex) {
            MeshPrimitives* Primitive   = &Primitives[PrimitiveIndex];
            MeshMaterial*   Material    =  &Primitive->Material;

            tglUniform3fv(VarStorage->MaterialInfo.MaterialAmbientColorLocation, 1, &Material->AmbientColor[0]);
            tglUniform3fv(VarStorage->MaterialInfo.MaterialDiffuseColorLocation, 1, &Material->DiffuseColor[0]);
            tglUniform3fv(VarStorage->MaterialInfo.MaterialSpecularColorLocation, 1, &Material->SpecularColor[0]);

            tglBindVertexArray(Primitive->BuffersHandler[OpenGLBuffersLocation::GLVertexArrayLocation]);

            if (Material->HaveTexture) {
                tglActiveTexture(VarStorage->MaterialInfo.DiffuseTexture.Unit);
                glBindTexture(GL_TEXTURE_2D, Material->TextureHandle);
            }
            else {
                tglActiveTexture(VarStorage->MaterialInfo.DiffuseTexture.Unit);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            if (Material->HaveSpecularExponent) {
                tglActiveTexture(VarStorage->MaterialInfo.SpecularExpMap.Unit);
                glBindTexture(GL_TEXTURE_2D, Material->SpecularExponentMapTextureHandle);
            }
            else {
                tglActiveTexture(VarStorage->MaterialInfo.SpecularExpMap.Unit);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            tglDrawElements(GL_TRIANGLES, Primitive->InidicesAmount, GL_UNSIGNED_INT, 0);
        }
    }

}

static void DrawStaticMesh(Platform *Platform, GameContext *Cntx, FrameData *Data)
{
    ShaderProgram*                  Shader          = &ShadersProgramsCache[ShaderProgramsType::MeshShader];
    ShaderProgramVariablesStorage*  VarStorage      = &Shader->ProgramVarsStorage;

    tglUseProgram(Shader->Program);    

    SetupDirectionalLight(&Cntx->LightSource, ShaderProgramsType::MeshShader);

    for (i32 Index = 0; Index < SCENE_OBJECTS_MAX; ++Index) {
        SceneObject*    CurrentSceneObject  = &Cntx->TestSceneObjects[Index];
        MeshComponent*  Comp                = &CurrentSceneObject->ObjMesh;
        WorldTransform* Transform           = &CurrentSceneObject->Transform;

        Transform->Rotation.Heading += Cntx->RotationDelta;

        SetupObjectRendering(Cntx, Data, *Transform, VarStorage, CurrentSceneObject->Nesting, ShaderProgramsType::MeshShader);

        tglBindVertexArray(Comp->BuffersHandler[OpenGLBuffersLocation::GLVertexArrayLocation]);
        for (i32 Index = 0; Index < Comp->MeshesAmount; ++Index) {
            MeshComponentObjects*   MeshInfo        = &Comp->MeshesInfo[Index];
            MeshMaterial*           MeshMaterial    = &MeshInfo->Material;

            tglUniform3fv(VarStorage->MaterialInfo.MaterialDiffuseColorLocation, 1,   &MeshMaterial->DiffuseColor[0]);
            tglUniform3fv(VarStorage->MaterialInfo.MaterialAmbientColorLocation, 1,   &MeshMaterial->AmbientColor[0]);
            tglUniform3fv(VarStorage->MaterialInfo.MaterialSpecularColorLocation, 1,  &MeshMaterial->SpecularColor[0]);
            
            if (MeshMaterial->HaveTexture) {
                tglActiveTexture(VarStorage->MaterialInfo.DiffuseTexture.Unit);
                glBindTexture(GL_TEXTURE_2D, MeshMaterial->TextureHandle);
            }
            else {
                tglActiveTexture(VarStorage->MaterialInfo.DiffuseTexture.Unit);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            if (MeshMaterial->HaveSpecularExponent) {
                tglActiveTexture(VarStorage->MaterialInfo.SpecularExpMap.Unit);
                glBindTexture(GL_TEXTURE_2D, MeshMaterial->SpecularExponentMapTextureHandle);
            }
            else {
                tglActiveTexture(VarStorage->MaterialInfo.SpecularExpMap.Unit);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        
            tglDrawElementsBaseVertex(GL_TRIANGLES, MeshInfo->NumIndices, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * MeshInfo->IndexOffset), MeshInfo->VertexOffset);
        }
    }
}

void Frame(Platform *Platform, GameContext *Cntx)
{
    FrameData Frame = {};
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (Platform->Input.QButton.State == KeyState::Pressed && !Cntx->QWasTriggered) {
        Cntx->QWasTriggered = 1;

        if (!Cntx->PolygonModeActive) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            Cntx->PolygonModeActive = 1;
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            Cntx->PolygonModeActive = 0;
        }
        
    }
    else if (Platform->Input.QButton.State == KeyState::Released) {
        Cntx->QWasTriggered = 0;
    }

    if (Platform->Input.ArrowUp.State == KeyState::Pressed && !Cntx->ArrowUpWasTriggered) {
        Cntx->ArrowUpWasTriggered = 1;

        if (Cntx->BoneID < 5) {
            ++Cntx->BoneID;
        }
        else {
            Cntx->BoneID = 0;
        }
    }
    else if (Platform->Input.ArrowUp.State == KeyState::Released) {
        Cntx->ArrowUpWasTriggered = 0;
    }

    if (Platform->Input.MButton.State == KeyState::Pressed && !Cntx->MWasTriggered) {
        Cntx->MWasTriggered = 1;
        Platform->CursorSwitched = 1;

        Platform->CursorState = (MouseCursorState)!Platform->CursorState;
    }
    else if (Platform->Input.MButton.State == KeyState::Released) {
        Cntx->MWasTriggered = 0;
    }

    if (Cntx->EditorModeOn) {
        
    }

    RenderPrepareFrame(Cntx, &Frame);

    Mat4x4 PerspProjection = MakePerspProjection(60.0f, Platform->ScreenOpt.AspectRatio, 0.1f, 1500.0f);

    Cntx->PlayerCamera.Transform.Rotation.Bank      = 0.0f;
    Cntx->PlayerCamera.Transform.Rotation.Pitch     += RAD_TO_DEGREE(Platform->Input.MouseInput.Moution.y) * 0.5f;
    Cntx->PlayerCamera.Transform.Rotation.Heading   += RAD_TO_DEGREE(Platform->Input.MouseInput.Moution.x) * 0.5f;

    real32 ZTranslationMultiplyer = (real32)(Platform->Input.WButton.State + (-1 * Platform->Input.SButton.State)); // 1.0 if W Button -1.0 if S Button and 0 if W and S Button pressed together
    real32 XTranslationMultiplyer = (real32)(Platform->Input.DButton.State + (-1 * Platform->Input.AButton.State));

    Vec3 Target = {};
    Vec3 Right = {};
    Vec3 Up = {};

    RotationToDirectionVecotrs(&Cntx->PlayerCamera.Transform.Rotation, &Target, &Right, &Up);

    Cntx->PlayerCamera.Transform.Position += (Target * ZTranslationMultiplyer * 0.1f) + (Right * XTranslationMultiplyer * 0.1f);

    Mat4x4 CameraTranslation = MakeInverseTranslation(&Cntx->PlayerCamera.Transform.Position);

    Mat4x4 CameraUprightToObjectRotation = {};
    MakeUprightToObjectRotation(&CameraUprightToObjectRotation, &Cntx->PlayerCamera.Transform.Rotation);

    Mat4x4 CameraTransformation = PerspProjection * CameraUprightToObjectRotation * CameraTranslation;

    Frame.CameraTransformation = CameraTransformation;

    if(1) {
        DirectionalLight *SceneDirLight = &Cntx->LightSource;

        SpotLight* SceneSpotLight               = &Cntx->SpotLights[0];
        SceneSpotLight->Attenuation.Position    = Cntx->PlayerCamera.Transform.Position;
        SceneSpotLight->Direction               = Target;

        ShaderProgram *Shader = &ShadersProgramsCache[ShaderProgramsType::MeshShader];
        tglUseProgram(Shader->Program);

        ShaderProgramVariablesStorage *VarStorage = &Shader->ProgramVarsStorage;

        // TERRAIN RENDERING
        Terrain *Terrain = &Cntx->Terrain;

        Vec3 TerrainAmbientColor    = { 0.6f, 0.6f, 0.6f };
        Vec3 TerrainDiffuseColor    = { 0.8f, 0.8f, 0.8f };
        Vec3 TerrainSpecularColor   = { 0.1f, 0.1f, 0.1f };

        tglUniform3fv(VarStorage->MaterialInfo.MaterialAmbientColorLocation, 1, &TerrainAmbientColor[0]);
        tglUniform3fv(VarStorage->MaterialInfo.MaterialDiffuseColorLocation, 1, &TerrainDiffuseColor[0]);
        tglUniform3fv(VarStorage->MaterialInfo.MaterialSpecularColorLocation, 1, &TerrainSpecularColor[0]);

        tglActiveTexture(Shader->ProgramVarsStorage.MaterialInfo.DiffuseTexture.Unit);
        glBindTexture(GL_TEXTURE_2D, Terrain->TextureHandle);

        tglActiveTexture(Shader->ProgramVarsStorage.MaterialInfo.SpecularExpMap.Unit);
        glBindTexture(GL_TEXTURE_2D, Terrain->TextureHandle);

        SetupDirectionalLight(SceneDirLight, ShaderProgramsType::MeshShader);

        Mat4x4 TerrainWorldTranslation = {};
        MakeTranslationFromVec(&Terrain->Transform.Position, &TerrainWorldTranslation);

        Mat4x4 TerrainWorldRotation = {};
        MakeObjectToUprightRotation(&Terrain->Transform.Rotation, &TerrainWorldRotation);

        Mat4x4 TerrainWorldScale = {};
        MakeScaleFromVector(&Terrain->Transform.Scale, &TerrainWorldScale);

        Mat4x4 TerrainWorldTransformation = CameraTransformation * TerrainWorldTranslation;
        Mat4x4 TerrainWorldScaleAndRotate = TerrainWorldRotation * TerrainWorldScale;

        tglUniformMatrix4fv(VarStorage->Transform.ObjectToCameraSpaceTransformationLocation, 1, GL_TRUE, TerrainWorldTransformation[0]);
        tglUniformMatrix4fv(VarStorage->Transform.ObjectGeneralTransformationLocation, 1, GL_TRUE, TerrainWorldScaleAndRotate[0]);

        Vec3 ViewerPositionInTerrainUpright = Cntx->PlayerCamera.Transform.Position - Terrain->Transform.Position;
        tglUniform3fv(VarStorage->Light.ViewerPositionLocation, 1, &ViewerPositionInTerrainUpright[0]);

        tglBindVertexArray(Terrain->BuffersHandler[OpenGLBuffersLocation::GLVertexArrayLocation]);

        SetupPointLights(Cntx->PointLights, MAX_POINTS_LIGHTS, ShaderProgramsType::MeshShader, &Terrain->Transform.Position);
        SetupSpotLights(Cntx->SpotLights, MAX_SPOT_LIGHTS, ShaderProgramsType::MeshShader, &Terrain->Transform.Position);

        tglDrawElements(GL_TRIANGLES, Terrain->IndicesAmount, GL_UNSIGNED_INT, 0);
        // TERRAIN RENDERING END

        if (Cntx->PointLights[0].Attenuation.DisctanceMin >= 33.0f) {
            Cntx->TranslationDelta *= -1.0f;
        }
        else if (Cntx->PointLights[0].Attenuation.DisctanceMin <= 0.0f) {
            Cntx->TranslationDelta *= -1.0f;
        }

        for (i32 Index = 0; Index < MAX_POINTS_LIGHTS; ++Index) {
            PointLight* CurrentPointLight = &Cntx->PointLights[Index];

            CurrentPointLight->Attenuation.DisctanceMin += Cntx->TranslationDelta;
        }

        // MESHES RENDERING

        DrawStaticMesh(Platform, Cntx, &Frame);

        // MESHES RENDERING END

        // SKELETAL MESHES RENDERING

        DrawSkeletalMesh(Platform, Cntx, &Frame);

        // SKELETAL MESHES RENDERING END

        // PARTICLE RENDERER

        Shader = &ShadersProgramsCache[ShaderProgramsType::ParticlesShader];
        tglUseProgram(Shader->Program);

        VarStorage = &Shader->ProgramVarsStorage;

        Particle*       SceneParticles  = Cntx->SceneParticles;
        ParticleSystem* ParticleSys     = &Cntx->ParticleSystem;
        for (i32 Index = 0; Index < PARTICLES_MAX; ++Index) {
            Particle*       CurrentParticle = &SceneParticles[Index];
            WorldTransform* Transform       = &CurrentParticle->Transform;

            CurrentParticle->Integrate(Cntx->DeltaTimeSec);

            Mat4x4 ObjectToWorldTranslation = {};
            MakeTranslationFromVec(&Transform->Position, &ObjectToWorldTranslation);

            Mat4x4 ObjectToWorlRotation = {};
            MakeObjectToUprightRotation(&Transform->Rotation, &ObjectToWorlRotation);

            Mat4x4 ObjectToWorldScale = {};
            MakeScaleFromVector(&Transform->Scale, &ObjectToWorldScale);

            Mat4x4 ObjectToWorldTransformation = CameraTransformation * ObjectToWorldTranslation;
            Mat4x4 ObjectToWorldScaleAndRotate = ObjectToWorlRotation * ObjectToWorldScale;

            tglUniformMatrix4fv(VarStorage->Transform.ObjectToCameraSpaceTransformationLocation, 1, GL_TRUE, ObjectToWorldTransformation[0]);
            tglUniformMatrix4fv(VarStorage->Transform.ObjectGeneralTransformationLocation, 1, GL_TRUE, ObjectToWorldScaleAndRotate[0]);

            tglBindVertexArray(ParticleSys->BuffersHandler[OpenGLBuffersLocation::GLVertexArrayLocation]);

            tglDrawElements(GL_TRIANGLES, ParticleSys->IndicesAmount, GL_UNSIGNED_INT, 0);
        }

        // PARTICLE RENDERER END

        // DEBUG DRAW RENDERER



        // DEBUG DRAW RENDERER END
    }
    else {
        /*
        ShaderProgram*                  Shader          = &ShadersProgramsCache[ShaderProgramsType::MeshShader];
        ShaderProgramVariablesStorage*  VarStorage      = &Shader->ProgramVarsStorage;
        Mat4x4&                         CameraTransform = Frame.CameraTransformation;

        tglUseProgram(Shader->Program);    

        SetupDirectionalLight(&Cntx->LightSource, ShaderProgramsType::MeshShader);

        DynamicSceneObject* Player = &Cntx->TestDynamocSceneObjects[0];

        std::map<std::string, BoneIDs>& Bones = Player->ObjMesh.Skelet.Skin.Bones;

        BoneIDs IDs = Bones["mixamorig5:LeftHand"];
        Mat4x4& AttachMat = Frame.SkinMatrix.Matrices[IDs.OriginalBoneID];

        for (i32 Index = 0; Index < 1; ++Index) {
            SceneObject*    CurrentSceneObject  = &Cntx->TestSceneObjects[1];
            MeshComponent*  Comp                = &CurrentSceneObject->ObjMesh;
            WorldTransform* RootTransform       = &Player->Transform;
            WorldTransform* ObjectTransform     = &CurrentSceneObject->Transform;

            ObjectTransform->Position = {};

            // Transform->Rotation.Heading += Cntx->RotationDelta;

            Mat4x4 ObjectToWorldTranslation = {};
            MakeTranslationFromVec(&ObjectTransform->Position, &ObjectToWorldTranslation);

            Mat4x4 ObjectToWorlRotation = {};
            MakeObjectToUprightRotation(&ObjectTransform->Rotation, &ObjectToWorlRotation);

            Mat4x4 ObjectToWorldScale = {};
            MakeScaleFromVectorRelative(&ObjectTransform->Scale, &RootTransform->Scale, &ObjectToWorldScale);

            Mat4x4 RootToWorldTranslation = {};
            MakeTranslationFromVec(&RootTransform->Position, &RootToWorldTranslation);

            Mat4x4 RootToWorldRotation = {};
            MakeObjectToUprightRotation(&RootTransform->Rotation, &RootToWorldRotation);

            Mat4x4 RootToWorldScale = {};
            MakeScaleFromVector(&RootTransform->Scale, &RootToWorldScale);

            Mat4x4 RootTransformation = RootToWorldTranslation * RootToWorldRotation * RootToWorldScale * AttachMat;

            Mat4x4 ObjectToWorldTransformation = CameraTransform * RootTransformation * ObjectToWorldTranslation;
            Mat4x4 ObjectToWorldScaleAndRotate = ObjectToWorlRotation * ObjectToWorldScale;

            tglUniformMatrix4fv(VarStorage->Transform.ObjectToWorldTransformationLocation, 1, GL_TRUE, ObjectToWorldTransformation[0]);
            tglUniformMatrix4fv(VarStorage->Transform.ObjectToWorldScaleAndRotateLocation, 1, GL_TRUE, ObjectToWorldScaleAndRotate[0]);

            Vec3 CameraPositionInObjectUprightSpace = Cntx->PlayerCamera.Transform.Position - RootTransform->Position;

            tglUniform3fv(VarStorage->Light.ViewerPositionLocation, 1, &CameraPositionInObjectUprightSpace[0]);

            SetupPointLights(Cntx->PointLights, MAX_POINTS_LIGHTS, ShaderProgramsType::MeshShader, &RootTransform->Position);
            SetupSpotLights(Cntx->SpotLights, MAX_SPOT_LIGHTS, ShaderProgramsType::MeshShader, &RootTransform->Position);

            tglBindVertexArray(Comp->BuffersHandler[OpenGLBuffersLocation::GLVertexArrayLocation]);
            for (i32 Index = 0; Index < Comp->MeshesAmount; ++Index) {
                MeshComponentObjects*   MeshInfo        = &Comp->MeshesInfo[Index];
                MeshMaterial*           MeshMaterial    = &MeshInfo->Material;

                tglUniform3fv(VarStorage->MaterialInfo.MaterialDiffuseColorLocation, 1,   &MeshMaterial->DiffuseColor[0]);
                tglUniform3fv(VarStorage->MaterialInfo.MaterialAmbientColorLocation, 1,   &MeshMaterial->AmbientColor[0]);
                tglUniform3fv(VarStorage->MaterialInfo.MaterialSpecularColorLocation, 1,  &MeshMaterial->SpecularColor[0]);

                if (MeshMaterial->HaveTexture) {
                    tglActiveTexture(VarStorage->MaterialInfo.DiffuseTexture.Unit);
                    glBindTexture(GL_TEXTURE_2D, MeshMaterial->TextureHandle);
                }
                else {
                    tglActiveTexture(VarStorage->MaterialInfo.DiffuseTexture.Unit);
                    glBindTexture(GL_TEXTURE_2D, 0);
                }

                if (MeshMaterial->HaveSpecularExponent) {
                    tglActiveTexture(VarStorage->MaterialInfo.SpecularExpMap.Unit);
                    glBindTexture(GL_TEXTURE_2D, MeshMaterial->SpecularExponentMapTextureHandle);
                }
                else {
                    tglActiveTexture(VarStorage->MaterialInfo.SpecularExpMap.Unit);
                    glBindTexture(GL_TEXTURE_2D, 0);
                }
            
                tglDrawElementsBaseVertex(GL_TRIANGLES, MeshInfo->NumIndices, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * MeshInfo->IndexOffset), MeshInfo->VertexOffset);
            }
        }*/

        DrawStaticMesh(Platform, Cntx, &Frame);
        DrawSkeletalMesh(Platform, Cntx, &Frame);
    }
}