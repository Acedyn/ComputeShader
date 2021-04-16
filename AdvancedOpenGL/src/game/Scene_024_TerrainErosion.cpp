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

    ////////////////////////////////////////
    // Compute shader
    ////////////////////////////////////////

    glGenTextures(2, colorTextureID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTextureID[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, colorTextureID[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

    glGenTextures(2, heightTextureID);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, heightTextureID[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, heightTextureID[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

    Assets::loadComputeShader(SHADER_CUSTOM(SHADER_NAME, NOISE), PATH(SHADER_NAME, NOISE));
    cNoiseShader = Assets::getComputeShader(PATH(SHADER_NAME, NOISE));

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

    Assets::loadComputeShader(SHADER_CUSTOM(SHADER_NAME, EROSION), PATH(SHADER_NAME, EROSION));
    cErosionShader = Assets::getComputeShader(PATH(SHADER_NAME, EROSION));

    ////////////////////////////////////////
    // Graphic shader
    ////////////////////////////////////////
    
    Assets::loadShader(SHADER_VERT(SHADER_NAME), SHADER_FRAG(SHADER_NAME), SHADER_TECS(SHADER_NAME), SHADER_TESE(SHADER_NAME), "", SHADER_ID(SHADER_NAME));

    // Create the vertex array that will store the geometry and bind it
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glPatchParameteri(GL_PATCH_VERTICES, 4);

    glEnable(GL_CULL_FACE);

    shader = Assets::getShader(SHADER_ID(SHADER_NAME));
}

void Scene_024_TerrainErosion::update(float dt) {
    if (!paused)
        totalTime += dt;

    // Update the variables for the position of the camera
    t = totalTime * 0.03f;
    r = sinf(t * 5.37f) * 15.0f + 16.0f;
    h = cosf(t * 4.79f) * 2.0f + 10.2f;


    ////////////////////////////////////////
    // Compute shader
    ////////////////////////////////////////

    // Bind the program of the shader and fire the computation
    cErosionShader.use();
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

void Scene_024_TerrainErosion::draw()
{
    // Clear the color buffer and the depht buffer before drawing again
    static const GLfloat bgColor[] = {0.0f, 0.0f, 0.2f, 1.0f};
    static const GLfloat one = 1.0f;
    glClearBufferfv(GL_COLOR, 0, bgColor);
    glClearBufferfv(GL_DEPTH, 0, &one);

    // Update the position of the camera
    view = Matrix4::createLookAt(Vector3(sinf(t) * r, h, cosf(t) * r), Vector3::zero, Vector3(0.0f, 1.0f, 0.0f));
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
