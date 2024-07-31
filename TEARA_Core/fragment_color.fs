#version 460 core

uniform vec3 MeshColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(MeshColor.x, MeshColor.y, MeshColor.z, 1.0f);
}