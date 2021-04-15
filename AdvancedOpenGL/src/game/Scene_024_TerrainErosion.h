//
// Created by gaetz on 04/12/2019.
//

#ifndef Scene_024_TerrainErosion_H
#define Scene_024_TerrainErosion_H

#include "../engine/Scene.h"
#include "../engine/Assets.h"

constexpr int NUM_ELEMENTS = 2048;

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
    GLuint dataBuffer[2];
    float inputData[NUM_ELEMENTS];
    float outputData[NUM_ELEMENTS];
    ComputeShader cShader;
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
