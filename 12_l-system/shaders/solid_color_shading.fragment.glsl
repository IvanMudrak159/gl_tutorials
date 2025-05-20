#version 430 core

uniform vec4 u_solidColor = vec4(0.0, 0.0, 1.0, 1.0);

const vec3 u_lightDirection = vec3(15.0, 20.0, -10.0);

in vec3 f_normal;

out vec4 out_fragColor;

void main() {
	float lambertian = max(dot(normalize(u_lightDirection), f_normal), 0.0);
	out_fragColor = vec4((u_solidColor.rgb * lambertian), 1.0);
}
