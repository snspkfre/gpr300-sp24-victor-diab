#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 _Model;

void main()
{
    gl_Position = lightSpaceMatrix * _Model * vec4(aPos, 1.0);
}