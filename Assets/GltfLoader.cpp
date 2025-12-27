#include "GltfLoader.h"
#include "3rdparty/cgltf/cgltf.h"
#include "Math/Transformation.h"

#include <string.h>

// TODO(Ismail): move all test staff to somewhere

inline bool32 GltfMemoryArena::Init(u64 Len)
{
    if (Base) {
        Assert(false); // u can't allocate memory arena twice for GltfFile
    }

    Base = (byte*)malloc(Len);
    if (!Base) {
        Assert(false);

        return GltfFile::Failed;
    }

    Size      = Len;
    NextAvail = Base;
    AvailSize = Size;

    memset(Base, 0, Size);

    return GltfFile::Success;
}

inline void GltfMemoryArena::Clear()
{
    NextAvail = Base;
    AvailSize = Size;

    memset(Base, 0, Size);
}

inline void* GltfMemoryArena::Alloc(u64 Size)
{
    if (Size > AvailSize) {
        Assert(false); // just allocate enough 

        return 0;
    }

    Assert(*NextAvail == 0);

    void* Tmp = NextAvail;

    NextAvail += Size;

    AvailSize -= Size;

    return Tmp;
}

static cgltf_data* Open(const char* Path)
{
    cgltf_data*    Result;
    cgltf_options   LoadOptions = {};

    cgltf_result CallResult = cgltf_parse_file(&LoadOptions, Path, &Result);

    if (CallResult != cgltf_result::cgltf_result_success) {
        Assert(false);
        // TODO(ismail): diagnostic??
        return 0;
    }

    CallResult = cgltf_load_buffers(&LoadOptions, Result, Path);

    if (CallResult != cgltf_result::cgltf_result_success) {
        Assert(false);
        // TODO(ismail): diagnostic??
        return 0;
    }

    return Result;
}

inline static i32 FindBoneIndex(cgltf_node** Joints, i32 Amount, cgltf_node* ToFind)
{
    for (i32 JointIndex = 0; JointIndex < Amount; ++JointIndex) {
        cgltf_node* CompareJoint = Joints[JointIndex];
        if (CompareJoint == ToFind) {
            return JointIndex;
        }
    }

    return -1;
}

