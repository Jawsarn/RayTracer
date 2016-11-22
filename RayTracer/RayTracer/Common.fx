
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


struct Sphere
{
    float3 Position;
    float Radius;
    float3 Color;
};

struct Vertex
{
    float3 Position;
    float3 Normal;
    float2 TexCord;
};

static const float kEpsilon = 1e-8;


// Global memory resources
// TODO Check difference from changing color on backbuffer then variable in Rays
RWTexture2D<float4> output : register(u0);
RWStructuredBuffer<Ray> rays : register(u1);
StructuredBuffer<Vertex> vertices: register(t2);