#version 430

in vec4 vPosition;
in vec4 vNormal; // the normal data for the current primitive
in vec4 vColor;
out vec4 fColor;
out vec4 fNormal;
out vec4 fPosition;

layout(location = 1) uniform mat4 M;
layout(location = 2) uniform mat4 V;
layout(location = 3) uniform mat4 P;
layout(location = 5) uniform int mode;

vec3 Ia = vec3(0.3, 1.0, 0.3);  // incident ambient light intensity
vec3 Il = vec3(1.0, 1.0, 1.0);  // incident intensity
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
    float d = length(L); L = L/d; // normalize the Light vector
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
    mat4 VM = V*M;
    mat4 U = transpose(inverse(VM));

    // calculate vectors at eye coordinate
    vec3 vNormal_ec = vec3(normalize(U*vNormal));
    vec3 vPosition_ec = vec3(VM * vPosition);
    vec3 LightPos_ec = vec3(V * LightPos_wc);

    switch(mode)
    {
        case 0: // Original Shading
            gl_Position = P * vec4(vPosition_ec, 1);
            fColor = vColor;
            break;
            
        case 1: // Phong Shading
            gl_Position = P*V*M*vPosition;
            fNormal = vNormal;
            fPosition = vPosition;
            break;

        case 2: // Gouraud Shading
            gl_Position = P * vec4(vPosition_ec, 1);
            fColor = shading(LightPos_ec, vPosition_ec, vNormal_ec);
            break;
    }
}