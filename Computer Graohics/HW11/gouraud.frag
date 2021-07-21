#version 430

out vec4 FragColor;
in vec4 fColor;

uniform int ColorMode;

void main()
{
	switch(ColorMode)
	{
	case 0:
		FragColor = fColor;
		break;

	case 1:
		FragColor = vec4(0,0,0,1);
		break;

	case 2:
		FragColor = vec4(255,255,255,1);
		break;
	}

}