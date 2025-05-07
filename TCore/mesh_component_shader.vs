#version 460 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec2 VertexTextureCoordinate;
layout (location = 2) in vec3 VertexNormals;

out vec2        TextureCoordinate;
out vec3        Normals;
out vec3        FragmentPosition;

uniform mat4x4  ObjectToWorldTransformation;

void main()
{
    vec4 ObjectPosition     = vec4(VertexPosition, 1.0);

    gl_Position             = ObjectToWorldTransformation * ObjectPosition;

    TextureCoordinate       = VertexTextureCoordinate;
    Normals                 = VertexNormals;
    FragmentPosition        = VertexPosition;
}