#include "Common.fx"


[numthreads(NUM_GROUP_THREADS, NUM_GROUP_THREADS, 1)]
void CS(uint3 threadID : SV_DispatchThreadID)
{
    uint index = threadID.y * ScreenDimensions.x + threadID.x;
    if (threadID.x >= ScreenDimensions.x || threadID.y >= ScreenDimensions.y )
        return;


    ColorData data = colorData[index];


    // Create ray
    Ray newRay;
    newRay.Position = data.hitPosition;
    newRay.Direction = data.direction;
    newRay.Color = data.color;
    newRay.lastVertexIndex = data.indexTriangle;
    newRay.lastSphereIndex = data.indexSphere;
    newRay.lastInstanceIndex = data.indexInstance;
    newRay.reflectionFactor = data.reflection;
    newRay.dead = data.dead;
    rays[index] = newRay;
}