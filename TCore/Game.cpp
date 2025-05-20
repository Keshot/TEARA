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

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

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

struct Quat {
    union {
        struct {
            real32 w, x, y, z;
        };
        struct {
            real32 w;
            Vec3 n;
        };
        real32 ValueHolder[4];   
    };

    Quat(real32 w, real32 x, real32 y, real32 z)
        : w(w)
        , x(x)
        , y(y)
        , z(z)
    {
    }

    Quat(real32 angle, const Vec3& n) {
        real32 alpha = angle / 2.0f;

        w = cosf(alpha); 
        this->n = n;
        this->n *= sinf(alpha);
    }

    Quat operator-() const {
        Quat Result(-w, -x, -y, -z);
        return Result;
    }

    real32 Length() const {
        return sqrtf(SQUARE(w) + SQUARE(x) + SQUARE(y) + SQUARE(z));
    }

    Quat operator*(const Quat &b) const {
        Quat Result(
            w * b.w - x * b.x - y * b.y - z * b.z,
            w * b.x + x * b.w + y * b.z - z * b.y,
            w * b.y + y * b.w + z * b.x - x * b.z,
            w * b.z + z * b.w + x * b.y - y * b.x
        );
        return Result;
    }

    Vec3 operator*(const Vec3 &b) const 
    {
        // x = b.x(w^2 + x^2 - z^2 - y^2) + b.z(2*y*w + 2*z*x) + b.y(2*y*x - 2*z*w);
        // y = b.y(y^2 + w^2 - z^2 - x^2) + b.x(2*x*y + 2*w*z) + b.z(2*z*y - 2*x*w);
        // z = b.z(z^2 + w^2 - x^2 - y^2) + b.y(2*y*z + 2*w*x) + b.x(2*x*z - 2*w*y);

        real32 ww = SQUARE(w);
        real32 xx = SQUARE(x);
        real32 yy = SQUARE(y);
        real32 zz = SQUARE(z);

        real32 wwyy = ww - yy;
        real32 xxzz = xx - zz;

        real32 yw2 = y * w * 2.0f;
        real32 zx2 = z * x * 2.0f;
        real32 yx2 = y * x * 2.0f;
        real32 zw2 = z * w * 2.0f;
        real32 zy2 = z * y * 2.0f;
        real32 xw2 = x * w * 2.0f;

        Vec3 Result = {
            b.x * (wwyy + xxzz)         + b.z * (yw2 + zx2)     + b.y * (yx2 - zw2),
            b.y * (yy + ww - zz - xx)   + b.x * (yx2 + zw2)     + b.z * (zy2 - xw2),
            b.z * (wwyy - xxzz)         + b.y * (zy2 + xw2)     + b.x * (zx2 - yw2)
        };

        return Result;
    }

    void ToMat3(Mat3x3 *Result) const
    {
        real32 x2 = x * 2.0f;
        real32 y2 = y * 2.0f;
        real32 z2 = z * 2.0f;

        real32 xx2 = x2 * x;
        real32 yy2 = y2 * y;
        real32 zz2 = z2 * z;

        real32 wx2  = x2 * w;
        real32 xy2  = x2 * y;
        real32 xx   = x2 * x;

        real32 wz2  = z2 * w;
        real32 xz2  = z2 * x;
        real32 zz   = z2 * z;

        real32 wy2  = y2 * w;
        real32 yz2  = y2 * z;
        real32 yy   = y2 * y;

        *Result = {
            1.0f - yy2 - zz2,         xy2 - wz2,         xz2 + wy2,
                   xy2 + wz2,  1.0f - xx2 - zz2,         yz2 - wx2,
                   xz2 - wy2,         yz2 + wx2,  1.0f - xx2 - yy2,
        };
    }

    inline real32 Dot(const Quat &b) const {
        real32 Result = w * b.w + x * b.x + y * b.y + z * b.z;
        return Result;
    }

