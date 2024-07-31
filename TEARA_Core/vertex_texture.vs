#version 460 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TextureCoordinate;
uniform mat4 Transform;
out vec2 TextureCoord;

void main()
{
    gl_Position = Transform * vec4(Position, 1.0);
    TextureCoord = TextureCoordinate;
}