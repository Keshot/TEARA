#ifndef _TEARA_GAME_PLATFORM_H_
#define _TEARA_GAME_PLATFORM_H_

// NOTE(ismail): remove this shit
#include <string>
#include <map>
#include <list>

#include "TLib/Utils/Types.h"
#include "TLib/Utils/AssetsLoader.h"
#include "TLib/Math/Vector.h"
#include "TLib/Math/Rotation.h"
#include "TLib/Math/Quat.h"

#define BATTLE_AREA_GRID_VERT_AMOUNT    100
#define ONE_SQUARE_INDEX_AMOUNT         6
#define TERRAIN_INDEX_AMOUNT            SQUARE(BATTLE_AREA_GRID_VERT_AMOUNT - 1) * ONE_SQUARE_INDEX_AMOUNT
#define SCENE_OBJECTS_MAX               3
#define DYNAMIC_SCENE_OBJECTS_MAX       1
#define MAX_POINTS_LIGHTS               2
#define MAX_SPOT_LIGHTS                 1
#define MAX_BONES                       200
#define MAX_KEYFRAMES                   400
#define MAX_CHARACTER_ANIMATIONS        20
#define MAX_JOINT_CHILDREN_AMOUNT       10
#define MAX_MESH_PRIMITIVES             5
#define MAX_MESHES                      1
#define SHADOW_MAP_W                    (2048)
#define SHADOW_MAP_H                    (2048)

enum OpenGLBuffersLocation {
    // STATIC MESH
    GLPositionLocation,
    GLTextureLocation,
    GLNormalsLocation,
    // SKELETAL MESH
    GLBoneIndicesLocation,
    GLBoneWeightsLocation,
    // MISC
    GLIndexArrayLocation,
    GLVertexArrayLocation,

    GLLocationMax,
};

const vec3 GRAVITY = { 0.0f, -10.0f, 0.0f };

struct JointsInfo {
    std::string BoneName;
    JointsInfo* Parent;
    JointsInfo* Children[MAX_JOINT_CHILDREN_AMOUNT];
    i32         ChildrenAmount;
    vec3        DefaultTranslation;
    vec3        DefaultScale;
    quat        DefaultRotation;
    mat4      InverseBindMatrix;
};

struct BoneIDs {
    i32 OriginalBoneID;
    i32 BoneID;
};

struct Skinning {
    std::map<std::string, BoneIDs>  Bones;
    JointsInfo*                     Joints;
    u32                             JointsAmount;
};

enum AnimationType {
    ATranslation,
    ARotation,
    AScale,
    AMax,
};

enum InterpolationType {
    IStep,
    ILinear,
    IMax,
};

// ?????
union TransformationStorage {
    TransformationStorage() {

    }

    vec3    Translation;
    quat    Rotation;
    vec3    Scale;
};

enum PLayerAnimations {
    IdleDynamic = 0,
    WalkDefault = 1,
    RunDefault  = 2,
    Max
};

struct AnimationTransformation {
    bool32                  Valid;
    InterpolationType       IType;
    real32                  Keyframes[MAX_KEYFRAMES];
    TransformationStorage   Transforms[MAX_KEYFRAMES];
    i32                     Amount;
};

struct AnimationFrame {
    i32                     Target;
    i32                     OriginalBoneID;
    AnimationTransformation Transformations[AMax];
};

struct Animation {
    AnimationFrame  PerBonesFrame[MAX_BONES];
    i32             FramesAmount;
    real32          MaxDuration;
};

struct AnimationsArray {
    Animation   Anims[MAX_CHARACTER_ANIMATIONS];
    i32         AnimsAmount;
};

struct WorldTransform {
    vec3        Position;
    Rotation    Rotation;
    vec3        Scale;
};

struct Camera {
    WorldTransform Transform;
};

enum ShaderProgramsType {
    MeshShader,
    SkeletalMeshShader,
    ParticlesShader,
    DebugDrawShader,
    DepthTestShader,
    ShaderProgramsTypeMax,
};

struct ShaderProgramVariablesStorage {

    struct ShaderTextureInfo {
        i32 Location;
        i32 Unit;
        i32 UnitNum;
    };

    struct ObjectTransform {
        i32 ObjectToCameraSpaceTransformationLocation;
        i32 ObjectGeneralTransformationLocation;
    } Transform;

    struct ObjectMaterial {
        i32 MaterialAmbientColorLocation;
        i32 MaterialDiffuseColorLocation;
        i32 MaterialSpecularColorLocation;

        ShaderTextureInfo   DiffuseTexture;
        ShaderTextureInfo   SpecularExpMap;
    } MaterialInfo;

    struct ShadowMapping {
        i32                 ObjectToLightSpaceTransformationLocation;
        ShaderTextureInfo   ShadowMapTexture;
    } Shadow;

    struct LightWork {

        struct LightSpecLocations {
            i32 ColorLocation;
            i32 IntensityLocation;
            i32 AmbientIntensityLocation;
            i32 SpecularIntensityLocation;
        };

