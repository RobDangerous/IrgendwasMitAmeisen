#version 450

in vec4 color;
in vec3 normal;
in vec3 halfway;
out vec4 frag;

//void main() {
//	frag = color;
//}

const vec4 Ca = vec4 ( 0,  0, .3, 0);
const vec4 Ce = vec4 ( 0,  0,  0, 0);
const vec4 Cd = vec4 ( 0, .5,  0, 0);
const vec4 Cs = vec4 (.8, .8, .8, 0);
const float kse = 30;

const vec3 light = normalize (vec3 (2, 1, 3));

void main() {
	vec3  n  = normalize (normal);
	vec3  h  = normalize (halfway);
	float Id = max (dot (light, n), 0);
	float Is = pow (max (dot (h, n), 0), kse);
	frag = Ca + Ce + Cd * Id + Cs * Is;
}