    static Quat Slerp(const Quat &Src, const Quat &Dst, real32 Delta) {
        if (Delta > 0.9999f) {
            return Dst;
        }
        else if (Delta < 0.0001f) {
            return Src;
        }

        real32 k0, k1;
        Quat SrcTemp = Src;
        real32 CosOmega = SrcTemp.Dot(Dst);

        if (CosOmega < 0.0f) {
            SrcTemp.w   = -Src.w;
            SrcTemp.x   = -Src.x;
            SrcTemp.y   = -Src.y;
            SrcTemp.z   = -Src.z;
            CosOmega    = -CosOmega;
        }

        if (CosOmega > 0.9999f) {
            k0 = 1.0f - Delta;
            k1 = Delta;
        }
        else {
            real32 SinOmega = sqrtf(1.0f - SQUARE(CosOmega));

            real32 Omega = atan2f(SinOmega, CosOmega);
            real32 OneOverSinOmega = 1.0f / SinOmega;

            k0 = sinf((1.0f - Delta) * Omega) * OneOverSinOmega;
            k1 = sinf(Delta * Omega) * OneOverSinOmega;
        }

        Quat Result(
            SrcTemp.w * k0 + Dst.w * k1,
            SrcTemp.x * k0 + Dst.x * k1,
            SrcTemp.y * k0 + Dst.y * k1,
            SrcTemp.z * k0 + Dst.z * k1
        );

        return Result;
    }
};

