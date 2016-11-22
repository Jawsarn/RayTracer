
cbuffer ConstantBuffer : register(b0)
{
    matrix Proj;
    uint2 ScreenDimensions; //width height
    float2 DoubleScreenByDimension;
};

cbuffer PerFrameBuffer : register(b1)
{
    matrix InvView;

    float3 CameraPosition;
    uint NumOfPointLights;

    uint NumOfVertices;
    uint NumOfSpheres;

    float2 filler;
};


struct Ray
{
    float3 Position;
    float3 Direction;
    float3 Color;
    int lastVertexIndex;
    float reflectionFactor;
};


