#version 450 core

out vec4 color;

layout (binding = 0) uniform sampler2D tex_color0;

uniform vec4 fog_color = vec4(0.7, 0.8, 0.9, 0.0);
uniform int index = 0;

in TES_OUT
{
    vec2 tc;
} fs_in;

float fit(float value, float min1, float max1, float min2, float max2) {
  return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

void main(void)
{
    float size = 0.002;
    // Get the normal out of the height
    vec3 posN = vec3(fs_in.tc.x + size, texture(tex_color0, vec2(fs_in.tc.x + 0.001, fs_in.tc.y)).x, fs_in.tc.y);
    vec3 posS = vec3(fs_in.tc.x - size, texture(tex_color0, vec2(fs_in.tc.x - 0.001, fs_in.tc.y)).x, fs_in.tc.y);
    vec3 posE = vec3(fs_in.tc.x, texture(tex_color0, vec2(fs_in.tc.x, fs_in.tc.y + 0.001)).x, fs_in.tc.y + size);
    vec3 posW = vec3(fs_in.tc.x, texture(tex_color0, vec2(fs_in.tc.x, fs_in.tc.y - 0.001)).x, fs_in.tc.y - size);
    vec3 normal = normalize(cross(posW - posE, posN - posS));
    // Set the lights directions
    vec3 dirLigth1 = normalize(vec3(1.0, -0.0, 2.0));
    vec3 dirLigth2 = normalize(vec3(0.5, -0.0, -0.5));
    float angle1 = fit(clamp(dot(normal, dirLigth1), 0.0, 0.7), 0.0, 0.5, 0.0, 0.9);
    float angle2 = fit(clamp(dot(normal, dirLigth2), 0.0, 0.7), 0.0, 0.5, 0.0, 0.9);
    float light = mix(angle1, angle2, 0.2);
    color = vec4(vec3(mix(vec3(0.03, 0.06, 0.04), vec3(0.8, 0.85, 0.88), vec3(light))), 1.0);
}
