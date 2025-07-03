#version 460 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec2 VertexTextureCoordinate;
layout (location = 2) in vec3 VertexNormals;

out vec2    FragmentTextureCoordinate;
out vec3    FragmentNormal;
out vec3    FragmentPosition;

uniform mat4x4  ObjectToWorldTransformation;    // perspective projection, world to camera space rotation, world to camera space translation, object to world translation
uniform mat4x4  ObjectToWorldScaleAndRotate;    // object to world scale and rotation

void main()
{
    // position calculation
    vec4 Pos = vec4(VertexPosition, 1.0);

    vec4 FragmentPositionTmp = ObjectToWorldScaleAndRotate * Pos;

    FragmentPosition    = FragmentPositionTmp.xyz;
    gl_Position         = ObjectToWorldTransformation * FragmentPositionTmp;
    // position calculation end

    //normal calculation
    mat3x3 NormalObjecToWorldMatrix = transpose(inverse(mat3(ObjectToWorldScaleAndRotate)));

    FragmentNormal  = normalize(NormalObjecToWorldMatrix * VertexNormals);
    //normal calculation end

    // texture coordiante calculation
    FragmentTextureCoordinate   = VertexTextureCoordinate;
    // texture coordiante calculation end
}