inline static void ReadAnimation(cgltf_animation& SrcAnims, GltfAnimation& DstAnims, cgltf_node** Joints, i32 JointsAmount, GltfMemoryArena& Arena)
{
    i32                      ChannelsCount = (i32)SrcAnims.channels_count;
    cgltf_animation_channel* Channels      = SrcAnims.channels;

    GltfAnimationFrame* BonesFrames = (GltfAnimationFrame*)Arena.Alloc(sizeof(*BonesFrames) * JointsAmount);

    DstAnims.PerBonesFrame  = BonesFrames;
    DstAnims.FramesAmount   = JointsAmount;

    real32 AnimationDuration = 0.0f;
    for (i32 ChannelIndex = 0; ChannelIndex < ChannelsCount; ++ChannelIndex) {
        cgltf_animation_channel& CurrentChannel = Channels[ChannelIndex];

        cgltf_animation_sampler* CurrentSampler = CurrentChannel.sampler;

        cgltf_animation_path_type ChannelType       = CurrentChannel.target_path;
        cgltf_interpolation_type  InterpalationType = CurrentSampler->interpolation;

        Assert(ChannelType != cgltf_animation_path_type::cgltf_animation_path_type_invalid &&
               ChannelType != cgltf_animation_path_type::cgltf_animation_path_type_weights);
        
        Assert(InterpalationType != cgltf_interpolation_type::cgltf_interpolation_type_cubic_spline);

        cgltf_accessor* KeyframesAccessor  = CurrentSampler->input;
        cgltf_accessor* TransformsAccessor = CurrentSampler->output;

        Assert(KeyframesAccessor->has_max && KeyframesAccessor->has_min);
        Assert(KeyframesAccessor->max[1] == 0.0f && KeyframesAccessor->min[1] == 0.0f);

        real32 DurationTmp = KeyframesAccessor->max[0];

        if ((DurationTmp != AnimationDuration) && (ChannelIndex > 0)) {
            Assert(false);
        }

        AnimationDuration = DurationTmp > AnimationDuration ? DurationTmp : AnimationDuration;

        i32 KeyframesAmount  = (i32)CurrentSampler->input->count;
        i32 TransformsAmount = (i32)CurrentSampler->output->count;

        Assert(TransformsAmount == KeyframesAmount);

        cgltf_node* TargetNode = CurrentChannel.target_node;

        i32 BoneIdx = FindBoneIndex(Joints, JointsAmount, TargetNode);

        Assert(BoneIdx >= 0);

        GltfAnimationFrame& Frame = BonesFrames[BoneIdx];

        GltfAnimationTransform* Transform;
        switch(ChannelType) {
            case cgltf_animation_path_type::cgltf_animation_path_type_translation: {
                Transform = &Frame.Transformations[GltfAnimationFrame::ATranslation];

                GltfAnimationTransform::TransformationStorage* TransformsStorage = (GltfAnimationTransform::TransformationStorage*)Arena.Alloc(sizeof(*TransformsStorage) * TransformsAmount);

                for (i32 TransformIndex = 0; TransformIndex < TransformsAmount; ++TransformIndex) {
                    vec3& Elem = TransformsStorage[TransformIndex].Translation;
                    cgltf_accessor_read_float(TransformsAccessor, TransformIndex, Elem.vec, sizeof(Elem));
                }

                Transform->Transforms = TransformsStorage;
            } break;

            case cgltf_animation_path_type::cgltf_animation_path_type_rotation: {
                Transform = &Frame.Transformations[GltfAnimationFrame::ARotation];

                GltfAnimationTransform::TransformationStorage* TransformsStorage = (GltfAnimationTransform::TransformationStorage*)Arena.Alloc(sizeof(*TransformsStorage) * TransformsAmount);

                for (i32 TransformIndex = 0; TransformIndex < TransformsAmount; ++TransformIndex) {
                    real32 Elem[4] = {};
                    cgltf_accessor_read_float(TransformsAccessor, TransformIndex, Elem, sizeof(Elem));

                    quat& Rot = TransformsStorage[TransformIndex].Rotation;
                    Rot.w = Elem[_w_];
                    Rot.x = Elem[_x_];
                    Rot.y = Elem[_y_];
                    Rot.z = Elem[_z_];
                }

                Transform->Transforms = TransformsStorage;
            } break;

            case cgltf_animation_path_type::cgltf_animation_path_type_scale: {
                Transform = &Frame.Transformations[GltfAnimationFrame::AScale];
                
                GltfAnimationTransform::TransformationStorage* TransformsStorage = (GltfAnimationTransform::TransformationStorage*)Arena.Alloc(sizeof(*TransformsStorage) * TransformsAmount);

                for (i32 TransformIndex = 0; TransformIndex < TransformsAmount; ++TransformIndex) {
                    vec3& Elem = TransformsStorage[TransformIndex].Scale;
                    cgltf_accessor_read_float(TransformsAccessor, TransformIndex, Elem.vec, sizeof(Elem));
                }
                
                Transform->Transforms = TransformsStorage;
            } break;
        }

        real32* Keyframes = (real32*)Arena.Alloc(sizeof(*Keyframes) * KeyframesAmount);

        i32 Read = (i32)cgltf_accessor_unpack_floats(KeyframesAccessor, Keyframes, KeyframesAmount);

        Assert(Read == KeyframesAmount);

        Transform->Keyframes = Keyframes;
        Transform->Amount    = KeyframesAmount;
        Transform->IType     = InterpalationType == cgltf_interpolation_type::cgltf_interpolation_type_linear ? GltfAnimationTransform::ILinear : GltfAnimationTransform::IStep;
        
        Transform->Validated = 1;
    }

    DstAnims.Duration = AnimationDuration;

    // TODO(ismail): move it into tests for GltfLoader
    Assert(DstAnims.Duration > 0.0f);
    Assert(DstAnims.FramesAmount == JointsAmount);

    i32 FrmAm = DstAnims.FramesAmount;
    GltfAnimationFrame* Fr = DstAnims.PerBonesFrame;
    for (i32 FrIndex = 0; FrIndex < FrmAm; ++FrIndex) {
        GltfAnimationFrame& FrTmp = Fr[FrIndex];

        Assert(FrTmp.Transformations[GltfAnimationFrame::ATranslation].Validated);
        Assert(FrTmp.Transformations[GltfAnimationFrame::ARotation].Validated);
        Assert(FrTmp.Transformations[GltfAnimationFrame::AScale].Validated);
    }
}

