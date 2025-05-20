#version 430 core
layout(lines) in;
layout(triangle_strip, max_vertices = 18) out;

in float out_level[];

uniform mat4 u_modelMat;
uniform mat4 u_viewMat;
uniform mat4 u_projMat;

uniform float u_initialThickness = 0.1;
uniform float u_shrinkageFactor = 0.8;

out vec3 f_normal;

void emitVertex(vec3 position, vec3 normal) {
    gl_Position = u_projMat * u_viewMat * u_modelMat * vec4(position, 1.0);
    f_normal = normalize(mat3(u_modelMat) * normal);
    EmitVertex();
}

void main()
{
    vec3 start = gl_in[0].gl_Position.xyz;
    vec3 end = gl_in[1].gl_Position.xyz;

    float startRadius = u_initialThickness * pow(u_shrinkageFactor, out_level[0]);
    float endRadius =  u_initialThickness * pow(u_shrinkageFactor, out_level[1]);

    vec3 direction = normalize(end - start);
    vec3 up = vec3(0.0, 1.0, 0.0);
    if (abs(direction.y) > 0.99) {
        up = vec3(1.0, 0.0, 0.0);
    }

    vec3 right = normalize(cross(direction, up));
    vec3 forward = normalize(cross(right, direction));

    // Four sides: right, -right, forward, -forward
    vec3 normals[4] = { right, forward, -right, -forward };

    vec3 startOffsets[4] = {
        right * startRadius,
        forward * startRadius,
        -right * startRadius,
        -forward * startRadius
    };

    vec3 endOffsets[4] = {
        right * endRadius,
        forward * endRadius,
        -right * endRadius,
        -forward * endRadius
    };

    for(int i = 0; i < 4; ++i) {
        int next = (i + 1) % 4;

        emitVertex(start + startOffsets[i], normals[i]);
        emitVertex(start + startOffsets[next], normals[next]);

        emitVertex(end + endOffsets[i], normals[i]);
        emitVertex(end + endOffsets[next], normals[next]);

        EndPrimitive();
    }
}
