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
    uint NumOfSpotLights;

    float filler;
};

// Primitives
struct Ray
{
    float3 Position;
    float3 Direction;
    float3 Color;
    int lastVertexIndex;
    int lastSphereIndex;
    float reflectionFactor;
};

struct ColorData
{
    int indexTriangle;
    int indexSphere;
    float3 startPosition;
    float3 direction;
    float3 hitPosition;
    float3 color;
    float reflection;
    float u;
    float v;
};

struct Vertex
{
    float3 Position;
    float3 Normal;
    float3 Tangent;
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

struct SpotLight
{
    float3 Position;
    float3 Direction;
    float Radius;
    float3 Color;
    float Spot;
};

static const float kEpsilon = 1e-8;


// Global memory resources
RWStructuredBuffer<Ray> rays : register(u0);
RWStructuredBuffer<ColorData> colorData : register(u1);

StructuredBuffer<Vertex> vertices: register(t3);
StructuredBuffer<Sphere> spheres : register(t4);
StructuredBuffer<PointLight> pointLights : register(t5);
StructuredBuffer<SpotLight> spotLights : register(t8);

Texture2D meshTexture : register(t6);
Texture2D normalMap : register(t9);

RWTexture2D<float4> ssOutput : register(u7);

SamplerState simpleSampler : register(s0);