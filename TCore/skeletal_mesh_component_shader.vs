#version 460 core

const int MaxBones  = 200;

layout (location = 0) in vec3   VertexPosition;
layout (location = 1) in vec2   VertexTextureCoordinate;
layout (location = 2) in vec3   VertexNormals;
layout (location = 3) in ivec4  VertexBoneIDs;
layout (location = 4) in vec4   VertexBoneWeights;

out vec2 FragmentTextureCoordinate;
out vec3 FragmentNormal;
out vec3 FragmentPosition;

uniform mat4x4  AnimationBonesMatrices[MaxBones];
uniform mat4x4  ObjectToCameraSpaceTransformation;  // root to world translation, camera space transformation and perspective projection.
uniform mat4x4  ObjectGeneralTransformation;        // object to world/root scale, rotation, translation, attach matrix transformation, and root to world rotation and scale.

void main()
{
    mat4x4 SkinningMatrix = AnimationBonesMatrices[VertexBoneIDs[0]] * VertexBoneWeights[0];
    SkinningMatrix += AnimationBonesMatrices[VertexBoneIDs[1]] * VertexBoneWeights[1];
    SkinningMatrix += AnimationBonesMatrices[VertexBoneIDs[2]] * VertexBoneWeights[2];
    SkinningMatrix += AnimationBonesMatrices[VertexBoneIDs[3]] * VertexBoneWeights[3];

    // position calculation
    vec4 Pos = vec4(VertexPosition, 1.0);

    vec4 FragmentPositionTmp = ObjectGeneralTransformation * SkinningMatrix * Pos;

    FragmentPosition    = FragmentPositionTmp.xyz;
    gl_Position         = ObjectToCameraSpaceTransformation * FragmentPositionTmp;
    // position calculation end

    //normal calculation
    mat3x3 NormalsSkinningMatrix    = transpose(inverse(mat3(SkinningMatrix)));
    mat3x3 NormalObjecToWorldMatrix = transpose(inverse(mat3(ObjectGeneralTransformation)));

    FragmentNormal  = normalize(NormalObjecToWorldMatrix * NormalsSkinningMatrix * VertexNormals);
    //normal calculation end

    // texture coordiante calculation
    FragmentTextureCoordinate   = VertexTextureCoordinate;
    // texture coordiante calculation end
}