inline static bool32 ReadSkin(cgltf_skin& SrcSkin, GltfSkin& DstSkin, GltfMemoryArena& Arena, i32 RootsMax)
{
    i32 RootsAmount = 0;

    cgltf_accessor* InverseMatAccessor = SrcSkin.inverse_bind_matrices;

    Assert(InverseMatAccessor);

    Assert(InverseMatAccessor->component_type == cgltf_component_type_r_32f);
    Assert(InverseMatAccessor->type == cgltf_type_mat4);

    mat4*   InverseBindMatrix;

    i32     MatAmount               = (i32)InverseMatAccessor->count;
    real32* InverseBindMatrixHolder = (real32*)Arena.Alloc(sizeof(*InverseBindMatrix) * MatAmount);

    InverseBindMatrix = (mat4*)InverseBindMatrixHolder;

    i32 Read = (i32)cgltf_accessor_unpack_floats(InverseMatAccessor, InverseBindMatrixHolder, MatAmount * mat4_size);

    cgltf_node**    SrcJoints   = SrcSkin.joints;
    i32             JointsCount = (i32)SrcSkin.joints_count;
    GltfJoint*      DstJoints   = (GltfJoint*)Arena.Alloc(sizeof(*DstJoints) * JointsCount);
    i32*            Roots       = (i32*)Arena.Alloc(sizeof(*Roots) * RootsMax);

    DstSkin.Joints       = DstJoints;
    DstSkin.JointsAmount = JointsCount;
    DstSkin.Roots        = Roots;

    for (i32 JointIndex = 0; JointIndex < JointsCount; ++JointIndex) {
        cgltf_node* SrcJoint    = SrcJoints[JointIndex];
        GltfJoint&  DstJoint    = DstJoints[JointIndex];

        cgltf_node* Parent      = SrcJoint->parent;
        i32         ParentIndex = FindBoneIndex(SrcJoints, JointsCount, Parent);

        if (!SrcJoint->parent || ParentIndex == -1) {
            Roots[RootsAmount++] = JointIndex;

            Assert(RootsAmount <= 1); // for now only 1 root bone we can process
        }

        const char* SrcJointName    = SrcJoint->name;
        u32         SrcNameLen      = (u32)strlen(SrcJointName);
        u32         DstNameLen      = SrcNameLen + 1;
        
        char* DstJointName = (char*)Arena.Alloc(sizeof(*DstJointName) * DstNameLen);

        memcpy_s(DstJointName, DstNameLen, SrcJointName, SrcNameLen);

        cgltf_node**    Childrens       = SrcJoint->children;
        i32             ChildrensAmount = (i32)SrcJoint->children_count;
        i32*            ChildrensTmp    = (i32*)Arena.Alloc(sizeof(*ChildrensTmp) * ChildrensAmount);

        for (i32 ChildIndex = 0; ChildIndex < ChildrensAmount; ++ChildIndex) {
            cgltf_node* Child = Childrens[ChildIndex];

            i32 inx = FindBoneIndex(SrcJoints, JointsCount, Child);

            Assert(inx >= 0);

            ChildrensTmp[ChildIndex] = inx;
        }

        DstJoint.BoneName           = DstJointName;
        DstJoint.NameLen            = SrcNameLen;
        DstJoint.Parent             = ParentIndex;
        DstJoint.Children           = ChildrensTmp;
        DstJoint.ChildrenAmount     = ChildrensAmount;
        DstJoint.InverseBindMatrix  = InverseBindMatrix[JointIndex];

        DstJoint.InverseBindMatrix .Transpose();
    }

    DstSkin.RootsAmount = RootsAmount;
        
    // TODO(ismail): move it into tests for GltfLoader
    for (i32 idx = 0; idx < JointsCount; ++idx) {
        cgltf_node* Joint       = SrcJoints[idx];
        GltfJoint&  JointTmp    = DstJoints[idx];

        const char* OriginalName    = Joint->name;
        const char* ReadName        = JointTmp.BoneName;

        i32 JointChildrenCount      = Joint->children_count;
        i32 JointTmpChildrenCount   = JointTmp.ChildrenAmount;

        Assert(JointChildrenCount == JointTmpChildrenCount);

        cgltf_node**    JointChildrens      = Joint->children;
        i32*            JointTmpChildrens   = JointTmp.Children;
        for (i32 ChildrenIdx = 0; ChildrenIdx < JointChildrenCount; ++ChildrenIdx) {
            cgltf_node* JointChildren       = JointChildrens[ChildrenIdx];
            i32         JointTmpChildren    = JointTmpChildrens[ChildrenIdx];

            const char* JointChildrenName       = JointChildren->name;
            const char* JointTmpChildrenName    = DstJoints[JointTmpChildren].BoneName;

            int TestRes = strcmp(OriginalName, ReadName);

            Assert(!TestRes);
        }

        int TestRes = strcmp(OriginalName, ReadName);

        Assert(!TestRes);
    }

    return GltfFile::Success;
}

