//
// Created by simon on 15/04/2021.
//

#ifndef Scene_024_TerrainErosion_H
#define Scene_024_TerrainErosion_H

#include "../engine/Scene.h"
#include "../engine/Assets.h"
#include <vector>

constexpr int TEXTURE_WIDTH = 2048;
constexpr int TEXTURE_HEIGHT = 2048;
constexpr int ITERATION = 20000;

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

    // Compute shader buffers
    GLuint heightTextureID[2];
    GLuint colorTextureID[2];
    GLuint brushBuffers[2];
    GLuint iterationBuffer;
    // Buffer data
    std::vector<int> brushOffsets;
    std::vector<float> brushWeights;
    // Compute shaders
    ComputeShader cNoiseShader;
    ComputeShader cErosionShader;
    // Frame index so we can know if we are in an odd frame or even frame
    int frameIndex;

    void computeNoise();
    void computeErosion();
    void createBrushBuffers(int radius);
    void createTexture(GLuint textureID);
};

#endif //Scene_024_TerrainErosion_H