        struct LightAttenuationLocations {
            i32 PositionLocation;
            i32 DisctanceMaxLocation;
            i32 DisctanceMinLocation;
            i32 AttenuationFactorLocation;
        };

        struct PointLightLocations {
            LightSpecLocations          SpecLocation;
            LightAttenuationLocations   AttenuationLocation;
        };

        struct SpotLightLocations {
            LightSpecLocations          SpecLocation;
            LightAttenuationLocations   AttenuationLocation;
            i32                         DirectionLocation;
            i32                         CosCutoffAngleLocation;
            i32                         CutoffAttenuationFactorLocation;
        };

        PointLightLocations PointLightsLocations[MAX_POINTS_LIGHTS];

        SpotLightLocations  SpotLightsLocations[MAX_SPOT_LIGHTS];

        LightSpecLocations  DirectionalLightSpecLocations;
        i32                 DirectionalLightDirectionLocation;

        i32                 ViewerPositionLocation;
        i32                 PointLightsAmountLocation;
        i32                 SpotLightsAmountLocation;
    } Light;

    struct AnimationInfo {
        i32 AnimationMatricesLocation[MAX_BONES];
        i32 HaveSkinMatricesLocation;
    } Animation;

};

struct ShaderProgram {
    u32                             Program;
    ShaderProgramVariablesStorage   ProgramVarsStorage;
};

struct LightSpec {
    vec3    Color;
    real32  Intensity;
    real32  AmbientIntensity;
    real32  SpecularIntensity;
};

struct LightAttenuation {
    vec3        Position;
    real32      DisctanceMax;
    real32      DisctanceMin;
    real32      AttenuationFactor;
};

struct DirectionalLight {
    LightSpec   Specification;
    Rotation    Rotation;
};

struct PointLight {
    LightSpec           Specification;
    LightAttenuation    Attenuation;
};

struct SpotLight {
    LightSpec           Specification;
    LightAttenuation    Attenuation;
    Rotation            Rotation;
    real32              CosCutoffAngle;
    real32              CutoffAttenuationFactor;
};

struct MeshMaterial {
    bool32  HaveTexture;
    u32     TextureHandle;
    bool32  HaveSpecularExponent;
    u32     SpecularExponentMapTextureHandle;
    vec3    AmbientColor;
    vec3    DiffuseColor;
    vec3    SpecularColor;
};

struct MeshComponentObjects {
    MeshMaterial    Material;
    u32             NumIndices;
    u32             IndexOffset;
    u32             VertexOffset;
};

#define MAX_CHARACTERS_ANIMATION_TASKS  (MAX_CHARACTER_ANIMATIONS)
#define ANIMATION_STACK_LENGTH          (8)

struct SkeletalComponent {
    AnimationsArray Animations;
    Skinning        Skin;
};

struct SkinningMatricesStorage {
    mat4    Matrices[MAX_BONES];
    i32     Amount;
};

enum TaskMode {
    Clip,
    _1D,
    _2D
};

struct AnimationStack {
    real32      Speed;
    real32      CurrentTime;
    real32      MaxDuration;
    real32      StackPositionX;
    real32      StackPositionY;
    Animation*  Animation;
};

struct AnimationTask {
    TaskMode        Mode;
    real32          x;
    real32          y;
    real32          MaxX;
    real32          MaxY;
    bool32          Loop;
    AnimationStack  Stack[ANIMATION_STACK_LENGTH];
    i32             StackAmount;
};

enum SkeletalCharacters {
    CharacterPlayer,
    SkeletalMax,
};

struct AnimationTrack {
    i32                     Id;
    SkeletalCharacters      SkinId;
    AnimationTask           AnimationTasks[MAX_CHARACTERS_ANIMATION_TASKS];
    i32                     AnimationTasksAmount;
    SkinningMatricesStorage Matrices;
};

class AnimationSystem {
public:
    AnimationSystem() = default;
    ~AnimationSystem() = default;
    
    AnimationSystem(const AnimationSystem&) = delete;
    AnimationSystem(AnimationSystem&&) = delete;
    
    AnimationSystem& operator=(const AnimationSystem&) = delete;
    AnimationSystem& operator=(AnimationSystem&&) = delete;

    SkeletalComponent& RegisterNewSkin(SkeletalCharacters Id) {
        return SkinningData[Id];
    }

    AnimationTrack& RegisterNewAnimationTrack() {
        CharactersAnimationTrack.emplace_back();
        return CharactersAnimationTrack.back();
    }

    Animation* GetAnimationById(SkeletalCharacters SkeletId, i32 AnimationId) {
        return &SkinningData[SkeletId].Animations.Anims[AnimationId];
    }

    void Play(i32 CharId, i32 AnimTaskId, real32 x, real32 y, real32 dt);
    mat4& GetBoneLocation(i32 CharId, i32 BoneId);
    mat4& GetBoneLocation(i32 CharId, const std::string& BoneName);
    void ExportToRender(SkinningMatricesStorage& Result, i32 CharId);

private:
    void PrepareSkinMatrices(AnimationTrack& Track, i32 TaskId, real32 x, real32 y, real32 dt);

