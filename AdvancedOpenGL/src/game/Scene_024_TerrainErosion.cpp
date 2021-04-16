//
// Created by simon on 15/04/2021.
//

#include "Scene_024_TerrainErosion.h"
#include "../engine/Timer.h"
#include "../engine/MacroUtils.h"
#include "../engine/Log.h"

#include <cstdlib>
#include <ctime>
#include <GL/glew.h>
#include <math.h>

#define NOISE Noise
#define EROSION Erosion

Scene_024_TerrainErosion::Scene_024_TerrainErosion():
    dmapDepth(8.0f), isFogEnabled(true), isDisplacementEnabled(true),
    wireframe(false), paused(false), totalTime(0), frameIndex(0)
{
}

Scene_024_TerrainErosion::~Scene_024_TerrainErosion() {
    clean();
}

void Scene_024_TerrainErosion::setGame(Game *_game) {
    game = _game;
}

void Scene_024_TerrainErosion::clean() {
    glDeleteVertexArrays(1, &vao);
}

void Scene_024_TerrainErosion::pause() {
}

void Scene_024_TerrainErosion::resume() {
}

void Scene_024_TerrainErosion::handleEvent(const InputState &inputState) {
    if(inputState.keyboardState.isDown(SDL_SCANCODE_Q)) {
        dmapDepth += 0.1f;
    }
    if(inputState.keyboardState.isDown(SDL_SCANCODE_E)) {
        dmapDepth -= 0.1f;
    }
    if(inputState.keyboardState.isJustPressed(SDL_SCANCODE_D)) {
        isDisplacementEnabled = !isDisplacementEnabled;
    }
    if(inputState.keyboardState.isJustPressed(SDL_SCANCODE_F)) {
        isFogEnabled = !isFogEnabled;
    }
    if(inputState.keyboardState.isJustPressed(SDL_SCANCODE_W)) {
        wireframe = !wireframe;
    }
    if(inputState.keyboardState.isJustPressed(SDL_SCANCODE_P)) {
        paused = !paused;
    }
}

void Scene_024_TerrainErosion::load() {

    // Create the noise compute shader
    Assets::loadComputeShader(SHADER_CUSTOM(SHADER_NAME, NOISE), PATH(SHADER_NAME, NOISE));
    cNoiseShader = Assets::getComputeShader(PATH(SHADER_NAME, NOISE));
    // Create the erosion compute shader
    Assets::loadComputeShader(SHADER_CUSTOM(SHADER_NAME, EROSION), PATH(SHADER_NAME, EROSION));
    cErosionShader = Assets::getComputeShader(PATH(SHADER_NAME, EROSION));
    // Create the graphic shaders
    Assets::loadShader(SHADER_VERT(SHADER_NAME), SHADER_FRAG(SHADER_NAME), SHADER_TECS(SHADER_NAME), SHADER_TESE(SHADER_NAME), "", SHADER_ID(SHADER_NAME));
    shader = Assets::getShader(SHADER_ID(SHADER_NAME));

    // Create the vertex array that will store the geometry and bind it
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Initialize the color and height textures (one for read, one for write for each)
    glGenTextures(2, colorTextureID);
    glGenTextures(2, heightTextureID);


    // Create the textures
    glActiveTexture(GL_TEXTURE0);
    createTexture(colorTextureID[0]);
    glActiveTexture(GL_TEXTURE1);
    createTexture(colorTextureID[1]);
    glActiveTexture(GL_TEXTURE2);
    createTexture(heightTextureID[0]);
    glActiveTexture(GL_TEXTURE3);
    createTexture(heightTextureID[1]);

    // Create the compute buffers
    createBrushBuffers(3);

    // Generate a noise on the height texture
    computeNoise();
    // Generate a noise on the height texture
    computeErosion();
    
    // Set some drawing parameters
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    glEnable(GL_CULL_FACE);
}

void Scene_024_TerrainErosion::update(float dt) {
    if (!paused)
        totalTime += dt;

    // Update the variables for the position of the camera
    t = totalTime * 0.03f;
    r = sinf(t * 5.37f) * 15.0f + 16.0f;
    h = cosf(t * 4.79f) * 2.0f + 10.2f;
}

