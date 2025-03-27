#include "TLib/Utils/Types.h"
#include "TLib/Math/Matrix.h"
#include "TLib/Utils/Debug.h"
#include "TLib/Utils/AssetsLoader.h"
#include "EnginePlatform.h"
#include "Game.h"
#include "Renderer/OpenGLLoader.cpp"

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

void PrepareFrame(Platform *Platform, GameContext *Cntx)
{
    // NOTE(Ismail): values specified by glClearColor are clamped to the range [0,1]
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glViewport(0, 0, Platform->ScreenOpt.ActualWidth, Platform->ScreenOpt.ActualHeight);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    i32 Success;
    char InfoLog[512] = {};

    File VertShader = Platform->ReadFile("data/shaders/shader.vs");

    u32 VertexShaderHandle = tglCreateShader(GL_VERTEX_SHADER);

    tglShaderSource(VertexShaderHandle, 1, (const char**)(&VertShader.Data), NULL);

    Platform->FreeFileData(&VertShader);

    tglCompileShader(VertexShaderHandle);
    tglGetShaderiv(VertexShaderHandle, GL_COMPILE_STATUS, &Success);

    if (!Success) {
        tglGetShaderInfoLog(VertexShaderHandle, 512, NULL, InfoLog);

        Assert(false);
    }

    File FragShader = Platform->ReadFile("data/shaders/shader.fs");

    u32 FragmentShaderHandle = tglCreateShader(GL_FRAGMENT_SHADER);

    tglShaderSource(FragmentShaderHandle, 1, (const char**)(&FragShader.Data), NULL);

    Platform->FreeFileData(&FragShader);

    tglCompileShader(FragmentShaderHandle);

    tglGetShaderiv(FragmentShaderHandle, GL_COMPILE_STATUS, &Success);

    if (!Success) {
        tglGetShaderInfoLog(VertexShaderHandle, 512, NULL, InfoLog);

        Assert(false);
    }

    u32 FinalShaderProgramm;
    FinalShaderProgramm = tglCreateProgram();

    tglAttachShader(FinalShaderProgramm, VertexShaderHandle);
    tglAttachShader(FinalShaderProgramm, FragmentShaderHandle);
    tglLinkProgram(FinalShaderProgramm);

    tglGetProgramiv(FinalShaderProgramm, GL_LINK_STATUS, &Success);
    if(!Success) {
        tglGetProgramInfoLog(FinalShaderProgramm, 512, NULL, InfoLog);
    }

    tglDeleteShader(VertexShaderHandle);
    tglDeleteShader(FragmentShaderHandle);

    Cntx->FinalShaderProgramm = FinalShaderProgramm;
    Cntx->MatLocation = tglGetUniformLocation(FinalShaderProgramm, "ObjectToWorldTranslation");

    ObjFileLoaderFlags Flags = {
        0, 0
    };

    Cntx->ReadedFile.Positions = (Vec3*)VirtualAlloc(0, sizeof(Vec3) * 30000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    Cntx->ReadedFile.Normals = (Vec3*)VirtualAlloc(0, sizeof(Vec3) * 30000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    Cntx->ReadedFile.TextureCoord = (Vec2*)VirtualAlloc(0, sizeof(Vec2) * 30000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    Cntx->ReadedFile.Indices = (u32*)VirtualAlloc(0, sizeof(u32) * 30000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    LoadObjFile("data/obj/cube.obj", &Cntx->ReadedFile, Flags);

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

    tglGenVertexArrays(1, &Cntx->VAO);  
    tglBindVertexArray(Cntx->VAO);

    u32 VBO[3];
    tglGenBuffers(3, VBO);
    tglBindBuffer(GL_ARRAY_BUFFER, VBO[0]);

    tglBufferData(GL_ARRAY_BUFFER, sizeof(*Cntx->ReadedFile.Positions) * Cntx->ReadedFile.PositionsCount, Cntx->ReadedFile.Positions, GL_STATIC_DRAW);
    tglVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    tglEnableVertexAttribArray(0);

    tglBindBuffer(GL_ARRAY_BUFFER, VBO[1]);

    tglBufferData(GL_ARRAY_BUFFER, sizeof(Colors), Colors, GL_STATIC_DRAW);
    tglVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    tglEnableVertexAttribArray(1);

    tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[2]);

    tglBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*Cntx->ReadedFile.Indices) * Cntx->ReadedFile.IndicesCount, Cntx->ReadedFile.Indices, GL_STATIC_DRAW);

    tglBindVertexArray(0);
    tglBindBuffer(GL_ARRAY_BUFFER, 0);
    tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    Cntx->Trans = 0.0f;
    Cntx->Delta = 0.005;

    Cntx->Rot = 0.0f;
    Cntx->RotDelta = 0.1f;
}

void Frame(Platform *Platform, GameContext *Cntx)
{
    glClear(GL_COLOR_BUFFER_BIT);

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

    Cntx->PlayerCamera.Position.x += XTranslationMultiplyer * 0.1f;
    Cntx->PlayerCamera.Position.z += ZTranslationMultiplyer * 0.1f;

    Mat4x4 CameraTranslation = MakeInverseTranslation(&Cntx->PlayerCamera.Position);

    Mat4x4 FinalMat = PerspProjection * CameraTranslation * ZTranslation.Trans * RotationMat * Scale.Scale;

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

    tglUseProgram(Cntx->FinalShaderProgramm);
    tglBindVertexArray(Cntx->VAO);

    tglUniformMatrix4fv(Cntx->MatLocation, 1, GL_TRUE, FinalMat.Matrix[0]);

    tglDrawElements(GL_TRIANGLES, Cntx->ReadedFile.IndicesCount, GL_UNSIGNED_INT, 0);
}