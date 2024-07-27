#version 460 core

out vec4 FragColor;
in vec2 TextureCoord;
uniform sampler2D Sampler;

void main()
{
    FragColor = texture2D(Sampler, TextureCoord);
}