inline static bool32 ReadMeshes(cgltf_data* GltfData, GltfMesh* Meshes, GltfMemoryArena& Arena)
{
    u32 IndicesRead         = 0;
    u32 PositionsRead       = 0;
    u32 NormalsRead         = 0;
    u32 TexturesCoordRead   = 0;
    u32 BoneWeightsRead     = 0;
    u32 BoneIdsRead         = 0;

    const cgltf_accessor* AccessorPositions     = 0;
    const cgltf_accessor* AccessorNormals       = 0;
    const cgltf_accessor* AccessorTexturesCoord = 0;
    const cgltf_accessor* AccessorWeights       = 0;
    const cgltf_accessor* AccessorJoints        = 0;

    i32 MeshesCount = GltfData->meshes_count;

    for (i32 MeshIndex = 0; MeshIndex < MeshesCount; ++MeshIndex) {
        cgltf_mesh& CurrentMesh     = GltfData->meshes[MeshIndex];
        GltfMesh&   CurrentMeshOut  = Meshes[MeshIndex];

        i32 PrimitivesAmount = (i32)CurrentMesh.primitives_count;

        CurrentMeshOut.Primitives       = (GltfPrimitive*)Arena.Alloc(sizeof(*CurrentMeshOut.Primitives) * PrimitivesAmount);
        CurrentMeshOut.PrimitivesAmount = PrimitivesAmount;
        
        cgltf_primitive*    PrimitivesBase      = CurrentMesh.primitives;
        GltfPrimitive*      MeshPrimitivesOut   = CurrentMeshOut.Primitives;
        
        for (i32 PrimitiveIndex = 0; PrimitiveIndex < PrimitivesAmount; ++PrimitiveIndex) {
            cgltf_primitive&    CurrentMeshPrimitive            = PrimitivesBase[PrimitiveIndex];
            cgltf_material*     CurrentMeshPrimitiveMaterial    = CurrentMeshPrimitive.material;
            GltfPrimitive&      CurrentPrimitiveOut             = MeshPrimitivesOut[PrimitiveIndex];
            GltfMaterial&       CurrentPrimitiveMaterial        = CurrentPrimitiveOut.Material;

            AccessorPositions     = cgltf_find_accessor(&CurrentMeshPrimitive, cgltf_attribute_type::cgltf_attribute_type_position, 0);
            AccessorNormals       = cgltf_find_accessor(&CurrentMeshPrimitive, cgltf_attribute_type::cgltf_attribute_type_normal, 0);
            AccessorTexturesCoord = cgltf_find_accessor(&CurrentMeshPrimitive, cgltf_attribute_type::cgltf_attribute_type_texcoord, 0);
            AccessorWeights       = cgltf_find_accessor(&CurrentMeshPrimitive, cgltf_attribute_type::cgltf_attribute_type_weights, 0);
            AccessorJoints        = cgltf_find_accessor(&CurrentMeshPrimitive, cgltf_attribute_type::cgltf_attribute_type_joints, 0);
            
            Assert(AccessorPositions && AccessorNormals && AccessorTexturesCoord && AccessorWeights && AccessorJoints);

            if (!AccessorPositions || !AccessorNormals || !AccessorTexturesCoord || 
                !AccessorWeights || !AccessorJoints) {
                return GltfFile::Failed;
            }

            i32 PositionsCount      = AccessorPositions->count;
            i32 NormalsCount        = AccessorNormals->count;
            i32 TexturesCoordCount  = AccessorTexturesCoord->count;
            i32 WeightsCount        = AccessorWeights->count;
            i32 JointsCount         = AccessorJoints->count;
            i32 IndicesCount        = CurrentMeshPrimitive.indices->count;

            vec3*           Positions       = (vec3*)Arena.Alloc(sizeof(*Positions) * PositionsCount);
            vec3*           Normals         = (vec3*)Arena.Alloc(sizeof(*Normals) * NormalsCount);
            vec2*           TextureCoords   = (vec2*)Arena.Alloc(sizeof(*TextureCoords) * TexturesCoordCount);
            GltfJointIndex* BoneIDs         = (GltfJointIndex*)Arena.Alloc(sizeof(*BoneIDs) * WeightsCount);
            vec4*           BoneWeights     = (vec4*)Arena.Alloc(sizeof(*BoneWeights) * JointsCount);
            u32*            Indices         = (u32*)Arena.Alloc(sizeof(*Indices) * IndicesCount);

            CurrentPrimitiveOut.PositionsCount   = PositionsCount;
            CurrentPrimitiveOut.NormalsCount     = NormalsCount;
            CurrentPrimitiveOut.TexturesCount    = TexturesCoordCount;
            CurrentPrimitiveOut.BoneWeightsCount = WeightsCount;
            CurrentPrimitiveOut.BoneIdsCount     = JointsCount;
            CurrentPrimitiveOut.IndicesCount     = IndicesCount;

            Assert(CurrentMeshPrimitive.type == cgltf_primitive_type::cgltf_primitive_type_triangles);

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

#if (TEARA_DEBUG)
            const cgltf_accessor* NextJoints    = cgltf_find_accessor(&CurrentMeshPrimitive, cgltf_attribute_type::cgltf_attribute_type_joints, 1);
            const cgltf_accessor* NextWeights   = cgltf_find_accessor(&CurrentMeshPrimitive, cgltf_attribute_type::cgltf_attribute_type_weights, 1);

            Assert(NextJoints == NULL && NextWeights == NULL);
#endif

            PositionsRead       = (u32)cgltf_accessor_unpack_floats(AccessorPositions,     (real32*)Positions, vec3_size * PositionsCount);
            NormalsRead         = (u32)cgltf_accessor_unpack_floats(AccessorNormals,       (real32*)Normals, vec3_size * NormalsCount);
            TexturesCoordRead   = (u32)cgltf_accessor_unpack_floats(AccessorTexturesCoord, (real32*)TextureCoords, vec2_size * TexturesCoordCount);
            BoneWeightsRead     = (u32)cgltf_accessor_unpack_floats(AccessorWeights,       (real32*)BoneWeights, vec4_size * WeightsCount);

            BoneIdsRead = (u32)cgltf_accessor_unpack_indices_32bit_package(AccessorJoints, BoneIDs, vec4_size * JointsCount);

            IndicesRead = (u32)cgltf_accessor_unpack_indices(CurrentMeshPrimitive.indices, Indices, sizeof(*Indices), IndicesCount);

            Assert(PositionsRead        == vec3_size * PositionsCount && 
                   NormalsRead          == vec3_size * NormalsCount && 
                   TexturesCoordRead    == vec2_size * TexturesCoordCount && 
                   BoneWeightsRead      == vec4_size * WeightsCount && 
                   BoneIdsRead          == vec4_size * JointsCount && 
                   IndicesRead          == IndicesCount);

            CurrentPrimitiveOut.Positions      = Positions;
            CurrentPrimitiveOut.Normals        = Normals;
            CurrentPrimitiveOut.TextureCoord   = TextureCoords;
            CurrentPrimitiveOut.BoneIds        = BoneIDs;
            CurrentPrimitiveOut.BoneWeights    = BoneWeights;
            CurrentPrimitiveOut.Indices        = Indices;

            Assert(CurrentMeshPrimitiveMaterial->has_pbr_metallic_roughness);

            if (CurrentMeshPrimitiveMaterial->has_pbr_metallic_roughness) {
                CurrentPrimitiveMaterial.AmbientColor = { 1.0f, 1.0f, 1.0f };

                cgltf_pbr_metallic_roughness* Diffuse = &CurrentMeshPrimitiveMaterial->pbr_metallic_roughness;

                char* DiffuseTextureFileName = Diffuse->base_color_texture.texture->image->uri;

                u64 SrcLen = strlen(DiffuseTextureFileName);
                u64 DstLen = SrcLen + 1;

                char* TextureFileNameTmp = (char*)Arena.Alloc(sizeof(*TextureFileNameTmp) * DstLen);

                memcpy_s(TextureFileNameTmp, DstLen, DiffuseTextureFileName, SrcLen);

                CurrentPrimitiveMaterial.TextureFilePath = TextureFileNameTmp;
            
                CurrentPrimitiveMaterial.DiffuseColor = { 
                    Diffuse->base_color_factor[_x_],
                    Diffuse->base_color_factor[_y_],
                    Diffuse->base_color_factor[_z_]
                };
                CurrentPrimitiveMaterial.HaveTexture = 1;
            }

            if (CurrentMeshPrimitiveMaterial->has_specular) {
                cgltf_specular& Specular = CurrentMeshPrimitiveMaterial->specular;

                char* SpecularTextureFileName = Specular.specular_texture.texture->image->uri;

                u64 SrcLen = strlen(SpecularTextureFileName);
                u64 DstLen = SrcLen + 1;

                char* SpecularFileNameTmp = (char*)Arena.Alloc(sizeof(*SpecularFileNameTmp) * DstLen);

                memcpy_s(SpecularFileNameTmp, DstLen, SpecularTextureFileName, SrcLen);

                CurrentPrimitiveMaterial.SpecularExpFilePath = SpecularFileNameTmp;

                CurrentPrimitiveMaterial.SpecularColor = { 
                    0.1f, //Specular->specular_color_factor[_x_], 
                    0.1f, //Specular->specular_color_factor[_y_], 
                    0.1f  //Specular->specular_color_factor[_z_] 
                };
                CurrentPrimitiveMaterial.HaveSpecularExponent  = 1;
            }
        }
    }

    return GltfFile::Success;
}

