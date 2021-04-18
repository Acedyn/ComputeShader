//
// Created by simon on 15/04/2021.
//

#ifndef Scene_024_TerrainErosion_H
#define Scene_024_TerrainErosion_H

#include "../engine/Scene.h"
#include "../engine/Assets.h"
#include <vector>

constexpr int TEXTURE_SIZE = 2048;
constexpr int ITERATION = 30000;

class Scene_024_TerrainErosion : public Scene {
public:
    Scene_024_TerrainErosion();
    ~Scene_024_TerrainErosion();
    void load();
    void clean();
    void pause();
    void resume();
    void handleEvent(const InputState &);
    void update(float dt);
    void draw();
    void setGame(Game *);

private:
    Game *game;
    GLuint vao;
    GLuint buffer;

    // Uniforms
    Matrix4 mvp;
    Matrix4 view;
    Matrix4 proj;
    float dmapDepth;
    bool isFogEnabled;

    // Graphic shader
    GLuint texDisplacement;
    GLuint texColor;
    bool isDisplacementEnabled;
    bool wireframe;
    bool paused;
    bool enableErosion;

    float totalTime;
    float t, r, h;
    float rotation;

    Shader shader;

    // Compute shader buffers
    GLuint heightTextureID;
    GLuint computeBuffers[3];
    // Buffer data
    std::vector<int> brushOffsets;
    std::vector<float> brushWeights;
    std::array<int, ITERATION> positions;
    // Compute shaders
    ComputeShader cNoiseShader;
    ComputeShader cErosionShader;
    Vector2 noiseOffset;
    bool isOutdated = true;

    void computeNoise();
    void computeErosion(float test);
    void createBrushBuffers(int radius);
    void createTexture(GLuint textureID);
};

static inline float randomFloat()
{
    static unsigned int seed = 0x13371337;

    float res;
    unsigned int tmp;

    seed *= 16807;
    tmp = seed ^ (seed >> 4) ^ (seed << 15);
    *((unsigned int*)&res) = (tmp >> 9) | 0x3F800000;

    return (res - 1.0f);
}


#endif //Scene_024_TerrainErosion_H
