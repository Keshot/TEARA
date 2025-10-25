#version 460 core

const int MaxBones  = 200;

layout (location = 0) in vec3   VertexPosition;
layout (location = 1) in vec2   VertexTextureCoordinate;
layout (location = 2) in vec3   VertexNormals;
layout (location = 3) in ivec4  VertexBoneIDs;
layout (location = 4) in vec4   VertexBoneWeights;

uniform mat4x4  AnimationBonesMatrices[MaxBones];
uniform mat4x4  ObjectToCameraSpaceTransformation;  // root to world translation, camera space transformation and perspective projection.
uniform mat4x4  ObjectGeneralTransformation;        // object to world/root scale, rotation, translation, attach matrix transformation, and root to world rotation and scale.
uniform bool    HaveSkinMatrices;

void main()
{
    mat4x4 SkinningMatrix = mat4x4(1.0);
    vec4 Pos = vec4(VertexPosition, 1.0);
    
    if (HaveSkinMatrices) {
        SkinningMatrix = AnimationBonesMatrices[VertexBoneIDs[0]] * VertexBoneWeights[0];
        SkinningMatrix += AnimationBonesMatrices[VertexBoneIDs[1]] * VertexBoneWeights[1];
        SkinningMatrix += AnimationBonesMatrices[VertexBoneIDs[2]] * VertexBoneWeights[2];
        SkinningMatrix += AnimationBonesMatrices[VertexBoneIDs[3]] * VertexBoneWeights[3];

        Pos = SkinningMatrix * Pos;
    }

    vec4 FragmentPositionTmp = ObjectGeneralTransformation * Pos;

    gl_Position = ObjectToCameraSpaceTransformation * FragmentPositionTmp;
}