#include "TLib/Utils/Types.h"
#include "TLib/Math/Matrix.h"
#include "TLib/Utils/Debug.h"
#include "TLib/Utils/AssetsLoader.h"
#include "EnginePlatform.h"
#include "Game.h"
#include "TGL.h"
#include "TLib/3rdparty/stb/stb_image.h"

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

    Vec3 operator*(const Vec3 &b) const {
        // NOTE: it is faster to make rotation from quaternion and then multiply vector to that matrix
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
void MakeObjectToUprightRotation(Mat4x4 *Mat, Rotation *Rot)
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

void MakeTranslationFromVec(Vec3 *Trans, Translation *Result)
{
    Result->Trans = {
        1.0f,   0.0f,   0.0f, Trans->x,
        0.0f,   1.0f,   0.0f, Trans->y,
        0.0f,   0.0f,   1.0f, Trans->z,
        0.0f,   0.0f,   0.0f,     1.0f,
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

void GenerateTerrainMesh(GameContext *Ctx, real32 TextureScale, i32 Size, i32 VertexAmount)
{
    Vec3 *Vertices              = Ctx->Vertices;
    Vec2 *Textures              = Ctx->Textures;
    u32 *Indices                = Ctx->Indices;
    i32 RowCubeAmount           = VertexAmount - 1;
    i32 TotalCubeAmount         = SQUARE(RowCubeAmount);
    real32 VertexLen            = (real32)Size / (real32)VertexAmount;
    real32 OneOverVertexAmount  = TextureScale / ((real32)VertexAmount - 1.0f);

    for (i32 ZIndex = 0; ZIndex < VertexAmount; ++ZIndex) {
        for (i32 XIndex = 0; XIndex < VertexAmount; ++XIndex) {
            Vertices[ZIndex * VertexAmount + XIndex] = {
                XIndex * VertexLen,
                0.0f,
                ZIndex * VertexLen,
            };

            Textures[ZIndex * VertexAmount + XIndex] = {
                (real32)XIndex * OneOverVertexAmount,
                (real32)ZIndex * OneOverVertexAmount,
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

void LoadTerrain(Vec3 *Position, u32 PosLen, Vec2 *Textures, u32 TetureLen, u32 *Indices, u32 IndLen, GraphicComponent *TerrainGraphicOut)
{
    u32 *BuffersHandler = TerrainGraphicOut->BuffersHandler;

    tglGenVertexArrays(1, &BuffersHandler[VERTEX_ARRAY_LOCATION]);  
    tglBindVertexArray(BuffersHandler[VERTEX_ARRAY_LOCATION]);

    tglGenBuffers(MAX - 1, BuffersHandler);

    tglBindBuffer(GL_ARRAY_BUFFER, BuffersHandler[POSITION_LOCATION]);
    tglBufferData(GL_ARRAY_BUFFER, sizeof(*Position) * PosLen, Position, GL_STATIC_DRAW);
    tglVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    tglEnableVertexAttribArray(POSITION_LOCATION);

    tglBindBuffer(GL_ARRAY_BUFFER, BuffersHandler[TEXTURES_LOCATION]);
    tglBufferData(GL_ARRAY_BUFFER, sizeof(*Textures) * TetureLen, Textures, GL_STATIC_DRAW);
    tglVertexAttribPointer(TEXTURES_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    tglEnableVertexAttribArray(TEXTURES_LOCATION);

    tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BuffersHandler[INDEX_ARRAY_LOCATION]);
    tglBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*Indices) * IndLen, Indices, GL_STATIC_DRAW);

    tglBindVertexArray(0);
    tglBindBuffer(GL_ARRAY_BUFFER, 0);
    tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

u32 CreateShaderProgram(Platform *Platform, const char *VertexShader, const char *FragmentShader)
{
    File VertShader;
    File FragShader;
    u32 VertexShaderHandle;
    u32 FragmentShaderHandle;
    u32 FinalShaderProgram;

    i32 Success;
    char InfoLog[512] = {};

    VertShader = Platform->ReadFile(VertexShader);

    VertexShaderHandle = tglCreateShader(GL_VERTEX_SHADER);

    tglShaderSource(VertexShaderHandle, 1, (const char**)(&VertShader.Data), NULL);

    Platform->FreeFileData(&VertShader);

    tglCompileShader(VertexShaderHandle);
    tglGetShaderiv(VertexShaderHandle, GL_COMPILE_STATUS, &Success);

    if (!Success) {
        tglGetShaderInfoLog(VertexShaderHandle, 512, NULL, InfoLog);

        Assert(false);
    }

    FragShader = Platform->ReadFile(FragmentShader);

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
    Quat b(DEGREE_TO_RAD(179.9f), n);

    Quat DeltaQuat = Quat::Slerp(a, b, 1.0f);

    Vec3 WooDooMagic = (DeltaQuat * a) * Pos;
    */

    // NOTE(Ismail): values specified by glClearColor are clamped to the range [0,1]
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glViewport(0, 0, Platform->ScreenOpt.ActualWidth, Platform->ScreenOpt.ActualHeight);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    Cntx->FinalShaderProgram = CreateShaderProgram(Platform, "data/shaders/shader.vs", "data/shaders/shader.fs");
    Cntx->MatLocation = tglGetUniformLocation(Cntx->FinalShaderProgram, "ObjectToWorldTranslation");

    ObjFileLoaderFlags Flags = {
        0, 0
    };

    Cntx->ReadedFile.Positions = (Vec3*)VirtualAlloc(0, sizeof(Vec3) * 30000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    Cntx->ReadedFile.Normals = (Vec3*)VirtualAlloc(0, sizeof(Vec3) * 30000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    Cntx->ReadedFile.TextureCoord = (Vec2*)VirtualAlloc(0, sizeof(Vec2) * 30000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    Cntx->ReadedFile.Indices = (u32*)VirtualAlloc(0, sizeof(u32) * 30000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    LoadObjFile("data/obj/cuber_textured_normals.obj", &Cntx->ReadedFile, Flags);

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

    TextureFile Texture = {};
    if (LoadTextureFile("data/textures/bricks_textures.jpg", &Texture, 1) != Statuses::Success) {
        Assert(false);
    }

    i32 ErrorCode = glGetError();
    if (ErrorCode != GL_NO_ERROR) {
        Assert(false);
    }

    u32 TextureHandle;
    glGenTextures(1, &TextureHandle);
    glBindTexture(GL_TEXTURE_2D, TextureHandle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Texture.Width, Texture.Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture.Data);
    tglGenerateMipmap(GL_TEXTURE_2D);

    FreeTextureFile(&Texture);

    Cntx->TextureHandle = TextureHandle;

    glBindTexture(GL_TEXTURE_2D, 0);

    ErrorCode = glGetError();
    if (ErrorCode != GL_NO_ERROR) {
        Assert(false);
    }

    tglUseProgram(Cntx->FinalShaderProgram);
    i32 TextLocation = tglGetUniformLocation(Cntx->FinalShaderProgram, "texture1");
    tglUniform1i(TextLocation, GL_TEXTURE_UNIT0);

    ErrorCode = glGetError();
    if (ErrorCode != GL_NO_ERROR) {
        Assert(false);
    }

    tglGenVertexArrays(1, &Cntx->VAO);  
    tglBindVertexArray(Cntx->VAO);

    u32 VBO[4];
    tglGenBuffers(4, VBO);
    tglBindBuffer(GL_ARRAY_BUFFER, VBO[0]);

    tglBufferData(GL_ARRAY_BUFFER, sizeof(*Cntx->ReadedFile.Positions) * Cntx->ReadedFile.PositionsCount, Cntx->ReadedFile.Positions, GL_STATIC_DRAW);
    tglVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    tglEnableVertexAttribArray(0);

    tglBindBuffer(GL_ARRAY_BUFFER, VBO[1]);

    tglBufferData(GL_ARRAY_BUFFER, sizeof(Colors), Colors, GL_STATIC_DRAW);
    tglVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    tglEnableVertexAttribArray(1);

    tglBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    tglBufferData(GL_ARRAY_BUFFER, sizeof(*Cntx->ReadedFile.TextureCoord) * Cntx->ReadedFile.TexturesCount, Cntx->ReadedFile.TextureCoord, GL_STATIC_DRAW);
    tglVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    tglEnableVertexAttribArray(2);

    tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[3]);
    tglBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*Cntx->ReadedFile.Indices) * Cntx->ReadedFile.IndicesCount, Cntx->ReadedFile.Indices, GL_STATIC_DRAW);

    tglBindVertexArray(0);
    tglBindBuffer(GL_ARRAY_BUFFER, 0);
    tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    Cntx->Trans = 0.0f;
    Cntx->Delta = 0.005;

    Cntx->Rot = 0.0f;
    Cntx->RotDelta = 0.1f;

    GenerateTerrainMesh(Cntx, 4.0f, 10, BATTLE_AREA_GRID_VERT_AMOUNT);
    LoadTerrain(Cntx->Vertices, SQUARE(BATTLE_AREA_GRID_VERT_AMOUNT), Cntx->Textures, SQUARE(BATTLE_AREA_GRID_VERT_AMOUNT), Cntx->Indices, TERRAIN_INDEX_AMOUNT, &Cntx->BattleGrid);

    TextureFile TerrainTexture = {};
    if (LoadTextureFile("data/textures/grass_texture.jpg", &TerrainTexture, 0) != Statuses::Success) {
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

    Cntx->TerrainTextureHandle = TerrainTextureHandle;

    glBindTexture(GL_TEXTURE_2D, 0);

    Cntx->BattleGrid.ShaderProgram = CreateShaderProgram(Platform, "data/shaders/terrain_shader.vs", "data/shaders/terrain_shader.fs");
    Cntx->BattleGridMatLocation = tglGetUniformLocation(Cntx->BattleGrid.ShaderProgram, "ObjectToWorldTranslation");
    i32 TextureSamplerLocation = tglGetUniformLocation(Cntx->BattleGrid.ShaderProgram, "texture1");

    tglUseProgram(Cntx->BattleGrid.ShaderProgram);
    tglUniform1i(TextureSamplerLocation, GL_TEXTURE_UNIT0);

    FreeTextureFile(&TerrainTexture);
    
    ErrorCode = glGetError();
    if (ErrorCode != GL_NO_ERROR) {
        //Assert(false);
    }
}

void Frame(Platform *Platform, GameContext *Cntx)
{
    glClear(GL_COLOR_BUFFER_BIT);

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

    if (Cntx->Trans >= 1.0f || Cntx->Trans <= -1.0f) {
        Cntx->Delta *= -1.0f;
    }

    if (Cntx->Rot >= 360.0f || Cntx->Rot <= -360.0f) {
        Cntx->RotDelta *= -1.0f;
    }

    Cntx->Trans += Cntx->Delta;
    Vec3 TransVec = {};

    TransVec.z = 2.0f;
    Translation ZTranslation;

    MakeTranslationFromVec(&TransVec, &ZTranslation);

    Cntx->Rot += Cntx->RotDelta;

    Mat4x4 RotationMat = {};
    Rotation Rotator = {};
    Rotator.Heading = Cntx->Rot;
    MakeObjectToUprightRotation(&RotationMat, &Rotator);

    UniformScale Scale = {};
    CreateUniformScale(0.7, &Scale);

    Mat4x4 PerspProjection = MakePerspProjection(60.0f, Platform->ScreenOpt.AspectRatio, 0.1f, 100.0f);

    real32 ZTranslationMultiplyer = (real32)(Platform->Input.WButton.State + (-1 * Platform->Input.SButton.State)); // 1.0 if W Button -1.0 if S Button and 0 if W and S Button pressed together
    real32 XTranslationMultiplyer = (real32)(Platform->Input.DButton.State + (-1 * Platform->Input.AButton.State));

    Vec3 Target = {};
    Vec3 Right = {};
    Vec3 Up = {};

    RotationToDirectionVecotrs(&Cntx->PlayerCamera.Rotator, &Target, &Right, &Up);

    Cntx->PlayerCamera.Position += (Target * ZTranslationMultiplyer * 0.1f) + (Right * XTranslationMultiplyer * 0.1f);

    Mat4x4 CameraTranslation = MakeInverseTranslation(&Cntx->PlayerCamera.Position);

    Cntx->PlayerCamera.Rotator.Bank = 0.0f;
    Cntx->PlayerCamera.Rotator.Pitch += RAD_TO_DEGREE(Platform->Input.MouseInput.Moution.y) * 0.5f;
    Cntx->PlayerCamera.Rotator.Heading += RAD_TO_DEGREE(Platform->Input.MouseInput.Moution.x) * 0.5f;

    Mat4x4 CameraUprightToObjectRotation = {};
    MakeUprightToObjectRotation(&CameraUprightToObjectRotation, &Cntx->PlayerCamera.Rotator);

    Mat4x4 FinalMat = PerspProjection * CameraUprightToObjectRotation * CameraTranslation * ZTranslation.Trans * RotationMat * Scale.Scale;

    u64 PositionsCount = Cntx->ReadedFile.PositionsCount;
    Vec4 ResultsVectorsWithoutPerspDiv[100] = {};
    Vec3 ResultsVectors[100] = {};
    i32 PosIndex = 0;
    for (Vec3 *Position = Cntx->ReadedFile.Positions; PositionsCount; --PositionsCount ) {
        Vec4 Tmp = { 
            Position->x,
            Position->y,
            Position->z,
            1.0f
        };

        Vec4 Result = FinalMat * Tmp;
        ResultsVectorsWithoutPerspDiv[PosIndex] = Result;

        Vec3 PerspDivisionRes = {
            Result.x / Result.w,
            Result.y / Result.w,
            Result.z / Result.w,
        };

        ResultsVectors[PosIndex++] = PerspDivisionRes;

        ++Position;
    }

    tglActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Cntx->TextureHandle);

    tglUseProgram(Cntx->FinalShaderProgram);

    tglBindVertexArray(Cntx->VAO);

    tglUniformMatrix4fv(Cntx->MatLocation, 1, GL_TRUE, FinalMat.Matrix[0]);

    tglDrawElements(GL_TRIANGLES, Cntx->ReadedFile.IndicesCount, GL_UNSIGNED_INT, 0);

    tglActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Cntx->TerrainTextureHandle);

    tglUseProgram(Cntx->BattleGrid.ShaderProgram);
    tglBindVertexArray(Cntx->BattleGrid.BuffersHandler[VERTEX_ARRAY_LOCATION]);

    FinalMat = PerspProjection * CameraUprightToObjectRotation * CameraTranslation;

    tglUniformMatrix4fv(Cntx->BattleGridMatLocation, 1, GL_TRUE, FinalMat.Matrix[0]);

    tglDrawElements(GL_TRIANGLES, TERRAIN_INDEX_AMOUNT, GL_UNSIGNED_INT, 0);
}