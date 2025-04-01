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

void GenerateTerrainMesh(GameContext *Ctx, i32 Size, i32 VertexAmount)
{
    Vec3 *Vertices      = Ctx->Vertices;
    u32 *Indices        = Ctx->Indices;
    i32 RowCubeAmount   = VertexAmount - 1;
    i32 TotalCubeAmount = SQUARE(RowCubeAmount);
    real32 VertexLen    = (real32)Size / (real32)VertexAmount;

    for (i32 ZIndex = 0; ZIndex < VertexAmount; ++ZIndex) {
        for (i32 XIndex = 0; XIndex < VertexAmount; ++XIndex) {
            Vertices[ZIndex * VertexAmount + XIndex] = {
                XIndex * VertexLen,
                0.0f,
                ZIndex * VertexLen,
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

void LoadTerrain(Vec3 *Position, u32 PosLen, u32 *Indices, u32 IndLen, GraphicComponent *TerrainGraphicOut)
{
    u32 *BuffersHandler = TerrainGraphicOut->BuffersHandler;

    tglGenVertexArrays(1, &BuffersHandler[VERTEX_ARRAY_LOCATION]);  
    tglBindVertexArray(BuffersHandler[VERTEX_ARRAY_LOCATION]);

    tglGenBuffers(MAX - 1, BuffersHandler);

    tglBindBuffer(GL_ARRAY_BUFFER, BuffersHandler[POSITION_LOCATION]);
    tglBufferData(GL_ARRAY_BUFFER, sizeof(*Position) * PosLen, Position, GL_STATIC_DRAW);
    tglVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    tglEnableVertexAttribArray(POSITION_LOCATION);

    tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BuffersHandler[INDEX_ARRAY_LOCATION]);
    tglBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*Indices) * IndLen, Indices, GL_STATIC_DRAW);

    tglBindVertexArray(0);
    tglBindBuffer(GL_ARRAY_BUFFER, 0);
    tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

u32 CreateShaderProgram(Platform *Platform, GameContext *Cntx, const char *VertexShader, const char *FragmentShader)
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
    }

    tglDeleteShader(VertexShaderHandle);
    tglDeleteShader(FragmentShaderHandle);

    return FinalShaderProgram;
}

void PrepareFrame(Platform *Platform, GameContext *Cntx)
{
    // NOTE(Ismail): values specified by glClearColor are clamped to the range [0,1]
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glViewport(0, 0, Platform->ScreenOpt.ActualWidth, Platform->ScreenOpt.ActualHeight);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    Cntx->FinalShaderProgram = CreateShaderProgram(Platform, Cntx, "data/shaders/shader.vs", "data/shaders/shader.fs");
    Cntx->MatLocation = tglGetUniformLocation(Cntx->FinalShaderProgram, "ObjectToWorldTranslation");

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

    GenerateTerrainMesh(Cntx, 10, BATTLE_AREA_GRID_VERT_AMOUNT);
    LoadTerrain(Cntx->Vertices, SQUARE(BATTLE_AREA_GRID_VERT_AMOUNT), Cntx->Indices, TERRAIN_INDEX_AMOUNT, &Cntx->BattleGrid);

    Cntx->BattleGrid.ShaderProgram = CreateShaderProgram(Platform, Cntx, "data/shaders/terrain_shader.vs", "data/shaders/terrain_shader.fs");
    Cntx->BattleGridMatLocation = tglGetUniformLocation(Cntx->FinalShaderProgram, "ObjectToWorldTranslation");
}

void Frame(Platform *Platform, GameContext *Cntx)
{
    glClear(GL_COLOR_BUFFER_BIT);

    if (Platform->Input.QButton.State == KeyState::Pressed) {
        if (!Cntx->PolygonModeActive) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            Cntx->PolygonModeActive = 1;
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            Cntx->PolygonModeActive = 0;
        }
        
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

    tglUseProgram(Cntx->FinalShaderProgram);
    tglBindVertexArray(Cntx->VAO);

    tglUniformMatrix4fv(Cntx->MatLocation, 1, GL_TRUE, FinalMat.Matrix[0]);

    tglDrawElements(GL_TRIANGLES, Cntx->ReadedFile.IndicesCount, GL_UNSIGNED_INT, 0);

    tglUseProgram(Cntx->BattleGrid.ShaderProgram);
    tglBindVertexArray(Cntx->BattleGrid.BuffersHandler[VERTEX_ARRAY_LOCATION]);

    FinalMat = PerspProjection * CameraUprightToObjectRotation * CameraTranslation;

    tglUniformMatrix4fv(Cntx->BattleGridMatLocation, 1, GL_TRUE, FinalMat.Matrix[0]);

    tglDrawElements(GL_TRIANGLES, TERRAIN_INDEX_AMOUNT, GL_UNSIGNED_INT, 0);
}