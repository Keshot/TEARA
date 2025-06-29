#version 460 core

const int BoneInfluenceColorsMax    = 4;
const int MaxBones                  = 100;

layout (location = 0) in vec3   VertexPosition;
layout (location = 1) in vec2   VertexTextureCoordinate;
layout (location = 2) in vec3   VertexNormals;
layout (location = 3) in ivec4  VertexBoneIDs;
layout (location = 4) in vec4   VertexBoneWeights;

out vec2 TextureCoordinate;

uniform mat4x4  AnimationBonesMatrices[MaxBones];
uniform mat4x4  ObjectToWorldTransformation;
uniform int     BoneID;

vec3 BoneInfluenceColors[BoneInfluenceColorsMax];

void InitBoneInfluenceColors()
{
    BoneInfluenceColors[0] = vec3(1.0, 0.0, 0.0);
    BoneInfluenceColors[1] = vec3(0.9, 0.6, 0.0);
    BoneInfluenceColors[2] = vec3(1.0, 0.87, 0.34);
    BoneInfluenceColors[3] = vec3(0.0, 0.0, 1.0);
}

void main()
{
    InitBoneInfluenceColors();

    vec3    VertexColor = vec3(0.0);
    float   Weight      = 0.0;

    if (VertexBoneIDs.x == BoneID) {
        Weight = VertexBoneWeights.x;
    }
    else if (VertexBoneIDs.y == BoneID) {
        Weight = VertexBoneWeights.y;
    }
    else if (VertexBoneIDs.z == BoneID) {
        Weight = VertexBoneWeights.z;
    }
    else if (VertexBoneIDs.w == BoneID) {
        Weight = VertexBoneWeights.w;
    }

    if (Weight >= 0.59) {
        VertexColor = BoneInfluenceColors[0];
    }
    else if (Weight >= 0.39) {
        VertexColor = BoneInfluenceColors[1];
    }
    else if (Weight >= 0.1) {
        VertexColor = BoneInfluenceColors[2];
    }
    else {
        VertexColor = BoneInfluenceColors[3];
    }

    mat4x4 SkinningMatrix = AnimationBonesMatrices[VertexBoneIDs[0]] * VertexBoneWeights[0];
    SkinningMatrix += AnimationBonesMatrices[VertexBoneIDs[1]] * VertexBoneWeights[1];
    SkinningMatrix += AnimationBonesMatrices[VertexBoneIDs[2]] * VertexBoneWeights[2];
    SkinningMatrix += AnimationBonesMatrices[VertexBoneIDs[3]] * VertexBoneWeights[3];

    vec4 VertPosition = vec4(VertexPosition, 1.0);

    vec4 SkinnedVertPosition = SkinningMatrix * VertPosition;

    gl_Position = ObjectToWorldTransformation * SkinnedVertPosition;

    TextureCoordinate   = VertexTextureCoordinate;
}