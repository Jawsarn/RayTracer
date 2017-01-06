#include "IntersectionFunctions.fx"


[numthreads(NUM_GROUP_THREADS, NUM_GROUP_THREADS, 1)]
void CS(uint3 threadID : SV_DispatchThreadID)
{
    // Out slot for rays 
    uint index = threadID.y * ScreenDimensions.x + threadID.x;
    if (threadID.x >= ScreenDimensions.x || threadID.y >= ScreenDimensions.y)
        return;


    Ray newRay = rays[index];
    ColorData data;

    if (newRay.dead)
        return;

    float t = 0;
    float u = 0;
    float v = 0;
    float maxT = 10000000000000.0f;

    int indexTriangle = -1;
    int indexSphere = -1;
    int indexInstance = -1;
    //output[threadID.xy] = float4(0,0, 0, 0);

    for (int inst = 0; inst < NumOfInstances; inst++)
    {
        ObjectInstance objInstance = objInstances[inst];
        matrix world = objInstance.world;
        int vertID = objInstance.startVertex;
        int stop = objInstance.stopVertex;

        for (; vertID < stop; vertID += 3)
        {
            if (CheckTriangleCollision(newRay, vertID, world, t, u, v))
            {
                if (t < maxT && (newRay.lastVertexIndex != vertID || newRay.lastVertexIndex != vertID))
                {
                    maxT = t;
                    indexTriangle = vertID;
                    indexInstance = inst;
                    data.u = u;
                    data.v = v;
                }
            }
        }
    }

    for (int i = 0; i < NumOfSpheres; i++)
    {
        if (CheckSphereCollision(newRay, i, t))
        {
            if (t < maxT && newRay.lastSphereIndex != i)
            {
                maxT = t;
                indexTriangle = -1;
                indexSphere = i;
            }
        }
    }

    data.dead = 0;
    data.indexTriangle = indexTriangle;
    data.indexSphere = indexSphere;
    data.indexInstance = indexInstance;
    data.startPosition = newRay.Position;
    data.direction = newRay.Direction;
    data.hitPosition = newRay.Position + newRay.Direction*maxT;
    data.color = newRay.Color;
    data.reflection = newRay.reflectionFactor;


    colorData[index] = data;
}
