#version 430

out vec4 FragColor;

in vec4 fColor;
in vec4 fPosition;
in vec4 fNormal;
layout(location = 1) uniform mat4 M;
layout(location = 2) uniform mat4 V;
layout(location = 3) uniform mat4 P;

uniform int ColorMode;

vec3 Ia = vec3(1.0, 1.0, 1.0); // incident ambient light intensity
vec3 Il = vec3(3.0, 3.0, 3.0); // incident intensity
uniform vec3 Ka; // ambient reflection coefficient
uniform vec3 Ks; // specular reflection coefficient
uniform vec3 Kd; // diffuse refletion coefficient
float c[3] = {0.01, 0.001, 0.0}; // arbitrary constants
uniform float n = 10.0;
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
	switch(ColorMode)
	{
	case 0:
    	mat4 VM = V*M;
	    mat4 U = transpose(inverse(VM));

		// calculate vectors at eye coordinate
	    vec3 vNormal_ec = vec3(normalize(U*fNormal));
	    vec3 fPosition_ec = vec3(VM * fPosition);
	    vec3 LightPos_ec = vec3(V * LightPos_wc);
		FragColor = shading(LightPos_ec, fPosition_ec, vNormal_ec);
		break;

	case 1: 
		FragColor = vec4(0,0,0,1);
		break;
	case 2:
		FragColor = vec4(255,255,255,1);
		break;
	}

}