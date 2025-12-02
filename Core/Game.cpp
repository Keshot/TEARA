#include "Types.h"
#include "Math/Matrix.h"
#include "Math/Transformation.h"
#include "Debug.h"
#include "Utils/AssetsLoader.h"
#include "3rdparty/ufbx/ufbx.h"
#include "3rdparty/cgltf/cgltf.h"
#include "EnginePlatform.h"
#include "Game.h"
#include "Rendering/OpenGL/TGL.h"
#include "3rdparty/stb/stb_image.h"
#include "Assets/GltfLoader.h"

ShaderProgram ShadersProgramsCache[ShaderProgramsTypeMax];

void GenerateTerrainMesh(TerrainLoadFile *ToLoad, real32 TextureScale, real32 Size, i32 VertexAmount)
{
    vec3 *Vertices              = ToLoad->Vertices;
    vec2 *Textures              = ToLoad->Textures;
    vec3 *Normals               = ToLoad->Normals;
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
        "../Rendering/shaders/mesh_component_shader.vs", 
        "../Rendering/shaders/mesh_component_shader.fs" 
    },
    { 
        "../Rendering/shaders/skeletal_mesh_component_shader.vs", 
        "../Rendering/shaders/skeletal_mesh_component_shader.fs" 
    },
    { 
        "../Rendering/shaders/particle_shader.vs", 
        "../Rendering/shaders/particle_shader.fs" 
    },
    {
        "../Rendering/shaders/debug_draw.vs", 
        "../Rendering/shaders/debug_draw.fs" 
    },
    {
        "../Rendering/shaders/depth_test_shader.vs", 
        "../Rendering/shaders/depth_test_shader.fs" 
    }
};

#define DIFFUSE_TEXTURE_UNIT            GL_TEXTURE0
#define DIFFUSE_TEXTURE_UNIT_NUM        GL_TEXTURE_UNIT0

#define SPECULAR_EXPONENT_MAP_UNIT      GL_TEXTURE1
#define SPECULAR_EXPONENT_MAP_UNIT_NUM  GL_TEXTURE_UNIT1

