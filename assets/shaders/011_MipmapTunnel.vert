#version 450 core

uniform mat4 mvp_matrix;
uniform float offset;

out VS_OUT
{
    vec2 tc;
} vs_out;

void main(void)
{
    const vec2[4] position = vec2[4](vec2(-0.5, -0.5),
                                    vec2( 0.5, -0.5),
                                    vec2(-0.5,  0.5),
                                    vec2( 0.5,  0.5));

    vs_out.tc = (position[gl_VertexID].xy + vec2(offset, 0.5)) * vec2(30.0, 1.0); 
    gl_Position = mvp_matrix * vec4(position[gl_VertexID], 0.0, 1.0);
}