    std::list<AnimationTrack>   CharactersAnimationTrack;
    SkeletalComponent           SkinningData[SkeletalCharacters::SkeletalMax];
};

struct MeshComponent {
    const char*             ObjectPath;
    MeshComponentObjects*   MeshesInfo;
    u32                     MeshesAmount;
    u32                     BuffersHandler[GLLocationMax];
};

struct MeshPrimitives {
    MeshMaterial    Material;
    u32             BuffersHandler[GLLocationMax];
    u32             InidicesAmount;
};

struct SkeletalMeshComponent {
    const char*         ObjectPath;
    MeshPrimitives      Primitives[MAX_MESH_PRIMITIVES];
    i32                 PrimitivesAmount;
    SkeletalComponent   Skelet;
};

struct DynamicSceneObject;

struct ObjectNesting {
    std::string         AttachedToBone;
    DynamicSceneObject* Parent;
};

struct SceneObject {
    WorldTransform  Transform;
    MeshComponent   ObjMesh;
    ObjectNesting   Nesting;
};

struct DynamicSceneObject {
    WorldTransform          Transform;
    SkeletalMeshComponent   ObjMesh;
    ObjectNesting           Nesting;
};

struct TerrainLoadFile {
    vec3    Vertices[SQUARE(BATTLE_AREA_GRID_VERT_AMOUNT)];
    vec2    Textures[SQUARE(BATTLE_AREA_GRID_VERT_AMOUNT)];
    vec3    Normals[SQUARE(BATTLE_AREA_GRID_VERT_AMOUNT)];
    u32     Indices[TERRAIN_INDEX_AMOUNT];
    u32     VerticesAmount;
    u32     TexturesAmount;
    u32     NormalsAmount;
    u32     IndicesAmount;
};

struct Terrain {
    vec3            AmbientColor;
    vec3            DiffuseColor;
    vec3            SpecularColor;
    u32             BuffersHandler[GLLocationMax];
    u32             TextureHandle;
    u32             IndicesAmount;
    WorldTransform  Transform;
};

#define PARTICLES_MAX 1

struct ParticleSystem {
    u32 BuffersHandler[GLLocationMax];
    i32 IndicesAmount;
};

struct Particle {
    WorldTransform  Transform;
    vec3            Velocity;
    vec3            Acceleration;
    real32          Damping;
    real32          InverseMass;

    void Integrate(real32 DeltaT)
    {
        // x(t) = x0 + v*t
        // v(t) = v0*t + a*t
        
        Transform.Position += Velocity * DeltaT;
        
        // NOTE(Ismail): it may cause some perfomance issue, if we will be simulate many particles
        // so check it for optimization if it will need
        real32 DampingEffect = powf(Damping, DeltaT);

        vec3 AccelerationTmp = Acceleration * DeltaT;

        Velocity += AccelerationTmp;

        Velocity *= DampingEffect;
    }
};

struct FrameDataStorage {
    SkinningMatricesStorage SkinFrameStorage;
    mat4                    ObjectToWorldTranslation;
    mat4                    ObjectGeneralTransformation;
    mat4                    ShadowPassObjectMatrices;
    vec3                    ObjectPosition;
};

struct FrameData {
    FrameDataStorage    TerrainFrameDataStorage;
    FrameDataStorage    TestSceneObjectsFrameStorage[SCENE_OBJECTS_MAX];
    i32                 TestSceneObjectsAmount;
    FrameDataStorage    TestDynamocSceneObjectsFrameStorage[DYNAMIC_SCENE_OBJECTS_MAX];
    i32                 TestDynamocSceneObjectsAmount;
    mat4                ShadowPassCameraTransformation;
    mat4                CameraTransformation;
    vec3                CameraPosition;
};

struct GameContext {
    u32 DepthFbo;
    u32 DepthTexture;

    real32  DeltaTimeSec;

    bool32  PolygonModeActive;
    bool32  QWasTriggered;
    bool32  MWasTriggered;
    bool32  EditorModeOn;

    bool32  ArrowUpWasTriggered;

    real32  TranslationDelta;
    real32  RotationDelta;
    
    i32     CurrentStep;

    i32     BoneID;
    real32  BlendingX;

    Camera              PlayerCamera;
    SceneObject         TestSceneObjects[SCENE_OBJECTS_MAX];
    DynamicSceneObject  TestDynamocSceneObjects[DYNAMIC_SCENE_OBJECTS_MAX];
    Terrain             Terrain;
    Particle            SceneParticles[PARTICLES_MAX];
    ParticleSystem      ParticleSystem;

    DirectionalLight    LightSource;
    PointLight          PointLights[MAX_POINTS_LIGHTS];
    SpotLight           SpotLights[MAX_SPOT_LIGHTS];

    AnimationSystem AnimSystem;

    FrameData FrameDt;

    bool32 EWasPressed;
};

#endif