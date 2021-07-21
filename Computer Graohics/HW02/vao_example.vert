#version 430 // version

in vec4 vPosition; // position of vertex
in vec4 vColor;
out vec4 fColor;

void main()
{
	gl_Position = vPosition;
	fColor = vColor;
}