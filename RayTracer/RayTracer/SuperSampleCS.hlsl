#include "Common.fx"

[numthreads(32, 32, 1)]
void CS( uint3 threadID : SV_DispatchThreadID)
{
    if (threadID.x >= ScreenDimensions.x || threadID.y >= ScreenDimensions.y)
        return;

    uint2 xxID = threadID.xy * 2;

    uint index = xxID.y * ScreenDimensions.x + xxID.x;
    uint indexX = (xxID.y + 1) * ScreenDimensions.x + xxID.x;

    float3 color = colorData[index].color + colorData[index + 1].color + colorData[indexX].color + colorData[indexX + 1].color;
    ssOutput[threadID.xy] = float4(color, 1);
}