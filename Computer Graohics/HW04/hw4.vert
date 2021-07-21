#version 430 // version
in vec4 vPosition;
in vec4 vColor;
out vec4 fColor;
uniform mat4 T; // uniform vector is used(it can be changed in hw4.cpp code easily), it is used for applying rotation and scale change

void main()
{
	gl_Position = T * vPosition; // apply rotation and scale change by muliply the T matrix 
	fColor = vColor; 
}