bool32 GltfFile::Read(const char* Path)
{
    bool32  Result  = GltfFile::Success;

    Assert(!Meshes);
    
    if (Arena.Init(MemoryArenaSize) == GltfFile::Failed) {
        return GltfFile::Failed;
    }

    cgltf_data* GltfData = Open(Path);

    MeshesAmount = (i32)GltfData->meshes_count;

    Assert(MeshesAmount == 1); // for now we read only one mesh per file

    Meshes = (GltfMesh*)Arena.Alloc(sizeof(*Meshes) * MeshesAmount);
    
    if (ReadMeshes(GltfData, Meshes, Arena) == GltfFile::Failed) {
        Assert(false);

        Result = GltfFile::Failed;

        goto Finish;
    }

    if (GltfData->skins_count > 0) {
        Assert(GltfData->skins_count == 1);

        SkinsAmount = GltfData->skins_count;

        cgltf_skin* SrcSkins = GltfData->skins;

        Skins = (GltfSkin*)Arena.Alloc(sizeof(*Skins) * SkinsAmount);

        for (i32 SkinIndex = 0; SkinIndex < SkinsAmount; ++SkinIndex) {
            cgltf_skin& SrcSkin = SrcSkins[SkinIndex];
            GltfSkin&   DscSkin = Skins[SkinIndex];

            if (ReadSkin(SrcSkin, DscSkin, Arena, RootJointsMax) == GltfFile::Failed) {
                Assert(false);

                Result = GltfFile::Failed;

                goto Finish;
            }
        }

        AnimationsAmount = (i32)GltfData->animations_count;

        if (AnimationsAmount > 0) {
            Assert(AnimationsAmount == 1); //NOTE(ismail): for now we support only one animation in single gltf file

            cgltf_animation* Anims = GltfData->animations;
            Animations = (GltfAnimation*)Arena.Alloc(sizeof(*Animations) * AnimationsAmount);

            for (i32 AnimIndex = 0; AnimIndex <  AnimationsAmount; ++AnimIndex) {
                cgltf_animation& Anim    = Anims[AnimIndex];
                GltfAnimation&   AnimOut = Animations[AnimIndex];

                ReadAnimation(Anim, AnimOut, SrcSkins->joints, SrcSkins->joints_count, Arena);
            }
        }
    }

Finish:
    cgltf_free(GltfData);

    return Result;
}