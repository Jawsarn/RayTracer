#include "Common.fx"




[numthreads(NUM_GROUP_THREADS, NUM_GROUP_THREADS, 1)]
void CS( uint3 threadID : SV_DispatchThreadID )
{
    if (threadID.x >= ScreenDimensions.x || threadID.y >= ScreenDimensions.y)
        return;

    // Screen cords
    // (1/ Y) * X * 2 - 1 to get screenspace
    // Less computation with;
    // (1/Y)*2) * X - 1
    float2 screenPos = float2(threadID.xy) * DoubleScreenByDimension + float2(-1.0f, 1.0f);

    // Create ray in screenspace
    // As the proj is diagonal matrix, we can use mul=>div to get from screen=>
    float2 screenSpaceDirection = float2(screenPos.x / Proj._11, screenPos.y / Proj._22);

    // Multiplie with inverse view here
    // TODO check if We could maybe multiply the vertices with view instead?
    float3 viewSpacePos = mul(float4(screenPos, 2.0f, 1.0f), InvView).xyz;

    // Get direction in view space
    float3 viewSpaceDirection = mul(float4(screenSpaceDirection, 1.0f, 0.0f), InvView).xyz;

    // Out slot for rays 
    uint outIndex = threadID.y * ScreenDimensions.x + threadID.x;

    // Create ray
    Ray newRay;
    newRay.Position = viewSpacePos;
    newRay.Direction = normalize(viewSpaceDirection);
    newRay.Color = float3(0,0,0);
    newRay.lastVertexIndex = -1;
    newRay.lastSphereIndex = -1;
    newRay.lastInstanceIndex = -1;
    newRay.reflectionFactor = 1.0f;
    newRay.dead = 0;
    rays[outIndex] = newRay;
}
