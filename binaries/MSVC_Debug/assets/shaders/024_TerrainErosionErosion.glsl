#version 450 core

layout (local_size_x = 1000) in;

// Texture buffers
layout (rgba32f, binding = 0) uniform image2D heightMap;
// Storage buffers
layout (binding = 0) coherent buffer block1
{
    int brushOffsets[gl_WorkGroupSize.x];
};
layout (binding = 1) coherent buffer block2
{
    float brushWeights[gl_WorkGroupSize.x];
};
layout (binding = 2) coherent buffer block3
{
    int positions[gl_WorkGroupSize.x];
};
// Uniforms
uniform int textureSize = 0;
uniform int brushLenght = 0;
uniform float depht = 0.0;

vec3 CalculateHeightAndGradient (float posX, float posY) {
    // Get the closest pixel to the given position
    int coordX = int(posX);
    int coordY = int(posY);

    // Get the offset between the pixel and the given position
    float x = posX - float(coordX);
    float y = posY - float(coordY);
    
    // Calculate heights of the four nodes of the droplet's cell
    float heightNW = imageLoad(heightMap, ivec2(coordX, coordY)).x;
    float heightNE = imageLoad(heightMap, ivec2(coordX + 1, coordY)).x;
    float heightSW = imageLoad(heightMap, ivec2(coordX, coordY + 1)).x;
    float heightSE = imageLoad(heightMap, ivec2(coordX + 1, coordY + 1)).x;

    // Calculate droplet's direction of flow with bilinear interpolation of height difference along the edges
    float gradientX = (heightNE - heightNW) * (1.0 - y) + (heightSE - heightSW) * y;
    float gradientY = (heightSW - heightNW) * (1.0 - x) + (heightSE - heightNE) * x;

    // Calculate height with bilinear interpolation of the heights of the nodes of the cell
    float height = heightNW * (1.0 - x) * (1.0 - y) + heightNE * x * (1.0 - y) + heightSW * (1.0 - x) * y + heightSE * x * y;

    return vec3(gradientX, gradientY, height);
}

void main(void)
{
    uint id = gl_GlobalInvocationID.x;
    int index = positions[id];
    
    int maxLifetime = 100;
    float inertia = 0.05;
    float borderSize = 3.0;
    float startSpeed = 3.0;
    float startWater = 3.0;
    float sedimentCapacityFactor = 20;
    float minSedimentCapacity = 0.1;
    float depositSpeed = 0.3;
    float erodeSpeed = 0.3;
    float evaporateSpeed = 0.01;
    float gravity = 4;
    // Get the coordinate of the current iteration
    float posX = mod(float(index), textureSize);
    float posY = float(index) / textureSize;
    // Initialize some attributes
    float dirX = 0;
    float dirY = 0;
    float speed = startSpeed;
    float water = startWater;
    float sediment = 0;

    for (int lifetime = 0; lifetime < maxLifetime; lifetime ++) {
        // Get the closest pixel to the current position
        int nodeX = int(posX);
        int nodeY = int(posY);
        // Get the offset between the pixel and the particle position
        float cellOffsetX = posX - nodeX;
        float cellOffsetY = posY - nodeY;

        // Compute the current gradient and height of the particle
        vec3 heightAndGradient = CalculateHeightAndGradient (posX, posY);
        
        // Update the droplet's direction
        dirX = (dirX * inertia - heightAndGradient.x * (1 - inertia));
        dirY = (dirY * inertia - heightAndGradient.y * (1 - inertia));
        // Normalize direction so we have a constant speed
        float len = max(0.01, sqrt(dirX * dirX + dirY * dirY));
        dirX /= len;
        dirY /= len;
        // Update the position with the normalized direction
        posX += dirX;
        posY += dirY;
        
        // Stop simulating droplet if it's not moving or goes out of the map
        if ((dirX == 0.0 && dirY == 0.0) || posX < borderSize || 
                posX > float(textureSize) - borderSize || posY < borderSize || 
                posY > float(textureSize) - borderSize) { break; }
        
        // Find the droplet's new height and calculate the deltaHeight
        float newHeight = CalculateHeightAndGradient (posX, posY).z;
        float deltaHeight = newHeight - heightAndGradient.z;
        
        // Calculate the droplet's sediment capacity (higher when moving fast down a slope and contains lots of water)
        float sedimentCapacity = max(-deltaHeight * speed * water * sedimentCapacityFactor, minSedimentCapacity);
        // If carrying more sediment than capacity, or if flowing uphill:
        if (sediment > 0.12 || deltaHeight > 0) {
            // If moving uphill (deltaHeight > 0) try fill up to the current height, otherwise deposit a fraction of the excess sediment
            float amountToDeposit = (deltaHeight > 0) ? min(deltaHeight, sediment) : (sediment - sedimentCapacity) * depositSpeed;
            sediment -= amountToDeposit;
        }
        else {
            // Erode a fraction of the droplet's current carry capacity.
            // Clamp the erosion to the change in height so that it doesn't dig a hole in the terrain behind the droplet
            float amountToErode = min((sedimentCapacity - sediment) * erodeSpeed, -deltaHeight);
            
            for (int i = 0; i < brushLenght; i++) {
                ivec2 erodeIndex = ivec2(nodeX + mod(brushOffsets[i], textureSize), nodeY + brushOffsets[i] / textureSize);

                float weightedErodeAmount = amountToErode * brushWeights[i];
                float deltaSediment = (imageLoad(heightMap, erodeIndex).x < weightedErodeAmount) ? imageLoad(heightMap, erodeIndex).x : weightedErodeAmount;
                float newHeight = imageLoad(heightMap, erodeIndex).x - deltaSediment * depht;
                imageStore(heightMap, erodeIndex, vec4(vec3(newHeight), 1.0));
                sediment += deltaSediment;
            }
        }
        // Update droplet's speed and water content
        speed = sqrt(max(0, speed * speed + deltaHeight * gravity));
        water *= (1 - evaporateSpeed);
    }
}