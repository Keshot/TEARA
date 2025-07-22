#version 460 core

in vec3    FragmentColor;

out vec3    ResultColor

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