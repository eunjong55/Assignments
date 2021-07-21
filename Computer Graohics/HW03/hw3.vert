#version 430 // version
in vec4 vPosition; // position of vertex
uniform mat4 T; // a global shader variable.

void main()
{
	gl_Position = T * vPosition;
}