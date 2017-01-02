
cbuffer ConstantBuffer : register(b0)
{
    matrix Proj;
    uint2 ScreenDimensions; //width height
    float2 DoubleScreenByDimension;
};

Texture2D<float4> output : register(t0);
RWTexture2D<float4> ssOutput : register(u7);

[numthreads(32, 32, 1)]
void CS( uint3 threadID : SV_DispatchThreadID)
{
    if (threadID.x >= ScreenDimensions.x || threadID.y >= ScreenDimensions.y)
        return;

    uint2 xxID = threadID.xy * 2;
    //ssOutput[threadID.xy] = output[threadID.xy];
    ssOutput[threadID.xy] = output[xxID] + output[xxID + uint2(1, 0)] + output[xxID + uint2(0, 1)] + output[xxID + uint2(1, 1)];
}