void Scene_024_TerrainErosion::draw()
{
    // Clear the color buffer and the depht buffer before drawing again
    static const GLfloat bgColor[] = {0.0f, 0.0f, 0.2f, 1.0f};
    static const GLfloat one = 1.0f;
    glClearBufferfv(GL_COLOR, 0, bgColor);
    glClearBufferfv(GL_DEPTH, 0, &one);

    // Update the position of the camera
    view = Matrix4::createLookAt(Vector3(0.0, 50, 0.0), Vector3::zero, Vector3(1.0f, 0.0f, 0.0f));
    proj = Matrix4::createPerspectiveFOV(45.0f, game->windowWidth, game->windowHeight, 0.1f, 1000.0f);

    // Set the uniforms 
    shader.use();
    shader.setMatrix4("mv_matrix", view);
    shader.setMatrix4("proj_matrix", proj);
    shader.setMatrix4("mvp_matrix", proj * view);
    shader.setFloat("dmap_depth", isDisplacementEnabled ? dmapDepth : 0.0f);
    shader.setFloat("enable_fog", isFogEnabled ? 1.0f : 0.0f);
    shader.setFloat("enable_fog", isFogEnabled ? 1.0f : 0.0f);
    shader.setInteger("index", frameIndex);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    if (wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawArraysInstanced(GL_PATCHES, 0, 4, 64 * 64);

    frameIndex ^= 1;
}

// Fire the compure shader to create a noise on the height map texture
void Scene_024_TerrainErosion::computeNoise()
{
    // Bind the program of the shader and fire the computation
    cNoiseShader.use();
    glBindTexture(GL_TEXTURE_2D, colorTextureID[frameIndex]);
    glBindImageTexture(0, colorTextureID[frameIndex], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindTexture(GL_TEXTURE_2D, colorTextureID[frameIndex ^ 1]);
    glBindImageTexture(1, colorTextureID[frameIndex ^ 1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindTexture(GL_TEXTURE_2D, heightTextureID[frameIndex]);
    glBindImageTexture(2, heightTextureID[frameIndex], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindTexture(GL_TEXTURE_2D, heightTextureID[frameIndex ^ 1]);
    glBindImageTexture(3, heightTextureID[frameIndex ^ 1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    // Dispach the computation on a single global working group
    glDispatchCompute(TEXTURE_WIDTH / 32, TEXTURE_HEIGHT / 32, 1);

    // Block the thread until the computation is completed
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glFinish();
}

// Fire the compure shader to erode the height map texture
void Scene_024_TerrainErosion::computeErosion()
{
    // Initialize the pointer to the output data
    float* ptr;

    // Bind the program of the shader and fire the computation
    cErosionShader.use();
    // Dispach the computation on a single global working group
    glDispatchCompute(ITERATION / 1000, 1, 1);

    // Block the thread until the computation is completed
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glFinish();
}

// Create a brush for the erosion and store the result in the associated buffers
void Scene_024_TerrainErosion::createBrushBuffers(int radius)
{
    // Compute the sum of all the weights to later normalize it
    float weightSum = 0;

    // Loop over all the cells around the center of the brush
    for (int brushY = -radius; brushY <= radius; brushY++) {
        for (int brushX = -radius; brushX <= radius; brushX++) {
            // Figure out if the given cell is in the radius of the brush
            float sqrDst = brushX * brushX + brushY * brushY;
            if (sqrDst < radius * radius) {
                // Append the position of the cell relative to the center (with a little trick to compress the two integer into one)
                brushOffsets.emplace_back(brushY * TEXTURE_WIDTH + brushX);
                // Get the normalized distance to the center
                float brushWeight = 1 - sqrt(sqrDst) / radius;
                weightSum += brushWeight;
                // Append the normalized distance to the buffer
                brushWeights.emplace_back(brushWeight);
            }
        }
    }
    // Normalize all the weights
    for (int i = 0; i < brushWeights.size(); i++) {
        brushWeights[i] /= weightSum;
    }

    brushWeights.clear();

    // Initialize the input buffer with random values
    for (int i = 0; i < NUM_ELEMENTS; i++)
    {
        brushWeights.emplace_back(randomFloat());
    }

    // Create two buffers that will hold the data of the input and the output of the compute shader
    glGenBuffers(3, computeBuffers);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, computeBuffers[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, brushWeights.size() * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, computeBuffers[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, brushWeights.size() * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, computeBuffers[2]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, brushWeights.size() * sizeof(float), NULL, GL_DYNAMIC_DRAW);

    // Set the binding of the storage buffer on the shader
    // Here, we attach the buffer to the binding 0 of the shader
    // And the buffer 1 to the binding 1 of the shader
    glShaderStorageBlockBinding(cErosionShader.id, 0, 0);
    glShaderStorageBlockBinding(cErosionShader.id, 1, 1);
    glShaderStorageBlockBinding(cErosionShader.id, 2, 2);

    // Initialize and pass the input array to the buffer 0
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, computeBuffers[0], 0, brushWeights.size() * sizeof(float));
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, brushWeights.size() * sizeof(float), &(brushWeights[0]));
    // Initialize and pass the input array to the buffer 1
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, computeBuffers[1], 0, brushWeights.size() * sizeof(float));
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, brushWeights.size() * sizeof(float), &(brushWeights[0]));
    // Initialize and pass the input array to the buffer 2
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, computeBuffers[2], 0, brushWeights.size() * sizeof(float));
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, brushWeights.size() * sizeof(float), &(brushWeights[0]));
}

// Create an opengl texture buffer
void Scene_024_TerrainErosion::createTexture(GLuint textureID)
{
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
}