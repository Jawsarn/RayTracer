// Buffers
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

// Primitives
struct Ray
{
    float3 Position;
    float3 Direction;
    float3 Color;
    int lastVertexIndex;
    float reflectionFactor;
};

struct ColorData
{
    int indexTriangle;
    int indexSphere;
    float3 hitPosition;
    float u;
    float v;
    float t;
};

struct Vertex
{
    float3 Position;
    float3 Normal;
    float2 TexCord;
};

struct Sphere
{
    float3 Position;
    float Radius;
    float3 Color;
};

struct PointLight
{
    float3 Position;
    float Radius;
    float3 Color;
};



static const float kEpsilon = 1e-8;


// Global memory resources
// TODO Check difference from changing color on backbuffer then variable in Rays
RWTexture2D<float4> output : register(u0);
RWStructuredBuffer<Ray> rays : register(u1);
RWStructuredBuffer<ColorData> colorData : register(u2);

StructuredBuffer<Vertex> vertices: register(t3);
StructuredBuffer<Sphere> spheres : register(t4);
StructuredBuffer<PointLight> pointLights : register(t5);