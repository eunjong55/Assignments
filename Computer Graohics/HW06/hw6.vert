#version 430 

layout(location=1) uniform mat4 T; 
in vec4 vPosition;
in vec4 vColor;
out vec4 fColor;

void main()
{
	gl_Position = T * vPosition;
	fColor = vColor; 
}