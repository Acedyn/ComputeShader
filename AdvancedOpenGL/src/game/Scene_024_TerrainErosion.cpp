//
// Created by gaetz on 04/12/2019.
//

#include "Scene_024_TerrainErosion.h"
#include "../engine/Timer.h"
#include "../engine/MacroUtils.h"
#include "../engine/Log.h"

#include <cstdlib>
#include <ctime>
#include <GL/glew.h>


Scene_024_TerrainErosion::Scene_024_TerrainErosion():
    dmapDepth(8.0f), isFogEnabled(true), isDisplacementEnabled(true),
    wireframe(false), paused(false), totalTime(0)
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
    // Graphic shader
    ////////////////////////////////////////
    
    Assets::loadShader(SHADER_VERT(SHADER_NAME), SHADER_FRAG(SHADER_NAME), SHADER_TECS(SHADER_NAME), SHADER_TESE(SHADER_NAME), "", SHADER_ID(SHADER_NAME));

    // Create the vertex array that will store the geometry and bind it
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glPatchParameteri(GL_PATCH_VERTICES, 4);

    glEnable(GL_CULL_FACE);

    // Load the color and displacement textures and bind them to the vertex array
    glActiveTexture(GL_TEXTURE1);
    Assets::loadTextureKtx("assets/textures/terragen1.ktx", "terragen1");
    texDisplacement = Assets::getTextureKtx("terragen1").id;
    glBindTexture(GL_TEXTURE_2D, texDisplacement);

    glActiveTexture(GL_TEXTURE2);
    Assets::loadTextureKtx("assets/textures/terragen_color.ktx", "terragen_color");
    texColor = Assets::getTextureKtx("terragen_color").id;
    glBindTexture(GL_TEXTURE_2D, texColor);

    shader = Assets::getShader(SHADER_ID(SHADER_NAME));


    ////////////////////////////////////////
    // Compute shader
    ////////////////////////////////////////

    Assets::loadComputeShader(SHADER_COMP(SHADER_NAME), SHADER_ID(SHADER_NAME));

    // Create two buffers that will hold the data of the input and the output of the compute shader
    glGenBuffers(2, dataBuffer);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, dataBuffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_ELEMENTS * sizeof(float), NULL, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, dataBuffer[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_ELEMENTS * sizeof(float), NULL, GL_DYNAMIC_COPY);

    // Initialize the input buffer with random values
    for (int i = 0; i < NUM_ELEMENTS; i++)
    {
        inputData[i] = randomFloat();
    }

    // Bind the program of the shader
    cShader = Assets::getComputeShader(SHADER_ID(SHADER_NAME));

    // Set the binding of the storage buffer on the shader
    // Here, we attach the buffer to the binding 0 of the shader
    // And the buffer 1 to the binding 1 of the shader
    glShaderStorageBlockBinding(cShader.id, 0, 0);
    glShaderStorageBlockBinding(cShader.id, 1, 1);
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
    
    // Initialize the pointer to the output data
    float* ptr;

    // Initialize and pass the input array to the buffer 0
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, dataBuffer[0], 0, sizeof(float) * NUM_ELEMENTS);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(float) * NUM_ELEMENTS, inputData);

    // Initialize the buffer 1
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, dataBuffer[1], 0, sizeof(float) * NUM_ELEMENTS);

    // Bind the program of the shader and fire the computation
    cShader.use();
    // Dispach the computation on a single global working group
    glDispatchCompute(1, 1, 1);

    // Block the thread until the computation is completed
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glFinish();

    // Get the result of the buffer 1
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, dataBuffer[1], 0, sizeof(float) * NUM_ELEMENTS);
    ptr = (float*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(float) * NUM_ELEMENTS, GL_MAP_READ_BIT);

    // Copy the buffer 1 content into an other buffer and print it
    char buffer[1024];
    sprintf(buffer, "SUM: %2.2f %2.2f %2.2f %2.2f %2.2f %2.2f %2.2f %2.2f "
        "%2.2f %2.2f %2.2f %2.2f %2.2f %2.2f %2.2f %2.2f",
        ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7],
        ptr[8], ptr[9], ptr[10], ptr[11], ptr[12], ptr[13], ptr[14], ptr[15]);

    LOG(Info) << buffer;

    // Unmap the buffer (mapping a buffer means link its gpu data to cpu data so we can access it in C++)
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
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

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    if (wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawArraysInstanced(GL_PATCHES, 0, 4, 64 * 64);
}
