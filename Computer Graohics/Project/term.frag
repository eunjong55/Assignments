#version 430

layout(location=4) uniform int draw_mode; 
in vec4 fColor;
out vec4 FragColor;

void main()
{
	switch(draw_mode)
	{
	case 0:
		FragColor = fColor;
		break;
	case 1:
		FragColor = vec4(0,0,0,1);
		break;
	case 2:
		FragColor = vec4(255,0,0,1);
		break;
	case 3:
		FragColor = vec4(0,255,0,1);
		break;
	case 4:
		FragColor = vec4(0,0,255,1);
		break;
	case 5:
		FragColor = vec4(255,255,255,1);
		break;
	}
}