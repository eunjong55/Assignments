#version 430

layout(location=4) uniform int draw_mode; 

out vec4 FragColor;

in vec4 fColor;
in vec4 fPosition;
in vec4 fNormal;
uniform mat4 M;
uniform mat4 P;
uniform mat4 V;

vec3 Ia = vec3(0.3, 1.0, 0.3); // incident ambient light intensity
vec3 Il = vec3(1.0, 1.0, 1.0); // incident intensity
float Ka = 0.3; // ambient reflection coefficient
float Ks = 0.5; // specular reflection coefficient
float Kd = 0.8; // diffuse refletion coefficient
float c[3] = {0.01, 0.001, 0.0}; // arbitrary constants
float n = 10.0;
vec4 LightPos_wc = vec4(10, 10, 3, 1); // light postition in world coordinate

// to calculate shading vector
vec4 shading(vec3 LightPos_ec, vec3 vPosition_ec, vec3 vNormal_ec)
{
    vec3 N = normalize(vNormal_ec); // Normal vector
    vec3 L = LightPos_ec - vPosition_ec; // Light vector
    float d = length(L); L = L/d; // distance from light source to object
    vec3 V = normalize(vec3(0.0) - vPosition_ec); // View vector
    vec3 R = reflect(-L, N); // Reflection vector

    float fatt = min(1.0 / (c[0] + c[1]*d +c[2]*d*d), 1.0); // to apply light source attenuation

	//calculate cosine alpha and cosine theta
    float cos_theta = max(dot(N,L), 0);

    float cos_alpha = max(dot(V,R), 0);

	// apply phong reflection model
    vec3 I = Ia * Ka + fatt * Il * (Kd * cos_theta + Ks * pow(cos_alpha, n));

    return vec4(I, 1);
}

void main()
{
	switch(draw_mode)
	{
	case 0: // Original
		FragColor = fColor;
		break;

	case 1: // Phong
		mat4 VM = V*M;
	    mat4 U = transpose(inverse(VM));

		// calculate vectors at eye coordinate
	    vec3 vNormal_ec = vec3(normalize(U*fNormal));
	    vec3 fPosition_ec = vec3(VM * fPosition);
	    vec3 LightPos_ec = vec3(V * LightPos_wc);
		FragColor = shading(LightPos_ec, fPosition_ec, vNormal_ec);
		break;

	case 2 : // Gouraud
		FragColor = fColor;
		break;

	case 3 : // black
		FragColor = vec4(0,0,0,1);
		break;

	case 4: // red
		FragColor = vec4(255,0,0,1);
		break;

	case 5: // green
		FragColor = vec4(0,255,0,1);
		break;

	case 6: // blue
		FragColor = vec4(0,0,255,1);
		break;

	case 7: // white
		FragColor = vec4(255,255,255,1);
		break;
	}
}