#include "GltfLoader.h"
#include "3rdparty/cgltf/cgltf.h"

#include <string.h>

#define HANDLE_MEMORY_ALLOCATION(type, amount, assign)      \
    u64 size = sizeof(type) * (amount);                     \
    (assign) = ((type*)malloc(size));                       \
    if (!(assign)) {                                        \
        Assert(false);                                      \
        return GltfFile::Failed;                            \
    }                                                       \
    memset((assign), 0, size);

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

/*
static void ReadJointNode(Skinning* Skin, cgltf_node* Joint, JointsInfo* ParentJoint, i32& Index, cgltf_node** RootJoints, i32 Len)
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
*/

GltfFile::~GltfFile()
{
    if (Meshes) {
        for (i32 MeshIdx = 0; MeshIdx < MeshesAmount; ++MeshIdx) {
            GltfMesh& Mesh = Meshes[MeshIdx];
            
            GltfPrimitive* Primitives = Mesh.Primitives;
            if (!Primitives) {
                continue;
            }

            i32 PrimitivesAmount = Mesh.PrimitivesAmount;
            for (i32 PrimitiveIdx = 0; PrimitiveIdx < PrimitivesAmount; ++PrimitiveIdx) {
                GltfPrimitive& Primitive = Primitives[PrimitiveIdx];
                
                free(Primitive.Positions);
                free(Primitive.Normals);
                free(Primitive.TextureCoord);
                free(Primitive.BoneWeights);
                free(Primitive.BoneIds);
                free(Primitive.Indices);

                free(Primitive.Material.TextureFilePath);
                free(Primitive.Material.SpecularExpFilePath);
            }
            free(Primitives);
        }
        free(Meshes);
    }
    if (Animations) {
        free(Animations);
    }
    if (Skins) {
        free(Skins);
    }
}

bool32 GltfFile::Read(const char* Path)
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

    cgltf_data* Mesh = Open(Path);

    i32 MeshesCount = (i32)Mesh->meshes_count;

    Assert(!Meshes);

    HANDLE_MEMORY_ALLOCATION(GltfMesh, MeshesCount, Meshes);
    
    MeshesAmount = MeshesCount;

    for (i32 MeshIndex = 0; MeshIndex < MeshesCount; ++MeshIndex) {
        cgltf_mesh& CurrentMesh     = Mesh->meshes[MeshIndex];
        GltfMesh&   CurrentMeshOut  = Meshes[MeshIndex];

        i32 PrimitivesAmount = (i32)CurrentMesh.primitives_count;

        HANDLE_MEMORY_ALLOCATION(GltfPrimitive, PrimitivesAmount, CurrentMeshOut.Primitives);

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

            vec3*           Positions       = 0;
            vec3*           Normals         = 0;
            vec2*           TextureCoords   = 0;
            GltfJointIndex* BoneIDs         = 0;
            vec4*           BoneWeights     = 0;
            u32*            Indices         = 0;

            HANDLE_MEMORY_ALLOCATION(vec3, PositionsCount, Positions);
            HANDLE_MEMORY_ALLOCATION(vec3, NormalsCount, Normals);
            HANDLE_MEMORY_ALLOCATION(vec2, TexturesCoordCount, TextureCoords);
            HANDLE_MEMORY_ALLOCATION(GltfJointIndex, WeightsCount, BoneIDs);
            HANDLE_MEMORY_ALLOCATION(vec4, JointsCount, BoneWeights);
            HANDLE_MEMORY_ALLOCATION(u32, IndicesCount, Indices);

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
                char* TextureFileNameTmp = 0;

                u64 SrcLen = strlen(DiffuseTextureFileName);
                u64 DstLen = SrcLen + 1;

                HANDLE_MEMORY_ALLOCATION(char, DstLen, TextureFileNameTmp);

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
                char* SpecularFileNameTmp = 0;

                u64 SrcLen = strlen(SpecularTextureFileName);
                u64 DstLen = SrcLen + 1;

                HANDLE_MEMORY_ALLOCATION(char, DstLen, SpecularFileNameTmp);

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

        CurrentMeshOut.PrimitivesAmount = PrimitivesAmount;
    }

    if (Mesh->skins_count > 0) {
        Assert(Mesh->skins_count == 1);

        // TODO(Ismail): use already created inverse_bind_matrix in CurrentSkin
        cgltf_skin& CurrentSkin = Mesh->skins[0];

        HANDLE_MEMORY_ALLOCATION(GltfSkin, 1, Skins);

        SkinsAmount = Mesh->skins_count;

        

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

    return GltfFile::Success;
}