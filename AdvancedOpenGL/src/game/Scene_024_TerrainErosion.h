//
// Created by simon on 15/04/2021.
//

#ifndef Scene_024_TerrainErosion_H
#define Scene_024_TerrainErosion_H

#include "../engine/Scene.h"
#include "../engine/Assets.h"

constexpr int TEXTURE_WIDTH = 2048;
constexpr int TEXTURE_HEIGHT = 2048;
constexpr int NUM_ELEMENTS = TEXTURE_WIDTH * TEXTURE_HEIGHT;

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

    float totalTime;
    float t, r, h;

    Shader shader;

    // Compute shader
    GLuint heightTextureID[2];
    GLuint colorTextureID[2];
    ComputeShader cNoiseShader;
    ComputeShader cErosionShader;
    int frameIndex;
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
