#include "TLib/Utils/Types.h"
#include "TLib/Math/Matrix.h"
#include "TLib/Utils/Debug.h"
#include "TLib/Utils/AssetsLoader.h"
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
        tglGetShaderInfoLog(VertexShaderHandle, 512, NULL, InfoLog);

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
    i32 BitsPerPixel;
    u8* Data;
};

Statuses LoadTextureFile(const char *Path, TextureFile *ReadedFile, bool32 VerticalFlip)
{
    stbi_set_flip_vertically_on_load_thread(VerticalFlip);

    u8* ImageData = stbi_load(Path, &ReadedFile->Width, &ReadedFile->Height, &ReadedFile->BitsPerPixel, STBI_default);

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
        "data/shaders/terrain_shader.vs", 
        "data/shaders/terrain_shader.fs" 
    },
};

void InitShaderProgramsCache(Platform *Platform)
{
    for (i32 Index = 0; Index < ShaderProgramsType::ShaderProgramsTypeMax; ++Index) {
        ShaderProgram*  Shader  = &ShadersProgramsCache[Index];
        ShadersName*    Names   = &ShaderProgramNames[Index];

        u32 ShaderProgram = CreateShaderProgram(Platform, Names->VertexShaderName, Names->FragmentShaderName);

        Shader->ProgramVarsStorage.Common.ObjectToWorldTransformationLocation       = tglGetUniformLocation(ShaderProgram, "ObjectToWorldTransformation");
        Shader->ProgramVarsStorage.Common.MaterialAmbientColorLocation              = tglGetUniformLocation(ShaderProgram, "MeshMaterial.AmbientColor");
        Shader->ProgramVarsStorage.Common.MaterialDiffuseColorLocation              = tglGetUniformLocation(ShaderProgram, "MeshMaterial.DiffuseColor");
        Shader->ProgramVarsStorage.Common.DirectionalLightColorLocation             = tglGetUniformLocation(ShaderProgram, "SceneDirectionalLight.Color");
        Shader->ProgramVarsStorage.Common.DirectionalLightDirectionLocation         = tglGetUniformLocation(ShaderProgram, "SceneDirectionalLight.Direction");
        Shader->ProgramVarsStorage.Common.DirectionalLightIntensityLocation         = tglGetUniformLocation(ShaderProgram, "SceneDirectionalLight.Intensity");
        Shader->ProgramVarsStorage.Common.DirectionalLightAmbientIntensityLocation  = tglGetUniformLocation(ShaderProgram, "SceneDirectionalLight.AmbientIntensity");
        Shader->ProgramVarsStorage.Common.DiffuseTexture.Location                   = tglGetUniformLocation(ShaderProgram, "DiffuseTexture");
        Shader->ProgramVarsStorage.Common.DiffuseTexture.Unit                       = GL_TEXTURE0;
        Shader->ProgramVarsStorage.Common.DiffuseTexture.UnitNum                    = GL_TEXTURE_UNIT0;

        tglUseProgram(ShaderProgram);

        tglUniform1i(Shader->ProgramVarsStorage.Common.DiffuseTexture.Location, Shader->ProgramVarsStorage.Common.DiffuseTexture.UnitNum);

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

void InitMeshComponent(Platform *Platform, MeshComponent *ToLoad)
{
    ObjFileLoaderFlags  LoadFlags   = { 1, 0 };
    u32*                Buffers     = ToLoad->BuffersHandler;
    ObjFile             LoadFile    = {};

    AllocateDebugObjFile(&LoadFile);

    LoadObjFile(ToLoad->ObjectPath, &LoadFile, LoadFlags);

    MeshComponentObjects* ComponentObjects = (MeshComponentObjects*)VirtualAlloc(0, sizeof(MeshComponentObjects) * LoadFile.MeshesCount, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    tglGenVertexArrays(1, &Buffers[OpenGLBuffersLocation::GLVertexArrayLocation]); 
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
        TextureFile             Texture                     = {};

        CurrentComponentObject->IndexOffset     = CurrentMesh->IndexOffset;
        CurrentComponentObject->VertexOffset    = CurrentMesh->VertexOffset;
        CurrentComponentObject->NumIndices      = CurrentMesh->IndicesAmount;
    
        if (CurrentMeshMaterial->HaveTexture) {
            if (LoadTextureFile(CurrentMeshMaterial->TextureFilePath, &Texture, 1) != Statuses::Success) {
                Assert(false);
            }

            CurrentComponentMaterial->HaveTexture = 1;
    
            glGenTextures(1, &CurrentComponentMaterial->TextureHandle);
            glBindTexture(GL_TEXTURE_2D, CurrentComponentMaterial->TextureHandle);
        
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Texture.Width, Texture.Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture.Data);
            tglGenerateMipmap(GL_TEXTURE_2D);
        
            FreeTextureFile(&Texture);
    
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        CurrentComponentMaterial->AmbientColor = CurrentMeshMaterial->AmbientColor;
        CurrentComponentMaterial->DiffuseColor = CurrentMeshMaterial->DiffuseColor;
    }

    ToLoad->MeshesAmount    = LoadFile.MeshesCount;
    ToLoad->MeshesInfo      = ComponentObjects;
}

#define SCENE_OBJECTS_NAME_MAX 2

const char *SceneObjectsName[SCENE_OBJECTS_NAME_MAX] = {
    "data/obj/Golem.obj",
    "data/obj/cube_brick_texture.obj"
};

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

    // NOTE(Ismail): values specified by glClearColor are clamped to the range [0,1]
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    
    glViewport(0, 0, Platform->ScreenOpt.ActualWidth, Platform->ScreenOpt.ActualHeight);

    DirectionalLight* SceneMainLight    = &Cntx->LightSource;
    SceneMainLight->Color               = { 1.0f, 1.0f, 1.0f };
    SceneMainLight->Direction           = { 0.707106f, -0.707106f, 0.0f };
    SceneMainLight->Intensity           = 0.90f;
    SceneMainLight->AmbientIntensity    = 0.10f;

    InitShaderProgramsCache(Platform);

    real32 Position = 10.0f;
    for (i32 Index = 0; Index < SCENE_OBJECTS_NAME_MAX; ++Index) {
        SceneObject *Object = &Cntx->TestSceneObjects[Index];
        Object->ObjMesh.ObjectPath = SceneObjectsName[Index];

        Object->Transform.Rotation.Bank    = 0.0f;
        Object->Transform.Rotation.Pitch   = 0.0f;
        Object->Transform.Rotation.Heading = 0.0f;
    
        Object->Transform.Position.x = 0.0f;
        Object->Transform.Position.y = 0.0f;
        Object->Transform.Position.z = Position;

        InitMeshComponent(Platform, &Object->ObjMesh);

        Position += 10.0f;
    }

    LoadTerrain(Platform, &Cntx->Terrain, "data/textures/grass_texture.jpg");

    Cntx->TranslationDelta = 0.005;
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


    Terrain *Terrain = &Cntx->Terrain;
    Mat4x4 TerrainWorldTranslation = {};

    MakeTranslationFromVec(&Terrain->Transform.Position, &TerrainWorldTranslation);

    Mat4x4 TerrainWorldRotation = {};
    MakeObjectToUprightRotation(&Terrain->Transform.Rotation, &TerrainWorldRotation);

    Mat3x3 TerrainWorldToObjectRotation = {};
    MakeUprightToObjectRotationMat3x3(&TerrainWorldToObjectRotation, &Terrain->Transform.Rotation);

    Mat4x4 FinalMat = CameraTransformation * TerrainWorldTranslation * TerrainWorldRotation;

    ShaderProgram *TerrainShader = &ShadersProgramsCache[ShaderProgramsType::TerrainShader];

    tglUseProgram(TerrainShader->Program);

    tglActiveTexture(TerrainShader->ProgramVarsStorage.Common.DiffuseTexture.Unit);
    glBindTexture(GL_TEXTURE_2D, Terrain->TextureHandle);

    tglBindVertexArray(Terrain->BuffersHandler[OpenGLBuffersLocation::GLVertexArrayLocation]);

    ShaderProgram::ProgramVariablesStorage *VarStorage = &TerrainShader->ProgramVarsStorage;

    Vec3 LightDirectionInTerrainObjectSpace = TerrainWorldToObjectRotation * SceneDirLight->Direction;
    
    tglUniform3fv(VarStorage->Common.DirectionalLightColorLocation, 1, &SceneDirLight->Color[0]);
    tglUniform3fv(VarStorage->Common.DirectionalLightDirectionLocation, 1, &LightDirectionInTerrainObjectSpace[0]);
    tglUniform1f(VarStorage->Common.DirectionalLightIntensityLocation, SceneDirLight->Intensity);
    tglUniform1f(VarStorage->Common.DirectionalLightAmbientIntensityLocation, SceneDirLight->AmbientIntensity);

    Vec3 TerrainDiffuseColor = { 1.0f, 1.0f, 1.0f };
    Vec3 TerrainAmbientColor = { 1.0f, 1.0f, 1.0f };

    tglUniform3fv(VarStorage->Common.MaterialDiffuseColorLocation, 1, &TerrainDiffuseColor[0]);
    tglUniform3fv(VarStorage->Common.MaterialAmbientColorLocation, 1, &TerrainAmbientColor[0]);

    tglUniformMatrix4fv(VarStorage->Common.ObjectToWorldTransformationLocation, 1, GL_TRUE, FinalMat[0]);

    tglDrawElements(GL_TRIANGLES, Terrain->IndicesAmount, GL_UNSIGNED_INT, 0);


    ShaderProgram *MeshShader = &ShadersProgramsCache[ShaderProgramsType::MeshShader];
    tglUseProgram(MeshShader->Program);

    VarStorage = &MeshShader->ProgramVarsStorage;

    tglUniform3fv(VarStorage->Common.DirectionalLightColorLocation, 1, &SceneDirLight->Color[0]);
    tglUniform1f(VarStorage->Common.DirectionalLightIntensityLocation, SceneDirLight->Intensity);
    tglUniform1f(VarStorage->Common.DirectionalLightAmbientIntensityLocation, SceneDirLight->AmbientIntensity);

    for (i32 Index = 0; Index < SCENE_OBJECTS_MAX; ++Index) {
        SceneObject*    CurrentSceneObject  = &Cntx->TestSceneObjects[Index];
        MeshComponent*  Comp                = &CurrentSceneObject->ObjMesh;
        WorldTransform* Transform           = &CurrentSceneObject->Transform;

        Transform->Rotation.Heading += Cntx->RotationDelta;
    
        Mat4x4 ObjectToWorldTranslation = {};
        MakeTranslationFromVec(&Transform->Position, &ObjectToWorldTranslation);
    
        Mat4x4 ObjectToWorlRotation = {};
        MakeObjectToUprightRotation(&Transform->Rotation, &ObjectToWorlRotation);

        Mat3x3 WorldToObjectSpaceRotation = {};
        MakeUprightToObjectRotationMat3x3(&WorldToObjectSpaceRotation, &Transform->Rotation);

        Vec3 LightDirectionInMeshObjectSpace = WorldToObjectSpaceRotation * SceneDirLight->Direction;
        tglUniform3fv(VarStorage->Common.DirectionalLightDirectionLocation, 1, &LightDirectionInMeshObjectSpace[0]);
    
        tglBindVertexArray(Comp->BuffersHandler[OpenGLBuffersLocation::GLVertexArrayLocation]);
    
        Mat4x4 FinalTransform = CameraTransformation * ObjectToWorldTranslation * ObjectToWorlRotation;
    
        tglUniformMatrix4fv(MeshShader->ProgramVarsStorage.Common.ObjectToWorldTransformationLocation, 1, GL_TRUE, FinalTransform[0]);
    
        for (i32 Index = 0; Index < Comp->MeshesAmount; ++Index) {
            MeshComponentObjects*   MeshInfo        = &Comp->MeshesInfo[Index];
            MeshMaterial*           MeshMaterial    = &MeshInfo->Material;

            tglUniform3fv(VarStorage->Common.MaterialDiffuseColorLocation, 1, &MeshMaterial->DiffuseColor[0]);
            tglUniform3fv(VarStorage->Common.MaterialAmbientColorLocation, 1, &MeshMaterial->AmbientColor[0]);

            if (MeshMaterial->HaveTexture) {
                // TODO(Ismail): why i always call this instead i can just once call tglActiveTexture for needed texture units
                tglActiveTexture(MeshShader->ProgramVarsStorage.Common.DiffuseTexture.Unit);
                glBindTexture(GL_TEXTURE_2D, MeshMaterial->TextureHandle);
            }
            else {
                glBindTexture(GL_TEXTURE_2D, 0);
            }
    
            tglDrawElementsBaseVertex(GL_TRIANGLES, MeshInfo->NumIndices, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * MeshInfo->IndexOffset), MeshInfo->VertexOffset);
        }
    }
}