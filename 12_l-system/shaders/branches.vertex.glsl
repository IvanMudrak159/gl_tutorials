#version 430 core
layout(location = 0) in vec3 in_vert;
layout(location = 1) in float in_level;

out float out_level;

void main()
{
	gl_Position = vec4(in_vert, 1.0);
	out_level = in_level;
}

