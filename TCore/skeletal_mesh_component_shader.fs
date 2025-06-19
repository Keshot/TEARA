#version 460 core

in vec2 TextureCoordinate;

out vec4 FragmentColor;

uniform sampler2D   DiffuseTexture;

void main()
{
    FragmentColor = texture(DiffuseTexture, TextureCoordinate);
}