#define SHADOW_MAP_TEXTURE_UNIT         GL_TEXTURE2
#define SHADOW_MAP_TEXTURE_UNIT_NUM     GL_TEXTURE_UNIT2

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

        ShaderVariablesStorage->Shadow.ObjectToLightSpaceTransformationLocation = tglGetUniformLocation(ShaderProgram, "ObjectToLightSpaceTransformation");
        ShaderVariablesStorage->Shadow.ShadowMapTexture.Location                = tglGetUniformLocation(ShaderProgram, "ShadowMapTexture");
        ShaderVariablesStorage->Shadow.ShadowMapTexture.Unit                    = SHADOW_MAP_TEXTURE_UNIT;
        ShaderVariablesStorage->Shadow.ShadowMapTexture.UnitNum                 = SHADOW_MAP_TEXTURE_UNIT_NUM;

        ShaderVariablesStorage->Animation.HaveSkinMatricesLocation  = tglGetUniformLocation(ShaderProgram, "HaveSkinMatrices");

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

        if (Index == ShaderProgramsType::SkeletalMeshShader || Index == ShaderProgramsType::DepthTestShader) {
            for (i32 MatrixIndex = 0; MatrixIndex < MAX_BONES; ++MatrixIndex) {
                i32* MatrixLoc = &Shader->ProgramVarsStorage.Animation.AnimationMatricesLocation[MatrixIndex];

                snprintf(HelperStringBuffer, sizeof(HelperStringBuffer), "AnimationBonesMatrices[%d]", MatrixIndex);

                *MatrixLoc = tglGetUniformLocation(ShaderProgram, HelperStringBuffer);
            }
        }

        tglUseProgram(ShaderProgram);

        tglUniform1i(ShaderVariablesStorage->MaterialInfo.DiffuseTexture.Location, ShaderVariablesStorage->MaterialInfo.DiffuseTexture.UnitNum);
        tglUniform1i(ShaderVariablesStorage->MaterialInfo.SpecularExpMap.Location, ShaderVariablesStorage->MaterialInfo.SpecularExpMap.UnitNum);
        tglUniform1i(ShaderVariablesStorage->Shadow.ShadowMapTexture.Location, ShaderVariablesStorage->Shadow.ShadowMapTexture.UnitNum);

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
    File->Positions     = (vec3*)   VirtualAlloc(0, sizeof(vec3) * 140000,   MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    File->Normals       = (vec3*)   VirtualAlloc(0, sizeof(vec3) * 140000,   MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    File->TextureCoord  = (vec2*)   VirtualAlloc(0, sizeof(vec2) * 140000,   MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
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
    vec3 ParticleSquareAppearance[4] {
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
    vec3    Center;
    vec3    Extens;
};

struct ColliderDebugDraw {
    vec3    LinesColor;
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
    vec3 ColliderDebugSquareAppearance[8] {
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
    vec3                InitialScale;
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

struct SerialaizedAnimations {
    i32         AnimId;
    const char* Path;
};

struct DynamicSceneObjectLoader {
    SkeletalCharacters      SkeletCharId;
    SerialaizedAnimations   Mesh;
    SerialaizedAnimations   Animations[MAX_CHARACTER_ANIMATIONS];
    i32                     Amount;
};

DynamicSceneObjectLoader DynamicSceneObjectsName[] = {
    {
        SkeletalCharacters::CharacterPlayer,
        {
            PLayerAnimations::IdleDynamic,
            "data/obj/Idle.gltf"
        },
        {
            {
                PLayerAnimations::WalkDefault,
                "data/obj/Walk.gltf"
            },
            {
                PLayerAnimations::RunDefault,
                "data/obj/Run.gltf"
            }
        },
        2
    },
};

void SetupDirectionalLight(DirectionalLight* Light, ShaderProgramsType ShaderType)
{
    ShaderProgramVariablesStorage *VarStorage = &ShadersProgramsCache[ShaderType].ProgramVarsStorage;

    Rotation LightRotation = Light->Rotation;

    vec3 Target, Right, Up;
    LightRotation.ToVec(Target, Up, Right);

    tglUniform3fv(VarStorage->Light.DirectionalLightDirectionLocation, 1, &Target[0]);
    tglUniform3fv(VarStorage->Light.DirectionalLightSpecLocations.ColorLocation, 1, &Light->Specification.Color[0]);

    tglUniform1f(VarStorage->Light.DirectionalLightSpecLocations.IntensityLocation, Light->Specification.Intensity);
    tglUniform1f(VarStorage->Light.DirectionalLightSpecLocations.AmbientIntensityLocation, Light->Specification.AmbientIntensity);
    tglUniform1f(VarStorage->Light.DirectionalLightSpecLocations.SpecularIntensityLocation, Light->Specification.SpecularIntensity);
}

void SetupPointLights(PointLight* Lights, u32 LightAmount, ShaderProgramsType ShaderType, vec3* ObjectPosition)
{
    ShaderProgramVariablesStorage *VarStorage = &ShadersProgramsCache[ShaderType].ProgramVarsStorage;

    for (u32 Index = 0; Index < LightAmount; ++Index) {
        PointLight* CurrentPointLight = &Lights[Index];
        ShaderProgramVariablesStorage::LightWork::PointLightLocations* PointLightsVarLocations = &VarStorage->Light.PointLightsLocations[Index];

        vec3 LightInObjectUprightPosition  = CurrentPointLight->Attenuation.Position - *ObjectPosition;

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

void SetupSpotLights(SpotLight* Lights, u32 LightAmount, ShaderProgramsType ShaderType, vec3* ObjectPosition)
{
    ShaderProgramVariablesStorage *VarStorage = &ShadersProgramsCache[ShaderType].ProgramVarsStorage;

    for (u32 Index = 0; Index < LightAmount; ++Index) {
        SpotLight* CurrentSpotLight = &Lights[Index];
        ShaderProgramVariablesStorage::LightWork::SpotLightLocations* SpotLightsVarLocations = &VarStorage->Light.SpotLightsLocations[Index];

        vec3 LightInObjectUprightPosition  = CurrentSpotLight->Attenuation.Position - *ObjectPosition;

        tglUniform3fv(SpotLightsVarLocations->AttenuationLocation.PositionLocation, 1, &LightInObjectUprightPosition[0]);
        tglUniform1f(SpotLightsVarLocations->AttenuationLocation.DisctanceMaxLocation, CurrentSpotLight->Attenuation.DisctanceMax);
        tglUniform1f(SpotLightsVarLocations->AttenuationLocation.DisctanceMinLocation, CurrentSpotLight->Attenuation.DisctanceMin);
        tglUniform1f(SpotLightsVarLocations->AttenuationLocation.AttenuationFactorLocation, CurrentSpotLight->Attenuation.AttenuationFactor);

        tglUniform3fv(SpotLightsVarLocations->SpecLocation.ColorLocation, 1, &CurrentSpotLight->Specification.Color[0]);
        tglUniform1f(SpotLightsVarLocations->SpecLocation.IntensityLocation, CurrentSpotLight->Specification.Intensity);
        tglUniform1f(SpotLightsVarLocations->SpecLocation.AmbientIntensityLocation, CurrentSpotLight->Specification.AmbientIntensity);
        tglUniform1f(SpotLightsVarLocations->SpecLocation.SpecularIntensityLocation, CurrentSpotLight->Specification.SpecularIntensity);

        Rotation LightRotation = Lights->Rotation;

        vec3 Target, Right, Up;
        LightRotation.ToVec(Target, Up, Right);

        tglUniform3fv(SpotLightsVarLocations->DirectionLocation, 1, &Target[0]);
        tglUniform1f(SpotLightsVarLocations->CosCutoffAngleLocation, CurrentSpotLight->CosCutoffAngle);
        tglUniform1f(SpotLightsVarLocations->CutoffAttenuationFactorLocation, CurrentSpotLight->CutoffAttenuationFactor);
    }

    tglUniform1i(VarStorage->Light.SpotLightsAmountLocation, LightAmount);
}

struct ivec4 {
    i32 x, y, z, w;
};

struct uvec4 {
    union {
        struct {
            u32 x, y, z, w;
        };
        u32 ValueHolder[4];   
    };
};

struct u8vec4 {
    union {
        struct {
            u8 x, y, z, w;
        };
        u32 ValueHolder;   
    };
};

inline void Ufbxvec3Convert(ufbx_vec3 *From, vec3 *To)
{
    To->x = From->x;
    To->y = From->y;
    To->z = From->z;
}

struct glTF2Primitives {
    Material    MeshMaterial;
    vec3*       Positions;
    vec3*       Normals;
    vec2*       TextureCoord;
    vec4*       BoneWeights;
    ivec4*      BoneIds;
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
    Skinning*           Skelet;
    AnimationsArray*    Animations;
    glTF2Mesh           Meshes[MAX_MESHES];
    i32                 MeshesAmount;
};

void ReadJointNode(Skinning* Skin, cgltf_node* Joint, JointsInfo* ParentJoint, i32& Index, cgltf_node** RootJoints, i32 Len)
{
    if (!Joint) {
        return;
    }

    JointsInfo* CurrentJointInfo    = &Skin->Joints[Index];
    std::string BoneName            = Joint->name;

    cgltf_float*    Translation = Joint->translation;
    cgltf_float*    Scale       = Joint->scale;
    quat            Rotation    = Joint->rotation;

    mat4 InverseTranslationMatrix = {};
    mat4 InverseScaleMatrix       = {};
    mat4 InverseRotationMatrix    = {};
    mat4 ParentMatrix             = !ParentJoint ? Identity4 : ParentJoint->InverseBindMatrix;

    InverseTranslationFromArr(Translation, InverseTranslationMatrix);
    InverseScaleFromArr(Scale, InverseScaleMatrix);
    Rotation.UprightToObject(InverseRotationMatrix);

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
                            vec3 Elem = {};
                            cgltf_accessor_read_float(TransformAccessor, TransformIndex, Elem.vec, sizeof(Elem));

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

                            quat *Rot = &Transform->Transforms[TransformIndex].Rotation;
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
                            vec3 Elem = {};
                            cgltf_accessor_read_float(TransformAccessor, TransformIndex, Elem.vec, sizeof(Elem));

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

void glTFReadAnimations(const char* Path, AnimationsArray* AnimArray, Skinning* Skin)
{
    cgltf_data* Mesh = 0;

    glTFLoadFile(Path, &Mesh);

    cgltf_animation*    Animations      = Mesh->animations;
    i32                 AnimationsCount = Mesh->animations_count;

    glTFReadAnimations(Animations, AnimationsCount, *AnimArray, *Skin);

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

            vec3*    Positions      = (vec3*)   Platform->AllocMem(sizeof(*CurrentPrimitiveOut->Positions)      * DEFAULT_BUFFER_SIZE);
            vec3*    Normals        = (vec3*)   Platform->AllocMem(sizeof(*CurrentPrimitiveOut->Normals)        * DEFAULT_BUFFER_SIZE);
            vec2*    TextureCoords  = (vec2*)   Platform->AllocMem(sizeof(*CurrentPrimitiveOut->TextureCoord)   * DEFAULT_BUFFER_SIZE);
            ivec4*   BoneIDs        = (ivec4*)  Platform->AllocMem(sizeof(*CurrentPrimitiveOut->BoneIds)        * DEFAULT_BUFFER_SIZE);
            vec4*    BoneWeights    = (vec4*)   Platform->AllocMem(sizeof(*CurrentPrimitiveOut->BoneWeights)    * DEFAULT_BUFFER_SIZE);
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
        Skinning*   Skelet      = FileOut->Skelet;

        u32             JointsCount = (u32)CurrentSkin->joints_count;
        cgltf_node**    Joints      = CurrentSkin->joints;
        cgltf_node*     RootJoint   = Joints[0];

        Skelet->Joints = (JointsInfo*)Platform->AllocMem(sizeof(*Skelet->Joints) * JointsCount);

        i32 IndexCounter = 0;

        ReadJointNode(Skelet, *Joints, NULL, IndexCounter, Joints, (i32)JointsCount);

        FileOut->Skelet->JointsAmount = JointsCount;

        i32 AnimationsCount = (i32)Mesh->animations_count;

        Assert(AnimationsCount == 1);

        cgltf_animation*    Animations  = Mesh->animations;
        AnimationsArray*    AnimArray   = FileOut->Animations;

        glTFReadAnimations(Animations, AnimationsCount, *AnimArray, *Skelet);
    }

    cgltf_free(Mesh);
}

const char *ResourceFolderLocation = "data/obj/";

void InitSkeletalMeshComponent(Platform *Platform, GameContext* Cntx, SkeletalMeshComponent *SkeletalMesh,  DynamicSceneObjectLoader& Loader)
{
    char                FullFileName[500]   = {};
    glTF2File           LoadFile            = {};

    AnimationSystem& AnimSys = Cntx->AnimSystem;

    SkeletalComponent& NewComponent = AnimSys.RegisterNewSkin(SkeletalCharacters::CharacterPlayer);

    LoadFile.Animations = &NewComponent.Animations;
    LoadFile.Skelet     = &NewComponent.Skin;

    glTFRead(Loader.Mesh.Path, Platform, &LoadFile);

    i32 AnimCount = Loader.Amount;
    for (i32 AnimIndex = 0; AnimIndex < AnimCount; ++AnimIndex) {
        SerialaizedAnimations& SerialaizedAnims = Loader.Animations[AnimIndex];

        glTFReadAnimations(SerialaizedAnims.Path, LoadFile.Animations, LoadFile.Skelet);
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
    vec3*   Position        = (vec3*)Platform->AllocMem(sizeof(*Position) * MaxPositions);
    vec3*   Normals         = (vec3*)Platform->AllocMem(sizeof(*Normals) * MaxNormals);
    vec2*   TextureCoords   = (vec2*)Platform->AllocMem(sizeof(*TextureCoords) * MaxTexures);

    u32*    FlatIndices         = (u32*)Platform->AllocMem(sizeof(*Indices) * MaxIndices);
    vec3*   FlatPosition        = (vec3*)Platform->AllocMem(sizeof(*Position) * MaxPositions);
    vec3*   FlatNormals         = (vec3*)Platform->AllocMem(sizeof(*Normals) * MaxNormals);
    vec2*   FlatTextureCoords   = (vec2*)Platform->AllocMem(sizeof(*TextureCoords) * MaxTexures);
    ivec4*  FlatBoneIds         = (ivec4*)Platform->AllocMem(sizeof(*FlatBoneIds) * MaxTexures);
    vec4*   FlatBoneWeights     = (vec4*)Platform->AllocMem(sizeof(*FlatBoneWeights) * MaxTexures);

    u64 MaxMeshAmount = FbxScene->meshes.count;

    for (u64 MeshIndex = 0; MeshIndex < MaxMeshAmount; ++MeshIndex) {
        ufbx_mesh*  Mesh            = FbxScene->meshes[MeshIndex];
        u64         AmountOfIndices = Mesh->num_indices;

        for (u64 Index = 0; Index < AmountOfIndices; ++Index) {
            ufbx_vertex_vec3*   VertexPositon           = &Mesh->vertex_position;
            u32                 VertexPositionsIndex    = VertexPositon->indices[Index];

            Ufbxvec3Convert(&VertexPositon->values[VertexPositionsIndex], &FlatPosition[Index]);

            ufbx_vertex_vec3*   VertexNormal        = &Mesh->vertex_normal;
            u32                 VertexNormalIndex   = VertexNormal->indices[Index];

            Ufbxvec3Convert(&VertexNormal->values[VertexNormalIndex], &FlatNormals[Index]);

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
    vec3*   Position    = (vec3*)Platform->AllocMem(sizeof(*Position)   * 10000);
    vec3*   Normals     = (vec3*)Platform->AllocMem(sizeof(*Normals)    * 10000);
    ivec4*  BoneIDs     = (ivec4*)Platform->AllocMem(sizeof(*BoneIDs)   * 10000);
    vec4*   Weights     = (vec4*)Platform->AllocMem(sizeof(*Weights)    * 10000);

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

static void PrepareShadowPass(GameContext* Cntx)
{
    tglGenFramebuffers(1, &Cntx->DepthFbo);
    tglBindFramebuffer(GL_FRAMEBUFFER, Cntx->DepthFbo);
    
    glGenTextures(1, &Cntx->DepthTexture);
    glBindTexture(GL_TEXTURE_2D, Cntx->DepthTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, SHADOW_MAP_W, SHADOW_MAP_H, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    const real32 BlackBorderColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, BlackBorderColor);

    tglFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Cntx->DepthTexture, 0);
    
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    u32 Status = tglCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        Assert(false);
    }

    tglBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PrepareFrame(Platform *Platform, GameContext *Cntx)
{
    /*
    vec3 n = {
        0.0f,
        0.0f,
        1.0f
    };

    vec3 Pos = {
        1.0f,
        0.0f,
        0.0f
    };

    Quat a(DEGREE_TO_RAD(45.0f), n);
    Quat b(DEGREE_TO_RAD(135.0f), n);

    mat3 TestMat = {};

    vec3 Test1 = b * Pos;
    b.ToMat3(&TestMat);
    vec3 Test2 = TestMat * Pos;

    Quat DeltaQuat = Quat::Slerp(a, b, 1.0f);

    Quat RotQuat = (DeltaQuat * a);

    vec3 WooDooMagic = RotQuat * Pos;
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
    SceneMainLight->Rotation                           = { 90.0f, 45.0f, 0.0f };

    vec3        PointLightPosition  = { 20.0, 12.0f, 10.0f };
    vec3        PointLightColor     = { 1.0f, 0.3f, 0.3f };
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

        Object.ObjMesh.ObjectPath  = CurrentObject.Mesh.Path;

        InitSkeletalMeshComponent(Platform, Cntx, &Object.ObjMesh, CurrentObject);

        GltfFile fl;
        fl.Read(CurrentObject.Mesh.Path);

        Object.Transform.Rotation = { 0.0f, 0.0f, 0.0f };
        Object.Transform.Position = { 0.0f, 0.0f, Position };
        Object.Transform.Scale = { 0.04f, 0.04f, 0.04f };
        
        Position += 10.0f;
    }

    AnimationSystem& AnimSys = Cntx->AnimSystem;

    AnimationTrack& PlayerTrack = AnimSys.RegisterNewAnimationTrack();

    PlayerTrack.Id      = 0;
    PlayerTrack.SkinId  = SkeletalCharacters::CharacterPlayer;

    AnimationTask& PlayerWalkTask = PlayerTrack.AnimationTasks[0];
    PlayerWalkTask.MaxX         = 1.5f;
    PlayerWalkTask.Mode         = TaskMode::_1D;
    PlayerWalkTask.Loop         = 1;
    PlayerWalkTask.StackAmount  = 3;

    Animation* IdleAnim = AnimSys.GetAnimationById(SkeletalCharacters::CharacterPlayer, PLayerAnimations::IdleDynamic);
    AnimationStack& IdleStack = PlayerWalkTask.Stack[PLayerAnimations::IdleDynamic];
    IdleStack                   = {};
    IdleStack.Speed             = 1.0f;
    IdleStack.StackPositionX    = 0.0f;
    IdleStack.MaxDuration       = IdleAnim->MaxDuration;
    IdleStack.Animation         = IdleAnim;

    Animation* WalkAnim = AnimSys.GetAnimationById(SkeletalCharacters::CharacterPlayer, PLayerAnimations::WalkDefault);    
    AnimationStack& WalkStack = PlayerWalkTask.Stack[PLayerAnimations::WalkDefault];
    WalkStack                   = {};
    WalkStack.Speed             = 1.0f;
    WalkStack.StackPositionX    = 0.75f;
    WalkStack.MaxDuration       = WalkAnim->MaxDuration;
    WalkStack.Animation         = WalkAnim;

    Animation* RunAnim = AnimSys.GetAnimationById(SkeletalCharacters::CharacterPlayer, PLayerAnimations::RunDefault);    
    AnimationStack& RunStack = PlayerWalkTask.Stack[PLayerAnimations::RunDefault];
    RunStack                   = {};
    RunStack.Speed             = 1.0f;
    RunStack.StackPositionX    = 1.5f;
    RunStack.MaxDuration       = RunAnim->MaxDuration;
    RunStack.Animation         = RunAnim;

    PlayerTrack.AnimationTasksAmount = 1;

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

    Terrain& Terra = Cntx->Terrain;
    LoadTerrain(Platform, &Terra, "data/textures/grass_texture.jpg");

    Terra.AmbientColor    = { 0.6f, 0.6f, 0.6f };
    Terra.DiffuseColor    = { 0.8f, 0.8f, 0.8f };
    Terra.SpecularColor   = { 0.1f, 0.1f, 0.1f };

    PrepareShadowPass(Cntx);

    Cntx->TranslationDelta = 0.1f;
    Cntx->RotationDelta = 0.1f;
}

struct KeyframePair {
    i32 StartKeyframe;
    i32 EndKeyframe;
};

struct AnimationFrameTransform {
    vec3 Scale;
    quat Rotation;
    vec3 Translation;
};

static AnimationFrame* FindFrame(AnimationFrame* Frames, i32 FramesAmount, i32 BoneID)
{
    AnimationFrame* Result = 0;

    for (i32 FrameIndex = 0; FrameIndex < FramesAmount; ++FrameIndex) {
        AnimationFrame* Tmp = &Frames[FrameIndex];

        if (Tmp->Target == BoneID) {
            Result = Tmp;

            break;
        }
    }

    return Result;
}

static KeyframePair FindKeyframe(real32* Keyframes, i32 KeyframesAmount, real32 CurrentTime)
{
    KeyframePair Result = { };

    real32 ZeroKeyframe = Keyframes[0];
    if (ZeroKeyframe > CurrentTime) {
        Result.StartKeyframe = 0;
        Result.EndKeyframe = 1;

        return Result;
    }

    for (i32 KeyframeIndex = 0; KeyframeIndex < KeyframesAmount; ++KeyframeIndex) {
        Assert(KeyframeIndex < (KeyframesAmount - 1));

        real32 FrKeyframe = Keyframes[KeyframeIndex];
        real32 ScKeyframe = Keyframes[KeyframeIndex + 1];
        if (FrKeyframe <= CurrentTime && CurrentTime <= ScKeyframe) {
            Result.StartKeyframe    = KeyframeIndex;
            Result.EndKeyframe      = KeyframeIndex + 1;

            return Result;
        }
    }

    Assert(false);
}

static inline void HandleTranslationInterpolation(AnimationTransformation& Transform, real32 CurrentTime, vec3& FinalTranslation)
{
    Assert(Transform.Valid);

    real32* Keyframes = Transform.Keyframes;

    KeyframePair FoundKeyframes = FindKeyframe(Keyframes, Transform.Amount, CurrentTime);

    i32                     StartKeyframe               = FoundKeyframes.StartKeyframe;
    i32                     EndKeyframe                 = FoundKeyframes.EndKeyframe;
    TransformationStorage&  StartKeyframeTramsformation = Transform.Transforms[StartKeyframe];
    TransformationStorage&  EndKeyframeTramsformation   = Transform.Transforms[EndKeyframe];
    vec3&                   StartTranslation            = StartKeyframeTramsformation.Translation;
    vec3&                   EndTranslation              = EndKeyframeTramsformation.Translation;

    if (Transform.IType == InterpolationType::IStep) {
        FinalTranslation = StartTranslation;

        return;
    }

    real32 T = CalcT(CurrentTime, Keyframes[StartKeyframe], Keyframes[EndKeyframe]);

    vec3 DeltaTranslation = vec3::Lerp(StartTranslation, EndTranslation, T);

    FinalTranslation = StartTranslation + DeltaTranslation;
}

static inline void HandleRotationInterpolation(AnimationTransformation& Transform, real32 CurrentTime, quat& FinalRotation)
{
    Assert(Transform.Valid);

    real32* Keyframes = Transform.Keyframes;

    KeyframePair FoundKeyframes = FindKeyframe(Keyframes, Transform.Amount, CurrentTime);

    i32                     StartKeyframe               = FoundKeyframes.StartKeyframe;
    i32                     EndKeyframe                 = FoundKeyframes.EndKeyframe;
    TransformationStorage&  StartKeyframeTramsformation = Transform.Transforms[StartKeyframe];
    TransformationStorage&  EndKeyframeTramsformation   = Transform.Transforms[EndKeyframe];

    if (Transform.IType == InterpolationType::IStep) {
        FinalRotation = StartKeyframeTramsformation.Rotation;

        return;
    }

    real32 T = CalcT(CurrentTime, Keyframes[StartKeyframe], Keyframes[EndKeyframe]);

    FinalRotation = quat::Slerp(StartKeyframeTramsformation.Rotation, EndKeyframeTramsformation.Rotation, T);
}

static inline void HandleScaleInterpolation(AnimationTransformation& Transform, real32 CurrentTime, vec3& FinalScale)
{
    Assert(Transform.Valid);

    real32* Keyframes = Transform.Keyframes;

    KeyframePair FoundKeyframes = FindKeyframe(Keyframes, Transform.Amount, CurrentTime);

    i32                     StartKeyframe               = FoundKeyframes.StartKeyframe;
    i32                     EndKeyframe                 = FoundKeyframes.EndKeyframe;
    TransformationStorage&  StartKeyframeTramsformation = Transform.Transforms[StartKeyframe];
    TransformationStorage&  EndKeyframeTramsformation   = Transform.Transforms[EndKeyframe];
    vec3&                   StartScale                  = StartKeyframeTramsformation.Scale;
    vec3&                   EndScale                    = EndKeyframeTramsformation.Scale;

    if (Transform.IType == InterpolationType::IStep) {
        FinalScale = StartKeyframeTramsformation.Scale;

        return;
    }

    real32 T = CalcT(CurrentTime, Keyframes[StartKeyframe], Keyframes[EndKeyframe]);

    vec3 DeltaScale = vec3::Lerp(StartScale, EndScale, T);

    FinalScale = StartScale + DeltaScale;
}

static inline void CalculateAnimationTransform(AnimationTransformation* FrameTransform, real32 CurrentTime, AnimationFrameTransform& FinalTransform)
{
    AnimationTransformation& ScaleTransform = FrameTransform[AnimationType::AScale];
    HandleScaleInterpolation(ScaleTransform, CurrentTime, FinalTransform.Scale);

    AnimationTransformation& RotationTransform = FrameTransform[AnimationType::ARotation];
    HandleRotationInterpolation(RotationTransform, CurrentTime, FinalTransform.Rotation);

    AnimationTransformation& TranslationTransform = FrameTransform[AnimationType::ATranslation];
    HandleTranslationInterpolation(TranslationTransform, CurrentTime, FinalTransform.Translation);
}

static void Calc1DTask(std::map<std::string, BoneIDs>& Bones, mat4* Matrices, Animation& FrAnim, Animation& ScAnim, JointsInfo* Joint, mat4* Parent, real32 FrAnimTime, real32 ScAnimTime, real32 BlendingFactor)
{
    BoneIDs& Ids = Bones[Joint->BoneName];

    AnimationFrame* FrAnimFrame = FindFrame(FrAnim.PerBonesFrame, FrAnim.FramesAmount, Ids.BoneID);
    AnimationFrame* ScAnimFrame = FindFrame(ScAnim.PerBonesFrame, ScAnim.FramesAmount, Ids.BoneID);

    mat4 ParentMat  = Parent ? *Parent : Identity4;
    mat4 CurrentJointMat  = Identity4;

    if (FrAnimFrame && ScAnimFrame) {
        AnimationFrameTransform FrAnimCurrentFrameTransform;
        CalculateAnimationTransform(FrAnimFrame->Transformations, FrAnimTime, FrAnimCurrentFrameTransform);

        AnimationFrameTransform ScAnimCurrentFrameTransform;
        CalculateAnimationTransform(ScAnimFrame->Transformations, ScAnimTime, ScAnimCurrentFrameTransform);

        vec3& FirstAnimScale    = FrAnimCurrentFrameTransform.Scale;
        vec3& SecondAnimScale   = ScAnimCurrentFrameTransform.Scale;

        vec3 DeltaScale     = vec3::Lerp(FirstAnimScale, SecondAnimScale, BlendingFactor);
        vec3 BlendedScale   = FirstAnimScale + DeltaScale;

        quat& FirstAnimRotation     = FrAnimCurrentFrameTransform.Rotation;
        quat& SecondAnimRotation    = ScAnimCurrentFrameTransform.Rotation;

        quat BlendedRotation = quat::Slerp(FirstAnimRotation, SecondAnimRotation, BlendingFactor);

        vec3& FirstAnimTranslation  = FrAnimCurrentFrameTransform.Translation;
        vec3& SecondAnimTranslation = ScAnimCurrentFrameTransform.Translation;

        vec3 DeltaTranslation   = vec3::Lerp(FirstAnimTranslation, SecondAnimTranslation, BlendingFactor);
        vec3 BlendedTranslation = FirstAnimTranslation + DeltaTranslation;

        mat4 ScaleMat         = {};
        mat4 RotationMat      = {};
        mat4 TranslationMat   = {};

        ScaleFromVec(BlendedScale, ScaleMat);
        BlendedRotation.Mat4(RotationMat);
        TranslationFromVec(BlendedTranslation, TranslationMat);

        CurrentJointMat = TranslationMat * RotationMat * ScaleMat;
    }
    else {
        Assert(false);
    }

    mat4 ExportMat = ParentMat * CurrentJointMat;
    Matrices[Ids.OriginalBoneID] = ExportMat;

    i32 ChildrenAmount = Joint->ChildrenAmount;
    for (i32 ChildrenIndex = 0; ChildrenIndex < ChildrenAmount; ++ChildrenIndex) {
        Calc1DTask(Bones, Matrices, FrAnim, ScAnim, Joint->Children[ChildrenIndex], &ExportMat, FrAnimTime, ScAnimTime, BlendingFactor);
    }
}

static void CalcClipTask(std::map<std::string, BoneIDs>& Bones, mat4* Matrices, Animation& Anim, JointsInfo* Joint, mat4* ParentMat, real32 CurrentTime)
{
    BoneIDs& Ids = Bones[Joint->BoneName];

    AnimationFrame* CurrentFrame = FindFrame(Anim.PerBonesFrame, Anim.FramesAmount, Ids.BoneID);

    mat4 Parent           = ParentMat ? *ParentMat : Identity4;
    mat4 CurrentJointMat  = Identity4;

    if (CurrentFrame) {
        AnimationFrameTransform CurrentFrameTransform;
        CalculateAnimationTransform(CurrentFrame->Transformations, CurrentTime, CurrentFrameTransform);

        mat4 ScaleMat         = {};
        mat4 RotationMat      = {};
        mat4 TranslationMat   = {};

        ScaleFromVec(CurrentFrameTransform.Scale, ScaleMat);
        CurrentFrameTransform.Rotation.Mat4(RotationMat);
        TranslationFromVec(CurrentFrameTransform.Translation, TranslationMat);

        CurrentJointMat = TranslationMat * RotationMat * ScaleMat;
    }
    else {
        Assert(false);
    }

    mat4 ExportMat = Parent * CurrentJointMat;
    Matrices[Ids.OriginalBoneID] = ExportMat;

    i32 ChildrenAmount = Joint->ChildrenAmount;
    for (i32 ChildrenIndex = 0; ChildrenIndex < ChildrenAmount; ++ChildrenIndex) {
        CalcClipTask(Bones, Matrices, Anim, Joint->Children[ChildrenIndex], &ExportMat, CurrentTime);
    }
}

void AnimationSystem::PrepareSkinMatrices(AnimationTrack& Track, i32 TaskId, real32 x, real32 y, real32 dt)
{
    Assert(Track.AnimationTasksAmount > TaskId);

    SkeletalComponent& SkinData = SkinningData[Track.SkinId];

    AnimationTask& Task = Track.AnimationTasks[TaskId];

    Skinning& Skin = SkinData.Skin;

    TaskMode Mode = Task.Mode;

    switch(Mode) {
        
        case TaskMode::Clip: {
            AnimationStack& Stack       = Task.Stack[0];

            Assert(Task.StackAmount == 1); // NOTE(ismail): ok chel

            real32 AdvancedTime   = Stack.CurrentTime + dt;
            real32 MaxDuration    = Stack.MaxDuration;

            if (Task.Loop) {
                Stack.CurrentTime = fmodf(AdvancedTime, MaxDuration);
            }
            else {
                Stack.CurrentTime = AdvancedTime > MaxDuration ? MaxDuration : AdvancedTime;
            }

            Animation& AnimToPlay = *Stack.Animation;

            real32 CurrentTime = Stack.CurrentTime;

            SkinningMatricesStorage& MatStorage = Track.Matrices;

            JointsInfo* RootJoint = &Skin.Joints[0];

            CalcClipTask(Skin.Bones, MatStorage.Matrices, AnimToPlay, RootJoint, 0, CurrentTime);

            MatStorage.Amount = Skin.JointsAmount;

        } break;

        case TaskMode::_1D: {
            AnimationStack* Stack       = Task.Stack;
            AnimationStack* FrStackNode = 0;
            AnimationStack* ScStackNode = 0;

            Assert(Task.StackAmount > 1); // NOTE(ismail): If you need to play less than one animation use TaskMode::Clip
            
            for (i32 FirstEntry = 0, SecondEntry = 1;;) {
                FrStackNode = &Stack[FirstEntry];
                ScStackNode = &Stack[SecondEntry];

                if (FrStackNode->StackPositionX <= x && ScStackNode->StackPositionX >= x) {
                    break;
                }
                else {
                    FirstEntry = SecondEntry;
                    SecondEntry = FirstEntry + 1;
                }
            }

            real32 FrAdvancedTime   = FrStackNode->CurrentTime + dt;
            real32 ScAdvancedTime   = ScStackNode->CurrentTime + dt;
            real32 FrMaxDuration    = FrStackNode->MaxDuration;
            real32 ScMaxDuration    = ScStackNode->MaxDuration;

            if (Task.Loop) {
                FrStackNode->CurrentTime = fmodf(FrAdvancedTime, FrMaxDuration);
                ScStackNode->CurrentTime = fmodf(ScAdvancedTime, ScMaxDuration);
            }
            else {
                FrStackNode->CurrentTime = FrAdvancedTime > FrMaxDuration ? FrMaxDuration : FrAdvancedTime;
                ScStackNode->CurrentTime = ScAdvancedTime > ScMaxDuration ? ScMaxDuration : ScAdvancedTime;
            }

            real32 FrPos = FrStackNode->StackPositionX;
            real32 ScPos = ScStackNode->StackPositionX;

            real32 BlendingFactor = (x - FrPos) / (ScPos - FrPos);

            Animation& FrAnim = *FrStackNode->Animation;
            Animation& ScAnim = *ScStackNode->Animation;

            real32 FrCurrentTime = FrStackNode->CurrentTime;
            real32 ScCurrentTime = ScStackNode->CurrentTime;

            SkinningMatricesStorage& MatStorage = Track.Matrices;

            JointsInfo* RootJoint = &Skin.Joints[0];

            Calc1DTask(Skin.Bones, MatStorage.Matrices, FrAnim, ScAnim, RootJoint, 0, FrCurrentTime, ScCurrentTime, BlendingFactor);

            MatStorage.Amount = Skin.JointsAmount;
        } break;

        case TaskMode::_2D: {
            Assert(false); // TODO(ismail): implement 2D blending
        } break;
    }
}

void AnimationSystem::Play(i32 CharId, i32 AnimTaskId, real32 x, real32 y, real32 dt)
{
    for (AnimationTrack& AnimTrack : CharactersAnimationTrack) {
        if (CharId == AnimTrack.Id) {
            PrepareSkinMatrices(AnimTrack, AnimTaskId, x, y, dt);

            break;
        }
    }
}

void AnimationSystem::ExportToRender(SkinningMatricesStorage& Result, i32 CharId)
{
    for (AnimationTrack& CharAnimTrack : CharactersAnimationTrack) {
        if (CharAnimTrack.Id == CharId) {
            Skinning&                       Skin            = SkinningData[CharAnimTrack.SkinId].Skin;
            i32                             JointsAmount    = Skin.JointsAmount;
            std::map<std::string, BoneIDs>& Bones           = Skin.Bones;
            
            mat4* OriginMatrices = CharAnimTrack.Matrices.Matrices;
            mat4* ResultMatrices = Result.Matrices;
            
            Result.Amount = JointsAmount;

            for (i32 i = 0; i < JointsAmount; ++i) {
                JointsInfo *JointInfo = &Skin.Joints[i];
            
                BoneIDs& Ids = Bones[JointInfo->BoneName];
            
                const mat4&   CurrentOriginMat = OriginMatrices[Ids.OriginalBoneID];
                mat4&         CurrentResultMat = ResultMatrices[Ids.OriginalBoneID];
            
                CurrentResultMat = CurrentOriginMat * JointInfo->InverseBindMatrix;
            }

            return;
        }
    }

    Assert(false); // NOTE(ismail): we must not reache this line

}

mat4& AnimationSystem::GetBoneLocation(i32 CharId, const std::string& BoneName)
{
    for (AnimationTrack& Track : CharactersAnimationTrack) {
        if (Track.Id == CharId) {
            SkeletalComponent& SkinData = SkinningData[Track.SkinId];

            BoneIDs& IDs = SkinData.Skin.Bones[BoneName];

            return Track.Matrices.Matrices[IDs.OriginalBoneID];
        }
    }

    Assert(false); // NOTE(ismail): we must not reache this line
}

mat4& AnimationSystem::GetBoneLocation(i32 CharId, i32 BoneId)
{
    for (AnimationTrack& Track : CharactersAnimationTrack) {
        if (Track.Id == CharId) {
            Assert(Track.Matrices.Amount > BoneId);
            return Track.Matrices.Matrices[BoneId];
        }
    }

    Assert(false); // NOTE(ismail): we must not reache this line
}

static void SetupObjectRendering(GameContext* Ctx, FrameData* Data, WorldTransform& ObjectTransform,
                                 ShaderProgramVariablesStorage*  VarStorage, ObjectNesting& Nesting, 
                                 ShaderProgramsType ShaderType)
{
    mat4    ObjectToCameraSpaceTransformation   = {};
    mat4    ObjectGeneralTransformation         = {};
    mat4    ObjectTranslation                   = {};
    mat4    ObjectRotation                      = {};
    mat4    ObjectScale                         = {};
    vec3    CameraPosition                      = Ctx->PlayerCamera.Transform.Position;
    vec3    ObjectPosition;

    TranslationFromVec(ObjectTransform.Position, ObjectTranslation);
    ObjectTransform.Rotation.ObjectToUpright(ObjectRotation);

    if (Nesting.Parent) {
        DynamicSceneObject*             ParentObject    = Nesting.Parent;
        WorldTransform&                 ParentTransform = ParentObject->Transform;
        std::map<std::string, BoneIDs>& Bones           = ParentObject->ObjMesh.Skelet.Skin.Bones;

        mat4 RootToWorldTranslation   = {};
        mat4 RootToWorldRotation      = {};
        mat4 RootToWorldScale         = {};

        TranslationFromVec(ParentTransform.Position, RootToWorldTranslation);
        ParentTransform.Rotation.ObjectToUpright(RootToWorldRotation);
        ScaleFromVec(ParentTransform.Scale, RootToWorldScale);

        ScaleFromVecRelative(ObjectTransform.Scale, ParentTransform.Scale, ObjectScale);

        mat4& AttachedBoneMat = Ctx->AnimSystem.GetBoneLocation(0, Nesting.AttachedToBone);

        ObjectToCameraSpaceTransformation   = Data->CameraTransformation * RootToWorldTranslation;
        ObjectGeneralTransformation         = RootToWorldRotation * RootToWorldScale * AttachedBoneMat * 
                                              ObjectTranslation * ObjectRotation * ObjectScale;
                                        
        ObjectPosition = ParentTransform.Position;
    }
    else {
        ScaleFromVec(ObjectTransform.Scale, ObjectScale);

        ObjectToCameraSpaceTransformation   = Data->CameraTransformation * ObjectTranslation;
        ObjectGeneralTransformation         = ObjectRotation * ObjectScale;

        ObjectPosition = ObjectTransform.Position;
    }

    tglUniformMatrix4fv(VarStorage->Transform.ObjectToCameraSpaceTransformationLocation, 1, GL_TRUE, ObjectToCameraSpaceTransformation[0]);
    tglUniformMatrix4fv(VarStorage->Transform.ObjectGeneralTransformationLocation, 1, GL_TRUE, ObjectGeneralTransformation[0]);

    vec3 CameraPositionInObjectUprightSpace = CameraPosition - ObjectPosition;
    tglUniform3fv(VarStorage->Light.ViewerPositionLocation, 1, &CameraPositionInObjectUprightSpace[0]);

    SetupPointLights(Ctx->PointLights, MAX_POINTS_LIGHTS, ShaderType, &ObjectPosition);
    SetupSpotLights(Ctx->SpotLights, MAX_SPOT_LIGHTS, ShaderType, &ObjectPosition);
}

static void SetupObjects(AnimationSystem& AnimSys, WorldTransform& ObjectTransform, ObjectNesting& Nesting, FrameDataStorage& FrameStorage)
{
    mat4  RootTransformation                  = Identity4; 
    mat4  ObjectTranslation                   = {};
    mat4  ObjectRotation                      = {};
    mat4  ObjectScale                         = {};

    TranslationFromVec(ObjectTransform.Position, ObjectTranslation);
    ObjectTransform.Rotation.ObjectToUpright(ObjectRotation);

    if (Nesting.Parent) {
        DynamicSceneObject*             ParentObject    = Nesting.Parent;
        WorldTransform&                 ParentTransform = ParentObject->Transform;
        std::map<std::string, BoneIDs>& Bones           = ParentObject->ObjMesh.Skelet.Skin.Bones;

        mat4 RootToWorldTranslation   = {};
        mat4 RootToWorldRotation      = {};
        mat4 RootToWorldScale         = {};

        TranslationFromVec(ParentTransform.Position, RootToWorldTranslation);
        ParentTransform.Rotation.ObjectToUpright(RootToWorldRotation);
        ScaleFromVec(ParentTransform.Scale, RootToWorldScale);

        ScaleFromVecRelative(ObjectTransform.Scale, ParentTransform.Scale, ObjectScale);

        mat4& AttachedBoneMat = AnimSys.GetBoneLocation(0, Nesting.AttachedToBone);

        FrameStorage.ObjectToWorldTranslation       = RootToWorldTranslation;
        FrameStorage.ObjectGeneralTransformation    = RootToWorldRotation * RootToWorldScale * AttachedBoneMat * 
                                                      ObjectTranslation * ObjectRotation * ObjectScale;
                                        
        FrameStorage.ObjectPosition = ParentTransform.Position;
    }
    else {
        ScaleFromVec(ObjectTransform.Scale, ObjectScale);

        FrameStorage.ObjectToWorldTranslation       = ObjectTranslation;
        FrameStorage.ObjectGeneralTransformation    = ObjectRotation * ObjectScale;

        FrameStorage.ObjectPosition = ObjectTransform.Position;
    }
}

static void DrawSkeletalMesh(Platform *Platform, GameContext *Cntx, FrameData *Data)
{
    ShaderProgram* Shader = &ShadersProgramsCache[ShaderProgramsType::SkeletalMeshShader];
    ShaderProgramVariablesStorage* VarStorage = &Shader->ProgramVarsStorage;
    
    tglUseProgram(Shader->Program);

    SetupDirectionalLight(&Cntx->LightSource, ShaderProgramsType::SkeletalMeshShader);

    for (i32 Index = 0; Index < DYNAMIC_SCENE_OBJECTS_MAX; ++Index) {
        DynamicSceneObject*     CurrentSceneObject  = &Cntx->TestDynamocSceneObjects[Index];
        SkeletalMeshComponent*  Comp                = &CurrentSceneObject->ObjMesh;
        WorldTransform*         Transform           = &CurrentSceneObject->Transform;
        SkeletalComponent*      Skin                = &CurrentSceneObject->ObjMesh.Skelet;

        if (Platform->Input.EButton.State == KeyState::Pressed && !Cntx->EWasPressed) {;
            Cntx->EWasPressed = 1;
        }
        else if (Platform->Input.EButton.State == KeyState::Released) {
            Cntx->EWasPressed = 0;
        }

        SkinningMatricesStorage& SkinMatrices = Data->TestDynamocSceneObjectsFrameStorage[Index].SkinFrameStorage;
        Cntx->AnimSystem.ExportToRender(SkinMatrices, Index);
        ShaderProgramVariablesStorage::AnimationInfo&   AnimVar     = VarStorage->Animation;
        for (i32 MatrixIndex = 0; MatrixIndex < SkinMatrices.Amount; ++MatrixIndex) {
            const mat4* Mat = &SkinMatrices.Matrices[MatrixIndex];

            tglUniformMatrix4fv(AnimVar.AnimationMatricesLocation[MatrixIndex], 1, GL_TRUE, (*Mat)[0]);
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

        Transform->Rotation.h += Cntx->RotationDelta;

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

static void PrecalculateObjects(GameContext* Cntx)
{
    FrameData&          FrameData   = Cntx->FrameDt;
    AnimationSystem&    AnimSystem  = Cntx->AnimSystem;

    i32 Index = 0;
    for (; Index < DYNAMIC_SCENE_OBJECTS_MAX; ++Index) {
        FrameDataStorage&   CurrentObjectTransforms = FrameData.TestDynamocSceneObjectsFrameStorage[Index];
        DynamicSceneObject& CurrentSceneObject      = Cntx->TestDynamocSceneObjects[Index];

        WorldTransform& Transform   = CurrentSceneObject.Transform;
        ObjectNesting&  Nesting     = CurrentSceneObject.Nesting;

        SkinningMatricesStorage& SkinMatricesStore = CurrentObjectTransforms.SkinFrameStorage;

        AnimSystem.ExportToRender(SkinMatricesStore, Index);

        SetupObjects(AnimSystem, Transform, Nesting, CurrentObjectTransforms);
    }
    FrameData.TestDynamocSceneObjectsAmount = Index;

    for (Index = 0; Index < SCENE_OBJECTS_MAX; ++Index) {
        FrameDataStorage&   CurrentObjectTransforms = FrameData.TestSceneObjectsFrameStorage[Index];
        SceneObject&        CurrentSceneObject      = Cntx->TestSceneObjects[Index];

        WorldTransform& Transform   = CurrentSceneObject.Transform;
        ObjectNesting&  Nesting     = CurrentSceneObject.Nesting;

        Transform.Rotation.h += Cntx->RotationDelta;

        SetupObjects(AnimSystem, Transform, Nesting, CurrentObjectTransforms);
    }
    FrameData.TestSceneObjectsAmount = Index;

    Terrain&            Terra               = Cntx->Terrain;
    FrameDataStorage&   TerrainDataStorage  = FrameData.TerrainFrameDataStorage;

    mat4 TerrainWorldTranslation, TerrainWorldRotation, TerrainWorldScale;
    TranslationFromVec(Terra.Transform.Position, TerrainWorldTranslation);
    Terra.Transform.Rotation.ObjectToUpright(TerrainWorldRotation);
    ScaleFromVec(Terra.Transform.Scale, TerrainWorldScale);

    TerrainDataStorage.ObjectToWorldTranslation     = TerrainWorldTranslation;
    TerrainDataStorage.ObjectGeneralTransformation  = TerrainWorldRotation * TerrainWorldScale;
    TerrainDataStorage.ObjectPosition               = Terra.Transform.Position;
}

static void ShadowPass(GameContext* Cntx)
{
    FrameData&                                      FrameData                   = Cntx->FrameDt;
    ShaderProgram*                                  Shader                      = &ShadersProgramsCache[ShaderProgramsType::DepthTestShader];
    ShaderProgramVariablesStorage*                  VarStorage                  = &Shader->ProgramVarsStorage;
    ShaderProgramVariablesStorage::ObjectTransform& ShaderTransformsLocation    = VarStorage->Transform;

    tglBindFramebuffer(GL_FRAMEBUFFER, Cntx->DepthFbo);

    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, SHADOW_MAP_W, SHADOW_MAP_H);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 2.0f);

    tglUseProgram(Shader->Program);

    DirectionalLight& DirLight = Cntx->LightSource;

    mat4 CameraOrthoProjection = {};
    MakeOrthoProjection(CameraOrthoProjection, 40.0f, -40.0f, 40.0f, -40.0f, 40.0f, -40.0f);

    mat4 CameraSpaceRotation, CameraSpaceTranslation;
    DirLight.Rotation.UprightToObject(CameraSpaceRotation);

    vec3 CameraSpaceTranslationVector = { -15.0f, 0.0f, 25.0f };
    InverseTranslationFromVec(CameraSpaceTranslationVector, CameraSpaceTranslation);

    mat4 CameraTransformation                 = CameraOrthoProjection * CameraSpaceRotation * CameraSpaceTranslation;
    FrameData.ShadowPassCameraTransformation    = CameraTransformation;

    tglUniform1i(VarStorage->Animation.HaveSkinMatricesLocation, 0);

    i32 StaticSceneObjectsAmount = FrameData.TestSceneObjectsAmount;
    for (i32 Index = 0; Index < StaticSceneObjectsAmount; ++Index) {
        FrameDataStorage&   CurrentObjectTransforms = FrameData.TestSceneObjectsFrameStorage[Index];
        SceneObject*        CurrentSceneObject      = &Cntx->TestSceneObjects[Index];
        MeshComponent*      Comp                    = &CurrentSceneObject->ObjMesh;

        mat4 ObjectToCameraSpaceTransform = CameraTransformation * CurrentObjectTransforms.ObjectToWorldTranslation;

        CurrentObjectTransforms.ShadowPassObjectMatrices = ObjectToCameraSpaceTransform * CurrentObjectTransforms.ObjectGeneralTransformation;

        tglUniformMatrix4fv(ShaderTransformsLocation.ObjectToCameraSpaceTransformationLocation, 1, GL_TRUE, ObjectToCameraSpaceTransform[0]);
        tglUniformMatrix4fv(ShaderTransformsLocation.ObjectGeneralTransformationLocation, 1, GL_TRUE, CurrentObjectTransforms.ObjectGeneralTransformation[0]);

        tglBindVertexArray(Comp->BuffersHandler[OpenGLBuffersLocation::GLVertexArrayLocation]);
        for (i32 Index = 0; Index < Comp->MeshesAmount; ++Index) {
            MeshComponentObjects*   MeshInfo        = &Comp->MeshesInfo[Index];
        
            tglDrawElementsBaseVertex(GL_TRIANGLES, MeshInfo->NumIndices, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * MeshInfo->IndexOffset), MeshInfo->VertexOffset);
        }
    }

    tglUniform1i(VarStorage->Animation.HaveSkinMatricesLocation, 1);

    i32     DynamicSceneObjectsAmount   = FrameData.TestDynamocSceneObjectsAmount;
    i32*    SkinMatricesLocation        = VarStorage->Animation.AnimationMatricesLocation;
    for (i32 Index = 0; Index < DynamicSceneObjectsAmount; ++Index) {
        FrameDataStorage&       CurrentObjectTransforms = FrameData.TestDynamocSceneObjectsFrameStorage[Index];
        DynamicSceneObject*     CurrentSceneObject      = &Cntx->TestDynamocSceneObjects[Index];
        SkeletalMeshComponent*  Comp                    = &CurrentSceneObject->ObjMesh;

        i32     AmountSkinMatrices  = CurrentObjectTransforms.SkinFrameStorage.Amount;
        mat4* SkinMatrices        = CurrentObjectTransforms.SkinFrameStorage.Matrices;
        for (i32 MatrixIndex = 0; MatrixIndex < AmountSkinMatrices; ++MatrixIndex) {
            const mat4& Mat = SkinMatrices[MatrixIndex];

            tglUniformMatrix4fv(SkinMatricesLocation[MatrixIndex], 1, GL_TRUE, Mat[0]);
        }

        mat4 ObjectToCameraSpaceTransform = CameraTransformation * CurrentObjectTransforms.ObjectToWorldTranslation;
        CurrentObjectTransforms.ShadowPassObjectMatrices = ObjectToCameraSpaceTransform * CurrentObjectTransforms.ObjectGeneralTransformation;

        tglUniformMatrix4fv(ShaderTransformsLocation.ObjectToCameraSpaceTransformationLocation, 1, GL_TRUE, ObjectToCameraSpaceTransform[0]);
        tglUniformMatrix4fv(ShaderTransformsLocation.ObjectGeneralTransformationLocation, 1, GL_TRUE, CurrentObjectTransforms.ObjectGeneralTransformation[0]);

        i32             PrimitivesAmount    = Comp->PrimitivesAmount;
        MeshPrimitives* Primitives          = Comp->Primitives;
        for (i32 PrimitiveIndex = 0; PrimitiveIndex < PrimitivesAmount; ++PrimitiveIndex) {
            MeshPrimitives* Primitive   = &Primitives[PrimitiveIndex];

            tglBindVertexArray(Primitive->BuffersHandler[OpenGLBuffersLocation::GLVertexArrayLocation]);

            tglDrawElements(GL_TRIANGLES, Primitive->InidicesAmount, GL_UNSIGNED_INT, 0);
        }
    }

    Terrain&            Terra               = Cntx->Terrain;
    FrameDataStorage&   TerrainDataStorage  = FrameData.TerrainFrameDataStorage;

    tglUniform1i(VarStorage->Animation.HaveSkinMatricesLocation, 0);

    mat4 ObjectToCameraSpaceTransform = CameraTransformation * TerrainDataStorage.ObjectToWorldTranslation;
    TerrainDataStorage.ShadowPassObjectMatrices = ObjectToCameraSpaceTransform * TerrainDataStorage.ObjectGeneralTransformation;

    tglUniformMatrix4fv(VarStorage->Transform.ObjectToCameraSpaceTransformationLocation, 1, GL_TRUE, ObjectToCameraSpaceTransform[0]);
    tglUniformMatrix4fv(VarStorage->Transform.ObjectGeneralTransformationLocation, 1, GL_TRUE, TerrainDataStorage.ObjectGeneralTransformation[0]);

    tglBindVertexArray(Terra.BuffersHandler[OpenGLBuffersLocation::GLVertexArrayLocation]);

    tglDrawElements(GL_TRIANGLES, Terra.IndicesAmount, GL_UNSIGNED_INT, 0);

    glDisable(GL_POLYGON_OFFSET_FILL);

    tglBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void DrawPass(Platform* Platform, GameContext* Cntx)
{
    glViewport(0, 0, Platform->ScreenOpt.ActualWidth, Platform->ScreenOpt.ActualHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ShaderProgram*                                  Shader          = &ShadersProgramsCache[ShaderProgramsType::SkeletalMeshShader];
    ShaderProgramVariablesStorage*                  VarStorage      = &Shader->ProgramVarsStorage;
    ShaderProgramVariablesStorage::ObjectTransform* ObjTransform    = &VarStorage->Transform;
    FrameData&                                      FrameData       = Cntx->FrameDt;

    DirectionalLight&   MainLightSource = Cntx->LightSource;
    PointLight*         PointLights     = Cntx->PointLights;
    SpotLight*          SpotLights      = Cntx->SpotLights;

    mat4& CameraTransformation    = FrameData.CameraTransformation;
    vec3&   CameraPosition          = FrameData.CameraPosition;

    tglUseProgram(Shader->Program);

    tglActiveTexture(VarStorage->Shadow.ShadowMapTexture.Unit);
    glBindTexture(GL_TEXTURE_2D, Cntx->DepthTexture);

    SetupDirectionalLight(&MainLightSource, ShaderProgramsType::SkeletalMeshShader);

    i32                 DynamicSceneObjectsAmount   = FrameData.TestDynamocSceneObjectsAmount;
    FrameDataStorage*   DynamicObjectsFrameData     = FrameData.TestDynamocSceneObjectsFrameStorage;
    DynamicSceneObject* DynamicObjects              = Cntx->TestDynamocSceneObjects;
    for (i32 Index = 0; Index < DynamicSceneObjectsAmount; ++Index) {
        DynamicSceneObject&     Object              = DynamicObjects[Index];
        FrameDataStorage&       ObjectDataStorage   = DynamicObjectsFrameData[Index];

        const SkinningMatricesStorage&                  Matrices                = ObjectDataStorage.SkinFrameStorage;
        ShaderProgramVariablesStorage::AnimationInfo&   AnimVar                 = VarStorage->Animation;
        i32*                                            AnimMatricesLocations   = AnimVar.AnimationMatricesLocation;
        i32                                             SkinMatricesAmount      = Matrices.Amount;
        const mat4*                                   SkinMatrices            = Matrices.Matrices;
        for (i32 MatrixIndex = 0; MatrixIndex < SkinMatricesAmount; ++MatrixIndex) {
            const mat4& Mat = SkinMatrices[MatrixIndex];

            tglUniformMatrix4fv(AnimMatricesLocations[MatrixIndex], 1, GL_TRUE, Mat[0]);
        }

        mat4 ObjectToCameraSpaceTransformation = CameraTransformation * ObjectDataStorage.ObjectToWorldTranslation;
        mat4 ObjectToLightSpaceTransformation = FrameData.ShadowPassCameraTransformation * ObjectDataStorage.ObjectToWorldTranslation * ObjectDataStorage.ObjectGeneralTransformation;

        tglUniformMatrix4fv(ObjTransform->ObjectToCameraSpaceTransformationLocation, 1, GL_TRUE, ObjectToCameraSpaceTransformation[0]);
        tglUniformMatrix4fv(ObjTransform->ObjectGeneralTransformationLocation, 1, GL_TRUE, ObjectDataStorage.ObjectGeneralTransformation[0]);
        tglUniformMatrix4fv(VarStorage->Shadow.ObjectToLightSpaceTransformationLocation, 1, GL_TRUE, ObjectToLightSpaceTransformation[0]);

        vec3& ObjectPosition = ObjectDataStorage.ObjectPosition;

        vec3 CameraPositionInObjectUprightSpace = CameraPosition - ObjectPosition;
        tglUniform3fv(VarStorage->Light.ViewerPositionLocation, 1, &CameraPositionInObjectUprightSpace[0]);

        SetupPointLights(PointLights, MAX_POINTS_LIGHTS, ShaderProgramsType::SkeletalMeshShader, &ObjectPosition);
        SetupSpotLights(SpotLights, MAX_SPOT_LIGHTS, ShaderProgramsType::SkeletalMeshShader, &ObjectPosition);

        SkeletalMeshComponent&  Comp                = Object.ObjMesh;
        i32                     PrimitivesAmount    = Comp.PrimitivesAmount;
        MeshPrimitives*         Primitives          = Comp.Primitives;
        for (i32 PrimitiveIndex = 0; PrimitiveIndex < PrimitivesAmount; ++PrimitiveIndex) {
            MeshPrimitives& Primitive   = Primitives[PrimitiveIndex];
            MeshMaterial&   Material    = Primitive.Material;

            tglUniform3fv(VarStorage->MaterialInfo.MaterialAmbientColorLocation, 1, &Material.AmbientColor[0]);
            tglUniform3fv(VarStorage->MaterialInfo.MaterialDiffuseColorLocation, 1, &Material.DiffuseColor[0]);
            tglUniform3fv(VarStorage->MaterialInfo.MaterialSpecularColorLocation, 1, &Material.SpecularColor[0]);

            tglBindVertexArray(Primitive.BuffersHandler[OpenGLBuffersLocation::GLVertexArrayLocation]);

            if (Material.HaveTexture) {
                tglActiveTexture(VarStorage->MaterialInfo.DiffuseTexture.Unit);
                glBindTexture(GL_TEXTURE_2D, Material.TextureHandle);
            }
            else {
                tglActiveTexture(VarStorage->MaterialInfo.DiffuseTexture.Unit);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            if (Material.HaveSpecularExponent) {
                tglActiveTexture(VarStorage->MaterialInfo.SpecularExpMap.Unit);
                glBindTexture(GL_TEXTURE_2D, Material.SpecularExponentMapTextureHandle);
            }
            else {
                tglActiveTexture(VarStorage->MaterialInfo.SpecularExpMap.Unit);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            tglDrawElements(GL_TRIANGLES, Primitive.InidicesAmount, GL_UNSIGNED_INT, 0);
        }
    }

    Shader          = &ShadersProgramsCache[ShaderProgramsType::MeshShader];
    VarStorage      = &Shader->ProgramVarsStorage;
    ObjTransform    = &VarStorage->Transform;

    tglUseProgram(Shader->Program);

    tglActiveTexture(VarStorage->Shadow.ShadowMapTexture.Unit);
    glBindTexture(GL_TEXTURE_2D, Cntx->DepthTexture);

    SetupDirectionalLight(&MainLightSource, ShaderProgramsType::MeshShader);

    i32                 StaticSceneObjectsAmount    = FrameData.TestSceneObjectsAmount;
    FrameDataStorage*   StaticObjectsFrameData      = FrameData.TestSceneObjectsFrameStorage;
    SceneObject*        StaticObjects               = Cntx->TestSceneObjects;
    for (i32 Index = 0; Index < StaticSceneObjectsAmount; ++Index) {
        SceneObject&        CurrentSceneObject  = StaticObjects[Index];
        FrameDataStorage&   ObjectDataStorage   = StaticObjectsFrameData[Index];
        WorldTransform&     Transform           = CurrentSceneObject.Transform;

        mat4 ObjectToCameraSpaceTransformation = CameraTransformation * ObjectDataStorage.ObjectToWorldTranslation;
        mat4 ObjectToLightSpaceTransformation = FrameData.ShadowPassCameraTransformation * ObjectDataStorage.ObjectToWorldTranslation * ObjectDataStorage.ObjectGeneralTransformation;

        tglUniformMatrix4fv(ObjTransform->ObjectToCameraSpaceTransformationLocation, 1, GL_TRUE, ObjectToCameraSpaceTransformation[0]);
        tglUniformMatrix4fv(ObjTransform->ObjectGeneralTransformationLocation, 1, GL_TRUE, ObjectDataStorage.ObjectGeneralTransformation[0]);
        tglUniformMatrix4fv(VarStorage->Shadow.ObjectToLightSpaceTransformationLocation, 1, GL_TRUE, ObjectToLightSpaceTransformation[0]);

        vec3& ObjectPosition = ObjectDataStorage.ObjectPosition;

        vec3 CameraPositionInObjectUprightSpace = CameraPosition - ObjectPosition;
        tglUniform3fv(VarStorage->Light.ViewerPositionLocation, 1, &CameraPositionInObjectUprightSpace[0]);

        SetupPointLights(PointLights, MAX_POINTS_LIGHTS, ShaderProgramsType::MeshShader, &ObjectPosition);
        SetupSpotLights(SpotLights, MAX_SPOT_LIGHTS, ShaderProgramsType::MeshShader, &ObjectPosition);

        MeshComponent& Comp = CurrentSceneObject.ObjMesh;
        tglBindVertexArray(Comp.BuffersHandler[OpenGLBuffersLocation::GLVertexArrayLocation]);

        i32                     MeshesAmount        = Comp.MeshesAmount;
        MeshComponentObjects*   MeshComponentInfo   = Comp.MeshesInfo;
        for (i32 Index = 0; Index < MeshesAmount; ++Index) {
            MeshComponentObjects&   MeshInfo        = MeshComponentInfo[Index];
            MeshMaterial&           MeshMaterial    = MeshInfo.Material;

            tglUniform3fv(VarStorage->MaterialInfo.MaterialDiffuseColorLocation, 1,   &MeshMaterial.DiffuseColor[0]);
            tglUniform3fv(VarStorage->MaterialInfo.MaterialAmbientColorLocation, 1,   &MeshMaterial.AmbientColor[0]);
            tglUniform3fv(VarStorage->MaterialInfo.MaterialSpecularColorLocation, 1,  &MeshMaterial.SpecularColor[0]);
            
            if (MeshMaterial.HaveTexture) {
                tglActiveTexture(VarStorage->MaterialInfo.DiffuseTexture.Unit);
                glBindTexture(GL_TEXTURE_2D, MeshMaterial.TextureHandle);
            }
            else {
                tglActiveTexture(VarStorage->MaterialInfo.DiffuseTexture.Unit);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            if (MeshMaterial.HaveSpecularExponent) {
                tglActiveTexture(VarStorage->MaterialInfo.SpecularExpMap.Unit);
                glBindTexture(GL_TEXTURE_2D, MeshMaterial.SpecularExponentMapTextureHandle);
            }
            else {
                tglActiveTexture(VarStorage->MaterialInfo.SpecularExpMap.Unit);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        
            tglDrawElementsBaseVertex(GL_TRIANGLES, MeshInfo.NumIndices, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * MeshInfo.IndexOffset), MeshInfo.VertexOffset);
        }
    }

    Terrain&            Terra               = Cntx->Terrain;
    FrameDataStorage&   TerrainDataStorage  = FrameData.TerrainFrameDataStorage;

    tglBindVertexArray(Terra.BuffersHandler[OpenGLBuffersLocation::GLVertexArrayLocation]);

    mat4 ObjectToCameraSpaceTransform = CameraTransformation * TerrainDataStorage.ObjectToWorldTranslation;
    mat4 ObjectToLightSpaceTransformation = FrameData.ShadowPassCameraTransformation * TerrainDataStorage.ObjectToWorldTranslation * TerrainDataStorage.ObjectGeneralTransformation;

    tglUniformMatrix4fv(VarStorage->Transform.ObjectToCameraSpaceTransformationLocation, 1, GL_TRUE, ObjectToCameraSpaceTransform[0]);
    tglUniformMatrix4fv(VarStorage->Transform.ObjectGeneralTransformationLocation, 1, GL_TRUE, TerrainDataStorage.ObjectGeneralTransformation[0]);
    tglUniformMatrix4fv(VarStorage->Shadow.ObjectToLightSpaceTransformationLocation, 1, GL_TRUE, ObjectToLightSpaceTransformation[0]);

    vec3 ViewerPositionInTerrainUpright = CameraPosition - TerrainDataStorage.ObjectPosition;
    tglUniform3fv(VarStorage->Light.ViewerPositionLocation, 1, &ViewerPositionInTerrainUpright[0]);

    SetupPointLights(PointLights, MAX_POINTS_LIGHTS, ShaderProgramsType::MeshShader, &TerrainDataStorage.ObjectPosition);
    SetupSpotLights(SpotLights, MAX_SPOT_LIGHTS, ShaderProgramsType::MeshShader, &TerrainDataStorage.ObjectPosition);

    tglUniform3fv(VarStorage->MaterialInfo.MaterialAmbientColorLocation, 1, &Terra.AmbientColor[0]);
    tglUniform3fv(VarStorage->MaterialInfo.MaterialDiffuseColorLocation, 1, &Terra.DiffuseColor[0]);
    tglUniform3fv(VarStorage->MaterialInfo.MaterialSpecularColorLocation, 1, &Terra.SpecularColor[0]);

    tglActiveTexture(Shader->ProgramVarsStorage.MaterialInfo.DiffuseTexture.Unit);
    glBindTexture(GL_TEXTURE_2D, Terra.TextureHandle);

    tglActiveTexture(Shader->ProgramVarsStorage.MaterialInfo.SpecularExpMap.Unit);
    glBindTexture(GL_TEXTURE_2D, Terra.TextureHandle);

    tglDrawElements(GL_TRIANGLES, Terra.IndicesAmount, GL_UNSIGNED_INT, 0);
}

static inline void RenderFrame(Platform* Platform, GameContext* Cntx)
{
    PrecalculateObjects(Cntx);

    ShadowPass(Cntx);

    DrawPass(Platform, Cntx);
}

static inline void TakeInput(Platform *Platform, GameContext *Cntx)
{
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
    
    if (Platform->Input.ArrowUp.State == KeyState::Pressed) {
        if (Cntx->BlendingX <= 1.496f) {
            Cntx->BlendingX += 0.001f;
        }
        else {
            Cntx->BlendingX = 1.5f;
        }
    }
    else {
        if (Cntx->BlendingX > 0.005f) {
            Cntx->BlendingX -= 0.001f;
        }
        else {
            Cntx->BlendingX = 0.0f;
        }
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

    if (Platform->Input.EButton.State == KeyState::Pressed && !Cntx->EWasPressed) {;
        Cntx->EWasPressed = 1;
    }
    else if (Platform->Input.EButton.State == KeyState::Released) {
        Cntx->EWasPressed = 0;
    }
}

void Frame(Platform *Platform, GameContext *Cntx)
{
    // glViewport(0, 0, Platform->ScreenOpt.ActualWidth, Platform->ScreenOpt.ActualHeight);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (Cntx->EditorModeOn) {
        
    }

    FrameData& FrameData = Cntx->FrameDt;

    TakeInput(Platform, Cntx);

    Cntx->AnimSystem.Play(0, 0, Cntx->BlendingX, 0.0f, Cntx->DeltaTimeSec);

    mat4 PerspProjection = {};
    MakePerspProjection(PerspProjection, 60.0f, Platform->ScreenOpt.AspectRatio, 0.1f, 1500.0f);

    Cntx->PlayerCamera.Transform.Rotation.b = 0.0f;
    Cntx->PlayerCamera.Transform.Rotation.p += RAD_TO_DEGREE(Platform->Input.MouseInput.Moution.y) * 0.5f;
    Cntx->PlayerCamera.Transform.Rotation.h += RAD_TO_DEGREE(Platform->Input.MouseInput.Moution.x) * 0.5f;

    real32 ZTranslationMultiplyer = (real32)(Platform->Input.WButton.State + (-1 * Platform->Input.SButton.State)); // 1.0 if W Button -1.0 if S Button and 0 if W and S Button pressed together
    real32 XTranslationMultiplyer = (real32)(Platform->Input.DButton.State + (-1 * Platform->Input.AButton.State));

    vec3 Target, Right, Up;
    Rotation& PlayerCameraRotation = Cntx->PlayerCamera.Transform.Rotation;
    PlayerCameraRotation.ToVec(Target, Up, Right);

    Cntx->PlayerCamera.Transform.Position += (Target * ZTranslationMultiplyer * 0.1f) + (Right * XTranslationMultiplyer * 0.1f);

    mat4 CameraTranslation = {}; 
    InverseTranslationFromVec(Cntx->PlayerCamera.Transform.Position, CameraTranslation);

    mat4 CameraUprightToObjectRotation = {};
    PlayerCameraRotation.UprightToObject(CameraUprightToObjectRotation);

    mat4 CameraTransformation = PerspProjection * CameraUprightToObjectRotation * CameraTranslation;

    FrameData.CameraTransformation  = CameraTransformation;
    FrameData.CameraPosition        = Cntx->PlayerCamera.Transform.Position;

    SpotLight* SceneSpotLight               = &Cntx->SpotLights[0];
    SceneSpotLight->Attenuation.Position    = Cntx->PlayerCamera.Transform.Position;
    SceneSpotLight->Rotation                = PlayerCameraRotation;

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

    if(1) {
        RenderFrame(Platform, Cntx);

        // SKELETAL MESHES RENDERING END

        // PARTICLE RENDERER

        /*
        Shader = &ShadersProgramsCache[ShaderProgramsType::ParticlesShader];
        tglUseProgram(Shader->Program);

        VarStorage = &Shader->ProgramVarsStorage;

        Particle*       SceneParticles  = Cntx->SceneParticles;
        ParticleSystem* ParticleSys     = &Cntx->ParticleSystem;
        for (i32 Index = 0; Index < PARTICLES_MAX; ++Index) {
            Particle*       CurrentParticle = &SceneParticles[Index];
            WorldTransform* Transform       = &CurrentParticle->Transform;

            CurrentParticle->Integrate(Cntx->DeltaTimeSec);

            mat4 ObjectToWorldTranslation = {};
            MakeTranslationFromVec(&Transform->Position, &ObjectToWorldTranslation);

            mat4 ObjectToWorlRotation = {};
            MakeObjectToUprightRotation(&Transform->Rotation, &ObjectToWorlRotation);

            mat4 ObjectToWorldScale = {};
            MakeScaleFromVector(&Transform->Scale, &ObjectToWorldScale);

            mat4 ObjectToWorldTransformation = CameraTransformation * ObjectToWorldTranslation;
            mat4 ObjectToWorldScaleAndRotate = ObjectToWorlRotation * ObjectToWorldScale;

            tglUniformMatrix4fv(VarStorage->Transform.ObjectToCameraSpaceTransformationLocation, 1, GL_TRUE, ObjectToWorldTransformation[0]);
            tglUniformMatrix4fv(VarStorage->Transform.ObjectGeneralTransformationLocation, 1, GL_TRUE, ObjectToWorldScaleAndRotate[0]);

            tglBindVertexArray(ParticleSys->BuffersHandler[OpenGLBuffersLocation::GLVertexArrayLocation]);

            tglDrawElements(GL_TRIANGLES, ParticleSys->IndicesAmount, GL_UNSIGNED_INT, 0);
        }
        */

        // PARTICLE RENDERER END

        // DEBUG DRAW RENDERER



        // DEBUG DRAW RENDERER END
    }

    FrameData = {};
}