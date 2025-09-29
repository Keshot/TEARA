#version 460 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec2 VertexTextureCoordinate;
layout (location = 2) in vec3 VertexNormals;

out vec2    FragmentTextureCoordinate;
out vec3    FragmentNormal;
out vec3    FragmentPosition;

uniform mat4x4  ObjectToCameraSpaceTransformation;  // root to world translation, camera space transformation and perspective projection.
uniform mat4x4  ObjectGeneralTransformation;        // object to world/root scale, rotation, translation, attach matrix transformation, and root to world rotation and scale.

void main()
{
    // position calculation
    vec4 Pos = vec4(VertexPosition, 1.0);

    vec4 FragmentPositionTmp = ObjectGeneralTransformation * Pos;

    FragmentPosition    = FragmentPositionTmp.xyz;
    gl_Position         = ObjectToCameraSpaceTransformation * FragmentPositionTmp;
    // position calculation end

    //normal calculation
    mat3x3 NormalObjecToWorldMatrix = transpose(inverse(mat3(ObjectGeneralTransformation)));

    FragmentNormal  = normalize(NormalObjecToWorldMatrix * VertexNormals);
    //normal calculation end

    // texture coordiante calculation
    FragmentTextureCoordinate   = VertexTextureCoordinate;
    // texture coordiante calculation end
}