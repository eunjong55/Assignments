#version 430

in vec4 vPosition;
in vec4 vColor;
out vec4 fColor;
layout(location = 1) uniform mat4 M;
layout(location = 2) uniform mat4 V;
layout(location = 3) uniform mat4 P;

void main()
{
    gl_Position = P * V * M * vPosition;
    fColor = vColor;
}