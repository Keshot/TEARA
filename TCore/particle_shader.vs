#version 460 core

layout (location = 0) in vec3 VertexPosition;

uniform mat4x4  ObjectToWorldTransformation;    // perspective projection, world to camera space rotation, world to camera space translation, object to world translation
uniform mat4x4  ObjectToWorldScaleAndRotate;    // object to world scale and rotation

void main()
{
    // position calculation
    vec4 Pos = vec4(VertexPosition, 1.0);

    gl_Position = ObjectToWorldTransformation * ObjectToWorldScaleAndRotate * Pos;
    // position calculation end
}