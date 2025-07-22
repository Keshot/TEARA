#version 460 core

out vec3    FragmentColor;

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