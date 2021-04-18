#version 430

layout (local_size_x = 32, local_size_y = 32) in;

layout (rgba32f, binding = 0) uniform image2D heightMap;

// Uniforms
uniform vec2 offset = vec2(0.0, 0.0);

float rand1D(vec2 pos) {
    return fract(sin(pos.x * 921.0 + pos.y * 481.0) * 5421.0);
}

float noise1D(vec2 uv, int octave) {
    // If the octave if 0 return 0
    if(octave == 0) { return float(0.0); }

    // Initialize the result
    float result = 0.0;
    
    // Loop for each octaves
    for(int i = 0; i < octave; i++) {
        // Zoom out every octave to get more details
        vec2 uvLocal = uv * pow(2.0, i);

        vec2 lv = smoothstep(0.0, 1.0, fract(uvLocal*10.0));
        vec2 id = floor(uvLocal*10.0);

        float bottomLeft = rand1D(id);
        float bottomRight = rand1D(id + vec2(1, 0));
        float bottom = mix(bottomLeft, bottomRight, lv.x);
        float topLeft = rand1D(id + vec2(0, 1));
        float topRight = rand1D(id + vec2(1, 1));
        float top = mix(topLeft, topRight, lv.x);

        result += mix(bottom, top, lv.y) / pow(2.0, i*1.6);
    }

    return result / 2.0;
}

void main() {
    // Get the coordinates of the current pixel
    ivec2 coords = ivec2(gl_GlobalInvocationID);
    // Get the normalized coordinates of the current pixel
    vec2 uv = vec2(gl_GlobalInvocationID) / (vec2(gl_NumWorkGroups ) * vec2(gl_WorkGroupSize));

    float c = noise1D((uv + offset) * 0.5, 8);
    vec3 color = vec3(c);

    vec4 pixel = vec4(color, 1.0);

    imageStore(heightMap, coords, pixel);
}