// From a to b with factor of t
Vec3 Lerp(const Vec3 &a, const Vec3 &b, real32 t)
{
    Vec3 To = b - a;
    return To * t;
}

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
        "data/shaders/mesh_component_shader.vs", 
        "data/shaders/mesh_component_shader.fs" 
    },
    { 
        "../skeletal_mesh_component_shader.vs", 
        "../skeletal_mesh_component_shader.fs" 
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

        ShaderVariablesStorage->Transform.ObjectToWorldTransformationLocation   = tglGetUniformLocation(ShaderProgram, "ObjectToWorldTransformation");

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

struct MeshLoaderNode {
    const char*         ObjName;
    ObjFileLoaderFlags  Flags;
};

const MeshLoaderNode SceneObjectsName[] = {
    {
        "data/obj/Golem.obj",
        {
            /* GenerateSmoothNormals */  1,
            /* SelfGenerateNormals   */  0
        }
    },
    {
        "data/obj/antique_ceramic_vase_01_4k.obj",
        {
            /* GenerateSmoothNormals */  0,
            /* SelfGenerateNormals   */  0
        }
    }
};

const char *DynamicSceneObjectsName[] = {
    "data/obj/SimpleTest2.gltf",
};

void SetupPointLights(PointLight* Lights, u32 LightAmount, ShaderProgramsType ShaderType, Mat3x3* UprightToObjectSpace, Vec3* ObjectPosition)
{
    ShaderProgramVariablesStorage *VarStorage = &ShadersProgramsCache[ShaderType].ProgramVarsStorage;

    for (u32 Index = 0; Index < LightAmount; ++Index) {
        PointLight* CurrentPointLight = &Lights[Index];
        ShaderProgramVariablesStorage::LightWork::PointLightLocations* PointLightsVarLocations = &VarStorage->Light.PointLightsLocations[Index];

        Vec3 LightInTerrainUprightPosition  = CurrentPointLight->Attenuation.Position - *ObjectPosition;
        Vec3 LightInTerrainObjectPosition   = *UprightToObjectSpace * LightInTerrainUprightPosition;

        tglUniform3fv(PointLightsVarLocations->AttenuationLocation.PositionLocation, 1, &LightInTerrainObjectPosition[0]);
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

void SetupSpotLights(SpotLight* Lights, u32 LightAmount, ShaderProgramsType ShaderType, Mat3x3* UprightToObjectSpace, Vec3* ObjectPosition)
{
    ShaderProgramVariablesStorage *VarStorage = &ShadersProgramsCache[ShaderType].ProgramVarsStorage;

    for (u32 Index = 0; Index < LightAmount; ++Index) {
        SpotLight* CurrentSpotLight = &Lights[Index];
        ShaderProgramVariablesStorage::LightWork::SpotLightLocations* SpotLightsVarLocations = &VarStorage->Light.SpotLightsLocations[Index];

        Vec3 LightInTerrainUprightPosition  = CurrentSpotLight->Attenuation.Position - *ObjectPosition;
        Vec3 LightInTerrainObjectPosition   = *UprightToObjectSpace * LightInTerrainUprightPosition;

        tglUniform3fv(SpotLightsVarLocations->AttenuationLocation.PositionLocation, 1, &LightInTerrainObjectPosition[0]);
        tglUniform1f(SpotLightsVarLocations->AttenuationLocation.DisctanceMaxLocation, CurrentSpotLight->Attenuation.DisctanceMax);
        tglUniform1f(SpotLightsVarLocations->AttenuationLocation.DisctanceMinLocation, CurrentSpotLight->Attenuation.DisctanceMin);
        tglUniform1f(SpotLightsVarLocations->AttenuationLocation.AttenuationFactorLocation, CurrentSpotLight->Attenuation.AttenuationFactor);

        tglUniform3fv(SpotLightsVarLocations->SpecLocation.ColorLocation, 1, &CurrentSpotLight->Specification.Color[0]);
        tglUniform1f(SpotLightsVarLocations->SpecLocation.IntensityLocation, CurrentSpotLight->Specification.Intensity);
        tglUniform1f(SpotLightsVarLocations->SpecLocation.AmbientIntensityLocation, CurrentSpotLight->Specification.AmbientIntensity);
        tglUniform1f(SpotLightsVarLocations->SpecLocation.SpecularIntensityLocation, CurrentSpotLight->Specification.SpecularIntensity);

        Vec3 SpotLightDirectionInObjectSpace = *UprightToObjectSpace * Lights->Direction;
        tglUniform3fv(SpotLightsVarLocations->DirectionLocation, 1, &SpotLightDirectionInObjectSpace[0]);
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

struct glTF2File {
    Vec3*           Positions;
    Vec3*           Normals;
    Vec2*           TextureCoord;
    Vec4*           BoneWeights;
    iVec4*          BoneIds;
    u32*            Indices;
    u32             MeshesCount;
    u32             PositionsCount;
    u32             NormalsCount;
    u32             TexturesCount;
    u32             BoneWeightsCount;
    u32             BoneIdsCount;
    u32             IndicesCount;
};

struct glTF2LoaderSize {
    u32 PositionsSize;
    u32 NormalsSize;
    u32 TextureCoordSize;
    u32 BoneWeightsSize;
    u32 BoneIdsSize;
    u32 IndicesSize;
};

void glTFRead(const char *Path, glTF2File *FileOut, glTF2LoaderSize *LoaderSize)
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

    cgltf_result CallResult = cgltf_parse_file(&LoadOptions, Path, &Mesh);

    if (CallResult != cgltf_result::cgltf_result_success) {
        Assert(false);
    }

    CallResult = cgltf_load_buffers(&LoadOptions, Mesh, Path);

    if (CallResult != cgltf_result::cgltf_result_success) {
        Assert(false);
    }

    u32*    Indices         = FileOut->Indices;
    Vec3*   Positions       = FileOut->Positions;
    Vec3*   Normals         = FileOut->Normals;
    Vec2*   TextureCoords   = FileOut->TextureCoord;
    Vec4*   BoneWeights     = FileOut->BoneWeights;
    iVec4*  BoneIds         = FileOut->BoneIds;

    u32 IndicesSize         = LoaderSize->IndicesSize;
    u32 PositionsSize       = LoaderSize->PositionsSize;
    u32 NormalsSize         = LoaderSize->NormalsSize;
    u32 TextureCoordsSize   = LoaderSize->TextureCoordSize;
    u32 BoneWeightsSize     = LoaderSize->BoneWeightsSize;
    u32 BoneIdsSize         = LoaderSize->BoneIdsSize;

    i32 MeshesCount = (i32)Mesh->meshes_count;

    Assert(MeshesCount == 1);

    for (i32 MeshIndex = 0; MeshIndex < MeshesCount; ++MeshIndex) {
        cgltf_mesh *CurrentMesh = &Mesh->meshes[MeshIndex];

        if (CurrentMesh->primitives_count > 1) {
            Assert(false); // NOTE(Ismail): for now primitives more that 1 not supported
        }

        cgltf_primitive *CurrentMeshPrimitive = &CurrentMesh->primitives[0];

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
        
        PositionsRead       = (u32)cgltf_accessor_unpack_floats(AccessorPositions,     (real32*)Positions,     PositionsSize);
        NormalsRead         = (u32)cgltf_accessor_unpack_floats(AccessorNormals,       (real32*)Normals,       NormalsSize);
        TexturesCoordRead   = (u32)cgltf_accessor_unpack_floats(AccessorTexturesCoord, (real32*)TextureCoords, TextureCoordsSize);
        BoneWeightsRead     = (u32)cgltf_accessor_unpack_floats(AccessorWeights,       (real32*)BoneWeights,   BoneWeightsSize);

        BoneIdsRead = (u32)cgltf_accessor_unpack_indices_32bit_package(AccessorJoints, BoneIds, BoneIdsSize);

        IndicesRead = (u32)cgltf_accessor_unpack_indices(CurrentMeshPrimitive->indices, Indices, sizeof(*Indices), IndicesSize);
    }

    FileOut->MeshesCount = MeshesCount;

    FileOut->PositionsCount     = AccessorPositions->count;
    FileOut->NormalsCount       = AccessorNormals->count;
    FileOut->TexturesCount      = AccessorTexturesCoord->count;
    FileOut->BoneWeightsCount   = AccessorWeights->count;
    FileOut->BoneIdsCount       = AccessorJoints->count;
    FileOut->IndicesCount       = IndicesRead;
}

void InitSkeletalMeshComponent(Platform *Platform, MeshComponent *SkeletalMesh)
{
    glTF2File       LoadFile            = {};
    glTF2LoaderSize LoaderBufferSize    = {};
    u32*            Buffers             = SkeletalMesh->BuffersHandler;

    LoaderBufferSize.PositionsSize =    80000;
    LoaderBufferSize.NormalsSize =      80000;
    LoaderBufferSize.TextureCoordSize = 80000;
    LoaderBufferSize.BoneIdsSize =      80000;
    LoaderBufferSize.BoneWeightsSize =  80000;
    LoaderBufferSize.IndicesSize =      80000 * 2;

    LoadFile.Positions      = (Vec3*)Platform->AllocMem(sizeof(*LoadFile.Positions)     * LoaderBufferSize.PositionsSize);
    LoadFile.Normals        = (Vec3*)Platform->AllocMem(sizeof(*LoadFile.Normals)       * LoaderBufferSize.NormalsSize);
    LoadFile.TextureCoord   = (Vec2*)Platform->AllocMem(sizeof(*LoadFile.TextureCoord)  * LoaderBufferSize.TextureCoordSize);
    LoadFile.BoneIds        = (iVec4*)Platform->AllocMem(sizeof(*LoadFile.BoneIds)      * LoaderBufferSize.BoneIdsSize);
    LoadFile.BoneWeights    = (Vec4*)Platform->AllocMem(sizeof(*LoadFile.BoneWeights)   * LoaderBufferSize.BoneWeightsSize);
    LoadFile.Indices        = (u32*)Platform->AllocMem(sizeof(*LoadFile.Indices)        * LoaderBufferSize.IndicesSize);

    glTFRead(SkeletalMesh->ObjectPath, &LoadFile, &LoaderBufferSize);

    tglGenVertexArrays(1, &SkeletalMesh->BuffersHandler[OpenGLBuffersLocation::GLVertexArrayLocation]); 
    tglBindVertexArray(Buffers[OpenGLBuffersLocation::GLVertexArrayLocation]);

    tglGenBuffers(OpenGLBuffersLocation::GLLocationMax - 1, Buffers);

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

    tglBindBuffer(GL_ARRAY_BUFFER, Buffers[OpenGLBuffersLocation::GLBoneIndicesLocation]);
    tglBufferData(GL_ARRAY_BUFFER, sizeof(*LoadFile.BoneIds) * LoadFile.BoneIdsCount, LoadFile.BoneIds, GL_STATIC_DRAW);
    tglVertexAttribIPointer(OpenGLBuffersLocation::GLBoneIndicesLocation, 4, GL_INT, 0, (void*)0);
    tglEnableVertexAttribArray(OpenGLBuffersLocation::GLBoneIndicesLocation);

    tglBindBuffer(GL_ARRAY_BUFFER, Buffers[OpenGLBuffersLocation::GLBoneWeightsLocation]);
    tglBufferData(GL_ARRAY_BUFFER, sizeof(*LoadFile.BoneWeights) * LoadFile.BoneWeightsCount, LoadFile.BoneWeights, GL_STATIC_DRAW);
    tglVertexAttribPointer(OpenGLBuffersLocation::GLBoneWeightsLocation, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    tglEnableVertexAttribArray(OpenGLBuffersLocation::GLBoneWeightsLocation);

    tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[OpenGLBuffersLocation::GLIndexArrayLocation]);
    tglBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*LoadFile.Indices) * LoadFile.IndicesCount, LoadFile.Indices, GL_STATIC_DRAW);

    tglBindVertexArray(0);
    tglBindBuffer(GL_ARRAY_BUFFER, 0);
    tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    SkeletalMesh->MeshesInfo = (MeshComponentObjects*)Platform->AllocMem(sizeof(*SkeletalMesh->MeshesInfo) * LoadFile.MeshesCount);

    for (i32 MeshIndex = 0; MeshIndex < LoadFile.MeshesCount; ++MeshIndex) {
        MeshComponentObjects* CurrentMeshComponentObject = &SkeletalMesh->MeshesInfo[MeshIndex];
        
        CurrentMeshComponentObject->Material.AmbientColor   = {};
        CurrentMeshComponentObject->Material.DiffuseColor   = {};
        CurrentMeshComponentObject->Material.SpecularColor  = {};

        CurrentMeshComponentObject->Material.HaveSpecularExponent   = 0;
        CurrentMeshComponentObject->Material.HaveTexture            = 0;

        CurrentMeshComponentObject->Material.SpecularExponentMapTextureHandle   = 0;
        CurrentMeshComponentObject->Material.TextureHandle                      = 0;

        CurrentMeshComponentObject->NumIndices      = LoadFile.IndicesCount;
        CurrentMeshComponentObject->IndexOffset     = 0;
        CurrentMeshComponentObject->VertexOffset    = 0;
    }

    SkeletalMesh->MeshesAmount  = LoadFile.MeshesCount;
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

    Quat a(DEGREE_TO_RAD(0.0f), n);
    Quat b(DEGREE_TO_RAD(45.0f), n);

    Mat3x3 TestMat = {};

    Vec3 Test1 = b * Pos;
    b.ToMat3(&TestMat);
    Vec3 Test2 = TestMat * Pos;

    Quat DeltaQuat = Quat::Slerp(a, b, 1.0f);

    Vec3 WooDooMagic = (DeltaQuat * a) * Pos;
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
    SceneMainLight->Specification.Color                 = { 0.5f, 0.5f, 0.5f };
    SceneMainLight->Specification.Intensity             = 0.0f;
    SceneMainLight->Specification.AmbientIntensity      = 0.0;
    SceneMainLight->Specification.SpecularIntensity     = 0.0f;
    SceneMainLight->Direction                           = { 1.0f, 0.0f, 0.0f };

    Vec3        PointLightPosition  = { 20.0, 12.0f, 10.0f };
    Vec3        PointLightColor     = { 1.0f, 0.3f, 0.3f };
    PointLight* ScenePointLights    = Cntx->PointLights;
    for (i32 Index = 0; Index < MAX_POINTS_LIGHTS; ++Index) {
        PointLight* CurrentScenePointLight = &ScenePointLights[Index];

        CurrentScenePointLight->Specification.Color             = PointLightColor;
        CurrentScenePointLight->Specification.Intensity         = 0.0f;
        CurrentScenePointLight->Specification.AmbientIntensity  = 0.0;
        CurrentScenePointLight->Specification.SpecularIntensity = 0.0f;
        CurrentScenePointLight->Attenuation.DisctanceMax        = 35.0f;
        CurrentScenePointLight->Attenuation.DisctanceMin        = 5.0f;
        CurrentScenePointLight->Attenuation.AttenuationFactor   = 2.0f;
        CurrentScenePointLight->Attenuation.Position            = PointLightPosition;

        PointLightPosition.x    += 40.f;
        PointLightColor         = { 0.3f, 0.3f, 1.0f };
    }

    SpotLight* SceneSpotLight = &Cntx->SpotLights[0];
    SceneSpotLight->Specification.Color                 = { 1.0f, 1.0f, 1.0f };
    SceneSpotLight->Specification.Intensity             = 1.0f;
    SceneSpotLight->Specification.AmbientIntensity      = 0.05f;
    SceneSpotLight->Specification.SpecularIntensity     = 1.0f;
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

        Object->Transform.Rotation.Bank    = 0.0f;
        Object->Transform.Rotation.Pitch   = 0.0f;
        Object->Transform.Rotation.Heading = 0.0f;
    
        Object->Transform.Position.x = 0.0f;
        Object->Transform.Position.y = 0.0f;
        Object->Transform.Position.z = Position;

        InitMeshComponent(Platform, &Object->ObjMesh, CurrentMeshNode->Flags);

        Position += 10.0f;
    }

    for (i32 Index = 0; Index < DYNAMIC_SCENE_OBJECTS_MAX; ++Index) {
        SceneObject*    Object          = &Cntx->TestDynamocSceneObjects[Index];
        const char*     CurrentMeshName = DynamicSceneObjectsName[Index];

        Object->ObjMesh.ObjectPath = CurrentMeshName;

        Object->Transform.Rotation.Bank    = 0.0f;
        Object->Transform.Rotation.Pitch   = 0.0f;
        Object->Transform.Rotation.Heading = 0.0f;
    
        Object->Transform.Position.x = 0.0f;
        Object->Transform.Position.y = 0.0f;
        Object->Transform.Position.z = Position;

        InitSkeletalMeshComponent(Platform, &Object->ObjMesh);

        Position += 10.0f;
    }

    LoadTerrain(Platform, &Cntx->Terrain, "data/textures/grass_texture.jpg");

    Cntx->TranslationDelta = 0.1f;
    Cntx->RotationDelta = 0.1f;
}

void Frame(Platform *Platform, GameContext *Cntx)
{
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

    Mat4x4 PerspProjection = MakePerspProjection(60.0f, Platform->ScreenOpt.AspectRatio, 0.1f, 100.0f);

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


    DirectionalLight *SceneDirLight = &Cntx->LightSource;

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

    Mat4x4 TerrainWorldTranslation = {};

    MakeTranslationFromVec(&Terrain->Transform.Position, &TerrainWorldTranslation);

    Mat4x4 TerrainWorldRotation = {};
    MakeObjectToUprightRotation(&Terrain->Transform.Rotation, &TerrainWorldRotation);

    Mat3x3 TerrainWorldToObjectRotation = {};
    MakeUprightToObjectRotationMat3x3(&TerrainWorldToObjectRotation, &Terrain->Transform.Rotation);

    Mat4x4 FinalMat = CameraTransformation * TerrainWorldTranslation * TerrainWorldRotation;

    tglBindVertexArray(Terrain->BuffersHandler[OpenGLBuffersLocation::GLVertexArrayLocation]);

    Vec3 ViewerPositionInTerrainUpright     = Cntx->PlayerCamera.Transform.Position - Terrain->Transform.Position;
    Vec3 ViewerPositionInTerrainObjectSpace = TerrainWorldToObjectRotation * ViewerPositionInTerrainUpright;
    Vec3 LightDirectionInTerrainObjectSpace = TerrainWorldToObjectRotation * SceneDirLight->Direction;

    tglUniform3fv(VarStorage->Light.DirectionalLightDirectionLocation, 1, &LightDirectionInTerrainObjectSpace[0]);
    tglUniform3fv(VarStorage->Light.DirectionalLightSpecLocations.ColorLocation, 1, &SceneDirLight->Specification.Color[0]);
    tglUniform1f(VarStorage->Light.DirectionalLightSpecLocations.IntensityLocation, SceneDirLight->Specification.Intensity);
    tglUniform1f(VarStorage->Light.DirectionalLightSpecLocations.AmbientIntensityLocation, SceneDirLight->Specification.AmbientIntensity);
    tglUniform1f(VarStorage->Light.DirectionalLightSpecLocations.SpecularIntensityLocation, SceneDirLight->Specification.SpecularIntensity);

    tglUniform3fv(VarStorage->Light.ViewerPositionLocation, 1, &ViewerPositionInTerrainObjectSpace[0]);

    tglUniformMatrix4fv(VarStorage->Transform.ObjectToWorldTransformationLocation, 1, GL_TRUE, FinalMat[0]);

    PointLight* ScenePointLights = Cntx->PointLights;
    SetupPointLights(Cntx->PointLights, sizeof(Cntx->PointLights) / sizeof(*Cntx->PointLights), ShaderProgramsType::MeshShader, &TerrainWorldToObjectRotation, &Terrain->Transform.Position);

    SpotLight* SceneSpotLight               = &Cntx->SpotLights[0];
    SceneSpotLight->Attenuation.Position    = Cntx->PlayerCamera.Transform.Position;
    SceneSpotLight->Direction               = Target;
    SetupSpotLights(SceneSpotLight, 1, ShaderProgramsType::MeshShader, &TerrainWorldToObjectRotation, &Terrain->Transform.Position);

    tglDrawElements(GL_TRIANGLES, Terrain->IndicesAmount, GL_UNSIGNED_INT, 0);
    // TERRAIN RENDERING END

    if (ScenePointLights[0].Attenuation.DisctanceMin >= 33.0f) {
        Cntx->TranslationDelta *= -1.0f;
    }
    else if (ScenePointLights[0].Attenuation.DisctanceMin <= 0.0f) {
        Cntx->TranslationDelta *= -1.0f;
    }

    for (i32 Index = 0; Index < MAX_POINTS_LIGHTS; ++Index) {
        PointLight* CurrentPointLight = &ScenePointLights[Index];
        
        CurrentPointLight->Attenuation.DisctanceMin += Cntx->TranslationDelta;
    }

    // MESHES RENDERING
    tglUniform3fv(VarStorage->Light.DirectionalLightSpecLocations.ColorLocation, 1, &SceneDirLight->Specification.Color[0]);
    tglUniform1f(VarStorage->Light.DirectionalLightSpecLocations.IntensityLocation, SceneDirLight->Specification.Intensity);
    tglUniform1f(VarStorage->Light.DirectionalLightSpecLocations.AmbientIntensityLocation, SceneDirLight->Specification.AmbientIntensity);
    tglUniform1f(VarStorage->Light.DirectionalLightSpecLocations.SpecularIntensityLocation, SceneDirLight->Specification.SpecularIntensity);

    tglUniform1i(VarStorage->Light.PointLightsAmountLocation, 0);

    for (i32 Index = 0; Index < SCENE_OBJECTS_MAX; ++Index) {
        SceneObject*    CurrentSceneObject  = &Cntx->TestSceneObjects[Index];
        MeshComponent*  Comp                = &CurrentSceneObject->ObjMesh;
        WorldTransform* Transform           = &CurrentSceneObject->Transform;

        Transform->Rotation.Heading += Cntx->RotationDelta;
    
        Mat4x4 ObjectToWorldTranslation = {};
        MakeTranslationFromVec(&Transform->Position, &ObjectToWorldTranslation);
    
        Mat4x4 ObjectToWorlRotation = {};
        MakeObjectToUprightRotation(&Transform->Rotation, &ObjectToWorlRotation);

        Mat3x3 UprightToObjectSpaceRotation = {};
        MakeUprightToObjectRotationMat3x3(&UprightToObjectSpaceRotation, &Transform->Rotation);

        Vec3 LightDirectionInMeshObjectSpace = UprightToObjectSpaceRotation * SceneDirLight->Direction;
        tglUniform3fv(VarStorage->Light.DirectionalLightDirectionLocation, 1, &LightDirectionInMeshObjectSpace[0]);

        Vec3 CameraPositionInObjectUprightSpace = Cntx->PlayerCamera.Transform.Position - Transform->Position;
        Vec3 CameraPositionInObjectLocalSpace   = UprightToObjectSpaceRotation * CameraPositionInObjectUprightSpace;
        tglUniform3fv(VarStorage->Light.ViewerPositionLocation, 1, &CameraPositionInObjectLocalSpace[0]);
    
        tglBindVertexArray(Comp->BuffersHandler[OpenGLBuffersLocation::GLVertexArrayLocation]);
    
        Mat4x4 FinalTransform = CameraTransformation * ObjectToWorldTranslation * ObjectToWorlRotation;
    
        tglUniformMatrix4fv(VarStorage->Transform.ObjectToWorldTransformationLocation, 1, GL_TRUE, FinalTransform[0]);

        SetupPointLights(Cntx->PointLights, sizeof(Cntx->PointLights) / sizeof(*Cntx->PointLights), ShaderProgramsType::MeshShader, &UprightToObjectSpaceRotation, &Transform->Position);
        SetupSpotLights(SceneSpotLight, 1, ShaderProgramsType::MeshShader, &UprightToObjectSpaceRotation, &Transform->Position);

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

    // MESHES RENDERING END

    // SKELETAL MESHES RENDERING

    Shader = &ShadersProgramsCache[ShaderProgramsType::SkeletalMeshShader];
    tglUseProgram(Shader->Program);

    VarStorage = &Shader->ProgramVarsStorage;

    for (i32 Index = 0; Index < DYNAMIC_SCENE_OBJECTS_MAX; ++Index) {
        SceneObject*    CurrentSceneObject  = &Cntx->TestDynamocSceneObjects[Index];
        MeshComponent*  Comp                = &CurrentSceneObject->ObjMesh;
        WorldTransform* Transform           = &CurrentSceneObject->Transform;
    
        tglBindVertexArray(Comp->BuffersHandler[OpenGLBuffersLocation::GLVertexArrayLocation]);

        Mat4x4 ObjectToWorldTranslation = {};
        MakeTranslationFromVec(&Transform->Position, &ObjectToWorldTranslation);
    
        Mat4x4 ObjectToWorlRotation = {};
        MakeObjectToUprightRotation(&Transform->Rotation, &ObjectToWorlRotation);
    
        Mat4x4 FinalTransform = CameraTransformation * ObjectToWorldTranslation * ObjectToWorlRotation;
    
        tglUniformMatrix4fv(VarStorage->Transform.ObjectToWorldTransformationLocation, 1, GL_TRUE, FinalTransform[0]);

        tglUniform1i(VarStorage->Animation.BoneIDLocation, Cntx->BoneID);

        for (i32 Index = 0; Index < Comp->MeshesAmount; ++Index) {
            MeshComponentObjects*   MeshInfo        = &Comp->MeshesInfo[Index];
            MeshMaterial*           MeshMaterial    = &MeshInfo->Material;
    
            tglDrawElementsBaseVertex(GL_TRIANGLES, MeshInfo->NumIndices, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * MeshInfo->IndexOffset), MeshInfo->VertexOffset);
        }
    }

    // SKELETAL MESHES RENDERING END
}